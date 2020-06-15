#include <string.h>

#define ENABLE_DEBUG    (0)
#include "debug.h"
#include "thread.h"
#include "xtimer.h"
#include "blink_thread.h"

static char _stack[THREAD_STACKSIZE_DEFAULT];
static volatile bool  _do_blink = false; /* not sure if this must be volatile */
static pid_t _my_pid = -1;

static
void *_blink_thread(void *arg) {
    blink_msg_t *messages = ((blink_msg_t *)arg);
    while (1) {
        while (_do_blink) {
            blink_messages(messages);
        }
        thread_sleep();
    }
    return NULL;
}

int blink_thread_create(const blink_msg_t *messages) {
    /* there shall only be one blink thread */
    if (_my_pid < 0) {
        _do_blink = true;
        _my_pid = thread_create(_stack, sizeof(_stack),
                                THREAD_PRIORITY_MAIN,
                                THREAD_CREATE_STACKTEST,
                                _blink_thread, (void *)messages,
                                "blink");
    }
    return _my_pid;
}

void blink_thread_halt(void) {
    _do_blink = false;
}

void blink_thread_continue(void) {
    _do_blink = true;
    thread_wakeup(_my_pid);
}
