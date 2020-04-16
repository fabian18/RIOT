/*
 * Copyright (C) 2020 OvGU Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Application to monitor current draw and power consumption
 *              of miot-esp32 boards with two ina3221
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "shell.h"
#include "msg.h"
#include "event.h"
#include "ringbuffer.h"
#define ENABLE_DEBUG    (0)
#include "debug.h"

#include "include/powermon.h"
#include "include/format_influx_db.h"

#define EVENT_QUEUE_THREAD_PRIORITY             THREAD_PRIORITY_MAIN
#define EVENT_QUEUE_THREAD_STACK_SIZE           THREAD_STACKSIZE_DEFAULT
#define EVENT_BUFFER_SIZE                       (1024)
#define MAIN_MSG_QUEUE_SIZE                     (4)

#define POWERMON_EVENT(t, h)                                                    \
(powermon_event_t) {                                                            \
    .event = {                                                                  \
        .list_node = { .next = NULL },                                          \
        .handler = (h)                                                          \
    },                                                                          \
    .type = (t)                                                                 \
}

typedef enum {
    EVENT_MEASUREMENT,
    EVENT_CONFIG,
    EVENT_ERROR
} powermon_event_type_t;

struct powermon_event {
    event_t event;
    powermon_event_type_t type;
};
typedef struct powermon_event powermon_event_t;
struct measurement_event {
    powermon_event_t ev;
    const powermon_t* mon;
    powermon_measurement_result_t mres;
    powermon_result_t res;
};
typedef struct measurement_event measurement_event_t;

struct config_event {
    powermon_event_t ev;
    const powermon_t* mon;
    powermon_config_t cfg_new;
    powermon_config_t cfg_old;
};
typedef struct config_event config_event_t;

struct error_event {
    powermon_event_t ev;
    char msg[16];
};
typedef struct error_event error_event_t;

struct ringbuffer_event_queue {
    event_queue_t queue;
    ringbuffer_t buffer;
};
typedef struct ringbuffer_event_queue ringbuffer_event_queue_t;

int sc_powermon(int argc, char* argv[]);

static uint8_t _event_buffer[EVENT_BUFFER_SIZE];
static char _event_queue_thread_stack[EVENT_QUEUE_THREAD_STACK_SIZE];
static msg_t _main_msg_queue[MAIN_MSG_QUEUE_SIZE];

static const shell_command_t shell_commands[] = {
    {"powermon", "power monitor", sc_powermon},
    { NULL, NULL, NULL }
};

static
void* _event_thread_task(void* arg)
{
    DEBUG("Start event queue\n");
    ringbuffer_event_queue_t* queue = arg;
    event_queue_init(&queue->queue);

    union {
        powermon_event_t powermon_event;
        measurement_event_t measurement;
        config_event_t config;
        error_event_t error;
    } uevent;

    event_t *event;
    while ((event = event_wait(&queue->queue))) {
        DEBUG("Handle event %p\n", event);
        ringbuffer_get(&queue->buffer, (char*)&uevent, sizeof(powermon_event_t));
        switch (uevent.powermon_event.type) {
            case EVENT_MEASUREMENT:
                ringbuffer_get(&queue->buffer,
                               ((char*)&uevent) + sizeof(powermon_event_t),
                               sizeof(measurement_event_t) -
                               sizeof(powermon_event_t));
                uevent.measurement.ev.event.handler((event_t*)&uevent);
                break;
            case EVENT_CONFIG:
                ringbuffer_get(&queue->buffer,
                               ((char*)&uevent) + sizeof(powermon_event_t),
                               sizeof(config_event_t) -
                               sizeof(powermon_event_t));
                uevent.config.ev.event.handler((event_t*)&uevent);
                break;
            case EVENT_ERROR:
                ringbuffer_get(&queue->buffer,
                               ((char*)&uevent) + sizeof(powermon_event_t),
                               sizeof(error_event_t) -
                               sizeof(powermon_event_t));
                uevent.config.ev.event.handler((event_t*)&uevent);
            default:
                DEBUG_PUTS("Unknown event type, something is really wrong here\n");
                assert(false);
                break;
        }
    }

    /* never reached */
    assert(false);
    return NULL;
}

static
void _measurement_event_handler(event_t* ev)
{
    DEBUG("Execute measurement complete event: %p\n", ev);
    measurement_event_t* event = (measurement_event_t*)ev;
    ina3221_channel_t channels = powermon_get_channels(event->mon);
    const powermon_info_t* info = powermon_get_info(event->mon);
    size_t len;
    int i = 0;
    ina3221_channel_t cha[] = { INA3221_CH1, INA3221_CH2, INA3221_CH3 };
    for (unsigned chi = 0; chi < ARRAY_SIZE(cha); chi++) {
        if (channels & cha[chi]) {
            len = format_influx_db_calculate_len(info->ch_names[chi],
                                                 event->mres.shunt_uv[i],
                                                 event->mres.bus_mv[i],
                                                 event->res.current_ua[i],
                                                 event->res.power_uw[i]);
            char buffer[len];
            format_influx_db_write(buffer, sizeof(buffer),
                                   info->ch_names[chi],
                                   event->mres.shunt_uv[i],
                                   event->mres.bus_mv[i],
                                   event->res.current_ua[i],
                                   event->res.power_uw[i]);
            i++;
            puts(buffer);
        }
    }
}

static
void _config_event_handler(event_t* ev)
{
    DEBUG("Execute config change event: %p\n", ev);
    config_event_t* event = (config_event_t*)ev;
    (void)event;
}

static
void _error_event_handler(event_t* ev)
{
    DEBUG("Executing error event: %p\n", ev);
    error_event_t* event = (error_event_t*)ev;
    (void)event;
}

static
void _powermon_measurement(powermon_t* mon,
                           const powermon_measurement_result_t* mres,
                           const powermon_result_t* res,
                           void* user_data)
{
    ringbuffer_event_queue_t* event_queue = user_data;

    /* copy results and put event into application event queue */
    if (ringbuffer_full(&event_queue->buffer)) {
        DEBUG("Event buffer full!!\n");
        return;
    }
    measurement_event_t ev = {
        .ev = POWERMON_EVENT(EVENT_MEASUREMENT, _measurement_event_handler),
        .mon = mon,
        .mres = *mres,
        .res = *res
    };
    ringbuffer_add(&event_queue->buffer, (const char*)&ev, sizeof(ev));
    config_event_t* ev_ptr = NULL;
    ringbuffer_access(&event_queue->buffer, (char**)&ev_ptr, -sizeof(measurement_event_t));
    DEBUG("Post measurement complete event %p\n", ev_ptr);
    event_post(&event_queue->queue, (event_t*)ev_ptr);
}

static
void _powermon_config(powermon_t* mon,
                      powermon_config_t* cfg_new,
                      powermon_config_t* cfg_old,
                      void* user_data)
{
    ringbuffer_event_queue_t* event_queue = user_data;

    /* copy results and put event into application event queue */
    if (ringbuffer_full(&event_queue->buffer)) {
        DEBUG("Event buffer full!!\n");
        return;
    }
    config_event_t ev = {
        .ev = POWERMON_EVENT(EVENT_CONFIG, _config_event_handler),
        .mon = mon,
        .cfg_new = *cfg_new,
        .cfg_old = *cfg_old
    };

    ringbuffer_add(&event_queue->buffer, (const char*)&ev, sizeof(ev));
    config_event_t* ev_ptr = NULL;
    ringbuffer_access(&event_queue->buffer, (char**)&ev_ptr, -sizeof(config_event_t));
    DEBUG("Post config change event %p\n", ev_ptr);
    event_post(&event_queue->queue, (event_t*)ev_ptr);
}

static
void _powermon_error(powermon_t* mon, char* msg, void* user_data)
{
    ringbuffer_event_queue_t* event_queue = user_data;

    error_event_t ev = {
        .ev = POWERMON_EVENT(EVENT_ERROR, _error_event_handler),
        .msg = {0}
    };
    snprintf(ev.msg, sizeof(msg), "%s", msg);

    ringbuffer_add(&event_queue->buffer, (const char*)&ev, sizeof(ev));
    error_event_t* ev_ptr = NULL;
    ringbuffer_access(&event_queue->buffer, (char**)&ev_ptr, -sizeof(error_event_t));
    DEBUG("Post error event %p\n", ev_ptr);
    event_post(&event_queue->queue, (event_t*)ev_ptr);
}

int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_MSG_QUEUE_SIZE);

    /* init event buffer and queue */
    ringbuffer_event_queue_t event_queue;
    ringbuffer_init(&event_queue.buffer,
                    (char*)_event_buffer,
                    sizeof(_event_buffer));

    /* init event queue thread */
    int event_queue_thread_pid = thread_create(_event_queue_thread_stack,
                                               sizeof(_event_queue_thread_stack),
                                               EVENT_QUEUE_THREAD_PRIORITY,
                                               THREAD_CREATE_STACKTEST,
                                               _event_thread_task,
                                               &event_queue,
                                               "powermon_event_queue");

    if (event_queue_thread_pid <= 0) {
        DEBUG("event_queue_thread_pid: %d\n", event_queue_thread_pid);
        return EXIT_FAILURE;
    }

    /* start power monitoring */
    int num_powermon = powermon_init();
    DEBUG("Number of monitors: %d\n", num_powermon);
    for (int i = 0; i < num_powermon; i++) {
        powermon_t* mon = powermon_get_mon(i);
        powermon_set_user_data(mon, &event_queue);
        powermon_set_on_measurement(mon, _powermon_measurement);
        powermon_set_on_config(mon, _powermon_config);
        powermon_set_on_error(mon, _powermon_error);
    }

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
