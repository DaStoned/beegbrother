/**
@file DriverAm2302.cpp
@brief Implement a driver for AM2302/DHT22 temperature and humidity sensor
*/

#include "DriverAm2302.hpp"
#include "IfGpio.hpp"
#include "IfTimers.hpp"
extern "C" {
    // Include the C-only SDK headers
    #include "osapi.h"
    #include "user_interface.h"
}

bool DriverAm2302::init(IfGpio::Pin pin) {
    mPin = pin;
    return true;
}

bool DriverAm2302::update() {
    bool pinSt = false, pinStNew = false;
    IfTimers::Timespan ts, period;
    os_printf("Starting data transfer\n");
    // Request data transfer
    mGpio.setPinMode(mPin, IfGpio::MODE_OUT);
    setPinWait(1, 10000);
    setPinWait(0, 10000);
    setPinWait(1, 40);
    mGpio.setPinMode(mPin, IfGpio::MODE_IN_PULLUP);
    mTimers.delay(5);
    pinSt = mGpio.getPin(mPin);
    ts = mTimers.beginStopwatch();
    pinStNew = pinSt;
    while (1) {
        mTimers.delay(1);
        pinStNew = mGpio.getPin(mPin);
        if (pinStNew != pinSt) {
            period = mTimers.endStopwatch(ts);
            os_printf("Pin %u %u->%u after %u us\n", mPin, pinSt ? 1 : 0, pinStNew ? 1 : 0, (unsigned int) period);
            ts = mTimers.beginStopwatch();
            pinSt = pinStNew;
        }
        system_soft_wdt_feed();
    }
    // Switch
    return true;
}

void DriverAm2302::setPinWait(bool value, uint32_t durationUs) const {
    mGpio.setPin(mPin, value);
    if (durationUs > 0) {
        mTimers.delay(durationUs);
    }
}