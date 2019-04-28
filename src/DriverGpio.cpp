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

bool DriverGpio::init() {
    gpio_init();
    return true;
}

void DriverGpio::setPinMode(Pin pin, Mode mode) {
    switch (mode) {
        case MODE_IN:
            gpio_output_set(0, 0, 0, 1 << pin);
            break;
        case MODE_OUT:
            gpio_output_set(0, 0, 1 << pin, 0);
            break;
    }
}

IfGpio::Mode DriverGpio::getPinMode(Pin pin) const {
    // TODO: figure out how it's possible to read the mode
    return MODE_OUT;
}

void DriverGpio::setPin(Pin pin, bool value) {
    GPIO_OUTPUT_SET(static_cast<unsigned int>(pin), value ? 1 : 0);
}

bool DriverGpio::getPin(Pin pin) const {
    return (GPIO_INPUT_GET(pin) != 0);
}