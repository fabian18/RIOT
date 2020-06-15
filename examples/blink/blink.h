#ifndef BLINK_H
#define BLINK_H

#include "board.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif
#if 1
#define MESSAGES                                                    \
{                                                                   \
    {"SSID=Tech_D3858095", 18},                                     \
    {"PSK=6841118259180262", 20},                                   \
    {NULL, 0}                                                       \
}
#else
#define MESSAGES                                                    \
{                                                                   \
    {"abcdefghijklmnop", 16},                                                \
    {"1234567890", 10},                                                \
    {NULL, 0}                                                       \
}
#endif

#ifdef EXT_LED_PIN
#ifdef LED0_PIN
#undef LED0_PIN
#endif
#define LED0_PIN        (EXT_LED_PIN)
#endif

#ifndef BLINK_HZ
#define BLINK_HZ        (4)
#endif

#ifndef SYNC_WORD
#define SYNC_WORD       (0b01111111)
#endif

#define TOGGLE_DELAY    ((US_PER_SEC / BLINK_HZ) / 2)

typedef struct {
    const char *data;
    uint8_t data_len;
} blink_msg_t;

void blink_init(void);
void blink_sync(void);
void blink_byte(char byte);
void blink_data(const char *msg, uint8_t data_len);
void blink_messages(const blink_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif /* BLINK_H */
