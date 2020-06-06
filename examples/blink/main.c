#define BLINK_MAIN_THREAD

#include <stdlib.h>
#include <string.h>

#define ENABLE_DEBUG    (0)
#include "debug.h"
#include "xtimer.h"
#include "shell.h"
#include "blink.h"
#if defined(BLINK_OWN_THREAD)
#include "blink_thread.h"
#elif defined(BLINK_INTERRUPT)
#include "blink_interrupt.h"
#endif

int main(void) {
    xtimer_init();
    blink_init();
#if defined(BLINK_MAIN_THREAD)
    const uint8_t data[] = MESSAGE;
    uint16_t data_len = strlen((char *)data);
    while (1) {
        blink_sync();
        blink_data(data, data_len);
    }
#elif defined(BLINK_OWN_THREAD)
    static const shell_command_t shell_commands[] = {
        { NULL, NULL, NULL }
    };

    const uint8_t data[] = MESSAGE;
    uint16_t data_len = strlen((char *)data);

    if (blink_thread_create(data, &data_len) <= 0) {
        exit(EXIT_FAILURE);
    }

    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

#elif defined(BLINK_INTERRUPT)
    static const shell_command_t shell_commands[] = {
        { NULL, NULL, NULL }
    };

    const uint8_t data[] = MESSAGE;
    uint16_t data_len = strlen((char *)data);

    if (blink_interrupt_start(data, &data_len) != 0) {
        exit(EXIT_FAILURE);
    }

    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
#endif
    return EXIT_SUCCESS;
}
