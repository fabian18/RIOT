#include <string.h>

#define ENABLE_DEBUG    (0)
#include "debug.h"
#include "xtimer.h"
#include "blink_interrupt.h"

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

typedef struct {
    xtimer_t *timer;
    const blink_msg_t *messages;
    uint8_t message_index;
    uint8_t byte_index;
    uint8_t bit_index;
} blink_msg_state_t;

static void _isr_blink_even(void *arg);
static void _isr_blink_odd(void *arg);

static blink_msg_state_t _msg;
static xtimer_t _timer;

static void _isr_blink_even(void *arg) {
    blink_msg_state_t *state = arg;
    if (state->timer->callback) {
        state->timer->callback = _isr_blink_odd;
        xtimer_set(state->timer, TOGGLE_DELAY);
    }
    uint8_t c = state->byte_index == 0
        ? SYNC_WORD
        : state->messages[state->message_index].data[state->byte_index - 1];
    DEBUG("byte: %u\n", c);
    if (c & (1 << (7 - (state->bit_index)))) {
        /* blink 1, first signal */
        gpio_clear(LED0_PIN);
    }
    else {
        /* blink 0, first signal */
        gpio_set(LED0_PIN);
    }
}

static void _isr_blink_odd(void *arg) {
    blink_msg_state_t *state = arg;
    if (state->timer->callback) {
        state->timer->callback = _isr_blink_even;
        xtimer_set(state->timer, TOGGLE_DELAY);
    }
    uint8_t c = state->byte_index == 0
        ? SYNC_WORD
        : state->messages[state->message_index].data[state->byte_index - 1];
    if (c & (1 << (7 - (state->bit_index)))) {
        /* blink 1, second signal */
        gpio_set(LED0_PIN);
    }
    else {
        /* blink 0, second signal */
        gpio_clear(LED0_PIN);
    }
    assert(state->messages[state->message_index].data);
    assert(state->byte_index <= state->messages[state->message_index].data_len);
    if (state->bit_index < 7) {
        state->bit_index++;
    }
    else if (state->byte_index < state->messages[state->message_index].data_len) {
        state->byte_index++;
        state->bit_index = 0;
    }
    else if (state->messages[state->message_index + 1].data) {
        state->message_index++;
        state->byte_index = 0;
        state->bit_index = 0;
    }
    else {
        state->message_index = 0;
        state->byte_index = 0;
        state->bit_index = 0;
    }
}

void blink_interrupt_start(const blink_msg_t *messages) {
    _timer = (xtimer_t) {
        .next = NULL,
        .offset = 0,
        .long_offset = 0,
        .start_time = 0,
        .long_start_time = 0,
        .callback = _isr_blink_even,
        .arg = &_msg
    };
    _msg = (blink_msg_state_t) {
        .timer = &_timer,
        .messages = messages,
        .message_index = 0,
        .byte_index = 0,
        .bit_index = 0
    };
    xtimer_set(_msg.timer, 0);
}

void blink_interrupt_stop(void) {
    _timer.callback = NULL;
    xtimer_remove(_msg.timer);
}
