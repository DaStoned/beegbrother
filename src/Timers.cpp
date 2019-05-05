/**
@file Timers.cpp
@brief Define a module for accessing timers
*/

#include "Timers.hpp"

extern "C" {
#include "osapi.h"
#include "user_interface.h"
}

void ICACHE_FLASH_ATTR Timers::delay(uint32_t us) const {
    uint32_t delay_period = us;
    while (delay_period > UINT16_MAX) {
        os_delay_us(UINT16_MAX);
        delay_period -= UINT16_MAX;
    }
    os_delay_us(delay_period);
}

uint32_t ICACHE_FLASH_ATTR Timers::getSystemTimeUs() const {
    return system_get_time();
}

IfTimers::Timespan ICACHE_FLASH_ATTR Timers::beginStopwatch() const {
    return  system_get_time();
}


unsigned int ICACHE_FLASH_ATTR Timers::readStopwatch(Timespan start) const {
    uint32_t now = system_get_time();
    if (now > start) {
        return (unsigned int) (now - start);
    } else {
        // Microsecond timer overflowed, compensate
        return (unsigned int) (UINT32_MAX - start + now);
    }
}