#include "vendor/RP2040.h"
#include "reg_atomic.h"

int main(void) {
    SIO->GPIO_OE_CLR.reg = 1u << 25;
    SIO->GPIO_OUT_CLR.reg = 1u << 25;
    reg_atomic_assign(&PADS_BANK0->GPIO25,
                      1u << PADS_BANK0_GPIO25_IE_Pos,
                      PADS_BANK0_GPIO25_IE_Msk | PADS_BANK0_GPIO25_OD_Msk);
    reg_atomic_assign(&IO_BANK0->GPIO25_CTRL,
                      IO_BANK0_GPIO25_CTRL_FUNCSEL_sio_25 << IO_BANK0_GPIO25_CTRL_FUNCSEL_Pos,
                      IO_BANK0_GPIO25_CTRL_FUNCSEL_Msk);
    SIO->GPIO_OE_SET.reg = 1u << 25;
    SIO->GPIO_OUT_SET.reg = 1u << 25;

    while (1) {
        SIO->GPIO_OUT_SET.reg = 1u << 25;
        for (volatile unsigned i = 125000000; i;) {
            i--;
        }
        SIO->GPIO_OUT_CLR.reg = 1u << 25;
        for (volatile unsigned i = 125000000; i;) {
            i--;
        }
    }
}
