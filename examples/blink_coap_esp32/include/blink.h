#ifndef BLINK_H
#define BLINK_H

#include "board.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if 1
#define BLINK_MESSAGES                                              \
{                                                                   \
    {(const uint8_t *)"abcdefghijklmnop", 16},                      \
    {(const uint8_t *)"1234567890", 10},                            \
    {NULL, 0},                                                      \
    {NULL, 0}                                                       \
}
#endif

#ifndef BLINK_MESSAGES
#define BLINK_MESSAGES                                              \
{                                                                   \
    {NULL, 0},                                                      \
    {NULL, 0},                                                      \
    {NULL, 0},                                                      \
    {NULL, 0}                                                       \
}
#endif

#ifndef BLINK_MESSAGE_MAX_LEN
#define BLINK_MESSAGE_MAX_LEN       (32)
#endif

#ifdef EXT_LED_PIN
#ifdef LED0_PIN
#undef LED0_PIN
#endif
#define LED0_PIN                    (EXT_LED_PIN)
#endif

#ifndef BLINK_HZ
#define BLINK_HZ                    (4)
#endif

#ifndef SYNC_WORD
#define SYNC_WORD                   (0b01111111)
#endif

#define TOGGLE_DELAY                ((US_PER_SEC / BLINK_HZ) / 2)

typedef struct {
    const uint8_t *data;
    uint8_t data_len;
} blink_msg_t;

void blink_init(void);
void blink_sync(void);
void blink_byte(uint8_t byte);
void blink_data(const uint8_t *msg, uint8_t data_len);
void blink_messages(const blink_msg_t *msg);

#define BLINK_NUM_OF   (sizeof((blink_msg_t[])BLINK_MESSAGES) / \
                        sizeof(blink_msg_t))

#ifdef __cplusplus
}
#endif

#endif /* BLINK_H */
