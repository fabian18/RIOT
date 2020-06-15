#include <stdio.h>
#define ENABLE_DEBUG    (1)
#include "debug.h"
#include "board.h"
#include "kernel_defines.h"
#include "msg.h"
#include "thread.h"
#include "xtimer.h"
#include "net/gcoap.h"
#include "shell.h"

#include "include/blink.h"
#include "include/blink_resource_handler.h"
#include "include/blink_interrupt.h"

#define PROGRAM                 "blink"
#define MAIN_QUEUE_SIZE         (4)

static int _sc_blink(int argc, char **argv);

/* main thread message queue */
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

/* shell commands */
static const shell_command_t _shell_commands[] = {
    { PROGRAM, "blinking LED", _sc_blink },
    { NULL, NULL, NULL }
};

/* buffer for resource representation of blink/data */
static char _blink_data_buffer[UINT8_MAX];

/* indicates if blinking is enabled */
static bool _blinking;

/* blink/status resource handle */
static blink_status_res_handle_t _blink_status;

/* blink/data resource handle */
static blink_data_res_handle_t _blink_data;

/* messags to be binked */
static blink_msg_t _blink_messages[] = BLINK_MESSAGES;

/* actual data to be blinked */
static uint8_t _blink_message_buffer[BLINK_NUM_OF][BLINK_MESSAGE_MAX_LEN];

/* gcoap resources */
static const coap_resource_t _resources[] = {
    {"/blink/status", COAP_GET | COAP_PUT, blink_status_handler, &_blink_status},
    {"/blink/data", COAP_GET | COAP_PUT, blink_data_handler, &_blink_data}
};

static gcoap_listener_t _listener = {
    _resources,
    ARRAY_SIZE(_resources),
    gcoap_encode_link /* link format encoder */,
    NULL /* next listener */
};


static void _help(int argc, char **argv)
{
    puts(
        "USAGE: \n" \
        PROGRAM" start:   Start blinking \n" \
        PROGRAM" stop:    Stop blinkng"
    );
}

static int _sc_blink(int argc, char **argv)
{
    switch (argc) {
        case 2:
            if (!strcmp(argv[1], "start")) {
                blink_interrupt_start(_blink_messages);
            }
            else if (!strcmp(argv[1], "stop")) {
                blink_interrupt_stop();
            }
            break;
        default:
            _help(argc, argv);
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(void)
{
    msg_init_queue(_main_msg_queue, ARRAY_SIZE(_main_msg_queue));

    _blink_status.blinking = &_blinking;
    _blink_data.data = (uint8_t *)_blink_data_buffer;
    _blink_data.size = 0;
    _blink_data.capacity = sizeof(_blink_data_buffer);

    blink_init();

    DEBUG_PUTS("Init LED");

    blink_msg_t *m;
    for (uint8_t i = 0; (m = &_blink_messages[i])->data; i++) {
        if (m->data_len > MSG_MAX_LEN) {
            exit(EXIT_FAILURE);
        }
        memcpy(_blink_message_buffer[i], m->data, m->data_len);
        m->data = _blink_message_buffer[i];
    }
    ssize_t size = blink_messages_to_resource(_blink_messages,
                                              (uint8_t *)_blink_data_buffer,
                                              sizeof(_blink_data_buffer));

    DEBUG_PUTS("Blink Resources initialized");

    if (size < 0) {
        exit(EXIT_FAILURE);
    }
    _blink_data.size = (size_t)size;

    DEBUG_PUTS("All ready to blink");

    blink_interrupt_start(_blink_messages);

    gcoap_register_listener(&_listener);

    /* shell    */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(_shell_commands, line_buf, ARRAY_SIZE(line_buf));

    return 0;
}