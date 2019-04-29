/**
@file Timers.cpp
@brief Define a module for accessing timers
*/

#include "Timers.hpp"

extern "C" {
#include "osapi.h"
#include "user_interface.h"
}

void Timers::delay(uint32_t us) const {
    uint32_t delay_period = us;
    while (delay_period > UINT16_MAX) {
        os_delay_us(UINT16_MAX);
        delay_period -= UINT16_MAX;
    }
    os_delay_us(delay_period);
}

uint32_t Timers::getSystemTime() const {
    return system_get_time();
}

IfTimers::Timespan Timers::beginStopwatch() const {
    return  system_get_time();
}


IfTimers::Timespan Timers::endStopwatch(Timespan start) const {
    return  system_get_time() - start;
}