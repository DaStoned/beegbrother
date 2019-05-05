/**
@file DriverGpio.cpp
@brief Implement a driver for ESP8266 GPIO
*/

#include "DriverGpio.hpp"
extern "C" {
    // Include the C-only SDK headers
    #include "osapi.h"
    #include "gpio.h"
}

// Can't find offical documentation on ESP8266 GPIO, this fills in the gaps
// https://www.esp8266.com/wiki/doku.php?id=esp8266_gpio_pin_allocations

// From https://github.com/espressif/ESP8266_IOT_PLATFORM/blob/master/include/driver/gpio.h
#define GPIO_PIN_REG_0          PERIPHS_IO_MUX_GPIO0_U
#define GPIO_PIN_REG_1          PERIPHS_IO_MUX_U0TXD_U
#define GPIO_PIN_REG_2          PERIPHS_IO_MUX_GPIO2_U
#define GPIO_PIN_REG_3          PERIPHS_IO_MUX_U0RXD_U
#define GPIO_PIN_REG_4          PERIPHS_IO_MUX_GPIO4_U
#define GPIO_PIN_REG_5          PERIPHS_IO_MUX_GPIO5_U
#define GPIO_PIN_REG_6          PERIPHS_IO_MUX_SD_CLK_U
#define GPIO_PIN_REG_7          PERIPHS_IO_MUX_SD_DATA0_U
#define GPIO_PIN_REG_8          PERIPHS_IO_MUX_SD_DATA1_U
#define GPIO_PIN_REG_9          PERIPHS_IO_MUX_SD_DATA2_U
#define GPIO_PIN_REG_10         PERIPHS_IO_MUX_SD_DATA3_U
#define GPIO_PIN_REG_11         PERIPHS_IO_MUX_SD_CMD_U
#define GPIO_PIN_REG_12         PERIPHS_IO_MUX_MTDI_U
#define GPIO_PIN_REG_13         PERIPHS_IO_MUX_MTCK_U
#define GPIO_PIN_REG_14         PERIPHS_IO_MUX_MTMS_U
#define GPIO_PIN_REG_15         PERIPHS_IO_MUX_MTDO_U

#define GPIO_PIN_REG(i) \
    (i==0) ? GPIO_PIN_REG_0:  \
    (i==1) ? GPIO_PIN_REG_1:  \
    (i==2) ? GPIO_PIN_REG_2:  \
    (i==3) ? GPIO_PIN_REG_3:  \
    (i==4) ? GPIO_PIN_REG_4:  \
    (i==5) ? GPIO_PIN_REG_5:  \
    (i==6) ? GPIO_PIN_REG_6:  \
    (i==7) ? GPIO_PIN_REG_7:  \
    (i==8) ? GPIO_PIN_REG_8:  \
    (i==9) ? GPIO_PIN_REG_9:  \
    (i==10)? GPIO_PIN_REG_10: \
    (i==11)? GPIO_PIN_REG_11: \
    (i==12)? GPIO_PIN_REG_12: \
    (i==13)? GPIO_PIN_REG_13: \
    (i==14)? GPIO_PIN_REG_14: \
    GPIO_PIN_REG_15

// Don't use those, they are reserved for SPI Flash
#define FUNC_GPIO6  3
#define FUNC_GPIO7  3
#define FUNC_GPIO8  3
#define FUNC_GPIO11  3

#define GPIO_PIN_MODE_GPIO(i) \
    (i==0) ? FUNC_GPIO0:  \
    (i==1) ? FUNC_GPIO1:  \
    (i==2) ? FUNC_GPIO2:  \
    (i==3) ? FUNC_GPIO3:  \
    (i==4) ? FUNC_GPIO4:  \
    (i==5) ? FUNC_GPIO5:  \
    (i==6) ? FUNC_GPIO6:  \
    (i==7) ? FUNC_GPIO7:  \
    (i==8) ? FUNC_GPIO8:  \
    (i==9) ? FUNC_GPIO9:  \
    (i==10)? FUNC_GPIO10: \
    (i==11)? FUNC_GPIO11: \
    (i==12)? FUNC_GPIO12: \
    (i==13)? FUNC_GPIO13: \
    (i==14)? FUNC_GPIO14: \
    FUNC_GPIO15

bool ICACHE_FLASH_ATTR DriverGpio::init() {
    gpio_init();
    return true;
}

void ICACHE_FLASH_ATTR DriverGpio::setPinMode(Pin pin, Mode mode, bool defaultOut) {
    PIN_FUNC_SELECT(GPIO_PIN_REG(pin), GPIO_PIN_MODE_GPIO(pin));
    switch (mode) {
        case MODE_IN:
            PIN_PULLUP_DIS(GPIO_PIN_REG(pin));
            gpio_output_set(0, 0, 0, 1 << pin);
            break; 
        case MODE_IN_PULLUP:
            PIN_PULLUP_EN(GPIO_PIN_REG(pin));
            gpio_output_set(0, 0, 0, 1 << pin);
            break;
        case MODE_OUT:
            gpio_output_set(defaultOut ? 1 << pin : 0, defaultOut ? 0 : 1 << pin, 1 << pin, 0);
            break;
    }
}

IfGpio::Mode ICACHE_FLASH_ATTR DriverGpio::getPinMode(Pin pin) const {
    // TODO: figure out how it's possible to read the mode
    return MODE_OUT;
}

void ICACHE_FLASH_ATTR DriverGpio::setPin(Pin pin, bool value) {
    GPIO_OUTPUT_SET(static_cast<unsigned int>(pin), value ? 1 : 0);
}

bool ICACHE_FLASH_ATTR DriverGpio::getPin(Pin pin) const {
    return (GPIO_INPUT_GET(pin) != 0);
}