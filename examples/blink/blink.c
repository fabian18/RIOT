#define ENABLE_DEBUG    (0)
#include "debug.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "blink.h"

void blink_init(void) {
    DEBUG("LED Pin: %u\n", LED0_PIN);
    gpio_init(LED0_PIN, GPIO_OUT);
}

void blink_sync(void) {
    blink_byte(SYNC_WORD);
}

void blink_byte(char byte) {
    DEBUG("[blink] %u\n", byte);
    for(uint8_t i = 0; i < 8; i++) {
        if (byte & (1 << (7 - i))) {
            DEBUG("[blink] 1\n");
            gpio_clear(LED0_PIN);
            xtimer_usleep(TOGGLE_DELAY);
            gpio_set(LED0_PIN);
            xtimer_usleep(TOGGLE_DELAY);
        }
        else {
            DEBUG("[blink] 0\n");
            gpio_set(LED0_PIN);
            xtimer_usleep(TOGGLE_DELAY);
            gpio_clear(LED0_PIN);
            xtimer_usleep(TOGGLE_DELAY);
        }
    }
}

void blink_data(const char *data, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        blink_byte(data[i]);
    }
}

void blink_messages(const blink_msg_t *msg) {
    for (uint8_t i = 0; msg[i].data; i++) {
        blink_sync();
        DEBUG("%s %u\n", (char *)msg[i].data, msg[i].data_len);
        blink_data(msg[i].data, msg[i].data_len);
    }
}
