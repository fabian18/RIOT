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
#define ENABLE_DEBUG (0)
#include "debug.h"

#include "powermon.h"
#include "format_influx_db.h"

#define POWERMON_THREAD_PRIORITY                THREAD_PRIORITY_MAIN
#define POWERMON_THREAD_STACK_SIZE              THREAD_STACKSIZE_DEFAULT
#define MAIN_MSG_QUEUE_SIZE                     (4)

int sc_powermon(int argc, char* argv[]);

static msg_t _main_msg_queue[MAIN_MSG_QUEUE_SIZE];
static char _powermon_stack[POWERMON_THREAD_STACK_SIZE];

static const shell_command_t shell_commands[] = {
    {"powermon", "power monitor", sc_powermon},
    { NULL, NULL, NULL }
};

static void _powermon_measurement(const powermon_t* mon,
                                  const powermon_measurement_result_t* mres,
                                  const powermon_result_t* res,
                                  void* user_data)
{
    DEBUG("Execute measurement handler for monitor: %p\n", mon);
    static char buffer[128];
    const ina3221_channel_t cha[] = { INA3221_CH1, INA3221_CH2, INA3221_CH3 };
    ina3221_channel_t channels = powermon_get_channels(mon);
    const powermon_info_t* info = powermon_get_info(mon);
    int i = 0;
    for (unsigned chi = 0; chi < ARRAY_SIZE(cha); chi++) {
        if (channels & cha[chi]) {
            format_influx_db_write(buffer, sizeof(buffer),
                                   info->ch_names[chi],
                                   mres->shunt_uv[i],
                                   mres->bus_mv[i],
                                   res->current_ua[i],
                                   res->power_uw[i]);
            i++;
            puts(buffer);
        }
    }
}

static void _powermon_config(const powermon_t* mon,
                             powermon_config_t* cfg_new,
                             powermon_config_t* cfg_old,
                             void* user_data)
{
    DEBUG("Execute config change handler for monitor: %p\n", mon);
    (void)mon;
    (void)cfg_new;
    (void)cfg_old;
    (void)user_data;
}

static void _powermon_error(const powermon_t* mon, char* msg, void* user_data)
{
    DEBUG("Executing error handler for monitor: %p\n", mon);
    puts(msg);
    (void)mon;
    (void)user_data;
}

int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_MSG_QUEUE_SIZE);

    /* start power monitoring */
    int num_powermon = powermon_init();
    DEBUG("Number of monitors: %d\n", num_powermon);
    for (int i = 0; i < num_powermon; i++) {
        powermon_t* mon = powermon_get_mon(i);
        powermon_set_user_data(mon, NULL);
        powermon_set_on_measurement(mon, _powermon_measurement);
        powermon_set_on_config(mon, _powermon_config);
        powermon_set_on_error(mon, _powermon_error);
    }
    kernel_pid_t powermon_pid = powermon_start(_powermon_stack,
                                               sizeof(_powermon_stack),
                                               POWERMON_THREAD_PRIORITY);
    if (powermon_pid <= 0) {
        DEBUG("powermon thread pid: %d\n", powermon_pid);
        return EXIT_FAILURE;
    }

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
