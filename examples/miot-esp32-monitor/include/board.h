#include "kernel_defines.h"

#ifndef ETHOS_UART
#define ETHOS_UART      (UART_DEV(0))
#endif

#ifndef ETHOS_BAUDRATE
#define ETHOS_BAUDRATE  (115200)
#endif

#define POWERMON_0
#define POWERMON_1

#include_next "board.h"
