#ifndef BLINK_INTERRUPT_H
#define BLINK_INTERRUPT_H

#include "blink.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MSG_MAX_LEN
#define MSG_MAX_LEN     (64)
#endif

void blink_interrupt_start(const blink_msg_t *messages) ;
void blink_interrupt_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* BLINK_INTERRUPT_H */
