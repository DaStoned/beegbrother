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

bool ICACHE_FLASH_ATTR DriverAm2302::init(IfGpio::Pin pin) {
    mPin = pin;
    os_printf("Local GPIO: %08X\n", (unsigned int) &mGpio);
    mGpio.setPinMode(mPin, IfGpio::MODE_OUT);
    mGpio.setPin(mPin, true);
    return true;
}

bool ICACHE_FLASH_ATTR DriverAm2302::update() {
    bool pinSt = false, pinStNew = false;
    IfTimers::Timespan ts, period;
    os_printf("Starting data transfer on pin %u\n", (unsigned int) mPin);
    // Request data transfer
    mGpio.setPinMode(mPin, IfGpio::MODE_OUT);
    setPinWait(true, 25000);
    setPinWait(false, 20000);
    setPinWait(true, 40);
    mGpio.setPinMode(mPin, IfGpio::MODE_IN_PULLUP);
    pinSt = mGpio.getPin(mPin);
    os_printf("Read %u\n", pinSt ? 1 : 0);
    ts = mTimers.beginStopwatch();
    pinStNew = pinSt;
    while (mTimers.readStopwatch(ts) < 1000000) {
        //mTimers.delay(1);
        pinStNew = mGpio.getPin(mPin);
        if (pinStNew != pinSt) {
            period = mTimers.readStopwatch(ts);
            os_printf("Pin %u %u->%u after %u us\n", mPin, pinSt ? 1 : 0, pinStNew ? 1 : 0, (unsigned int) period);
            ts = mTimers.beginStopwatch();
            pinSt = pinStNew;
        }
        if (mTimers.readStopwatch(ts) % 100000 == 0) {
            os_printf("Wait %u\n", pinStNew ? 1 : 0);
            system_soft_wdt_feed();
            mTimers.delay(1);
        }
    }
    // Switch
    return true;
}

void ICACHE_FLASH_ATTR DriverAm2302::setPinWait(bool value, uint32_t durationUs) const {
    mGpio.setPin(mPin, value);
    if (durationUs > 0) {
        mTimers.delay(durationUs);
    }
}