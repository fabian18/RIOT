#define INA3221_PARAMS                                                         \
{                                                                              \
        .i2c = I2C_DEV(0),                                                     \
        .addr = INA3221_ADDR_00,                                               \
        .upins.pins = {                                                        \
            .pin_warn = GPIO16,                                                \
            .pin_crit = GPIO17,                                                \
            .pin_tc = GPIO_UNDEF,                                              \
            .pin_pv = GPIO_UNDEF                                               \
        },                                                                     \
        .gpio_config = (INA3221_PARAM_INT_PU_PIN_WRN << INA3221_ALERT_WRN) |   \
                       (INA3221_PARAM_INT_PU_PIN_CRT << INA3221_ALERT_CRT) |   \
                       (INA3221_PARAM_INT_PU_PIN_TC << INA3221_ALERT_TC)   |   \
                       (INA3221_PARAM_INT_PU_PIN_PV << INA3221_ALERT_PV),      \
        .config = (INA3221_ENABLE_CH1 |                                        \
                   INA3221_ENABLE_CH2 |                                        \
                   INA3221_ENABLE_CH3 |                                        \
                   INA3221_NUM_SAMPLES_64 |                                    \
                   INA3221_CONV_TIME_BADC_4156US |                             \
                   INA3221_CONV_TIME_SADC_4156US |                             \
                   INA3221_MODE_CONTINUOUS_SHUNT_BUS),                         \
        .rshunt_mohm = {                                                       \
            500,                                                               \
            500,                                                               \
            500                                                                \
        }                                                                      \
},                                                                             \
{                                                                              \
        .i2c = I2C_DEV(0),                                                     \
        .addr = INA3221_ADDR_01,                                               \
        .upins.pins = {                                                        \
            .pin_warn = GPIO25,                                                \
            .pin_crit = GPIO32,                                                \
            .pin_tc = GPIO_UNDEF,                                              \
            .pin_pv = GPIO_UNDEF                                               \
        },                                                                     \
        .gpio_config = (INA3221_PARAM_INT_PU_PIN_WRN << INA3221_ALERT_WRN) |   \
                       (INA3221_PARAM_INT_PU_PIN_CRT << INA3221_ALERT_CRT) |   \
                       (INA3221_PARAM_INT_PU_PIN_TC << INA3221_ALERT_TC)   |   \
                       (INA3221_PARAM_INT_PU_PIN_PV << INA3221_ALERT_PV),      \
        .config = (INA3221_ENABLE_CH1 |                                        \
                   INA3221_ENABLE_CH2 |                                        \
                   INA3221_ENABLE_CH3 |                                        \
                   INA3221_NUM_SAMPLES_512 |                                   \
                   INA3221_CONV_TIME_BADC_4156US |                             \
                   INA3221_CONV_TIME_SADC_4156US |                             \
                   INA3221_MODE_CONTINUOUS_SHUNT_BUS),                         \
        .rshunt_mohm = {                                                       \
            500,                                                               \
            500,                                                               \
            500                                                                \
        }                                                                      \
}

#include_next "ina3221_params.h"
