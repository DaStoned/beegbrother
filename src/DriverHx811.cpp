/**
@file DriverHx811.cpp
@brief Implement a driver for HX811 load sensor
*/

#include "DriverHx811.hpp"
#include "IfGpio.hpp"
#include "IfTimers.hpp"
extern "C" {
    // Include the C-only SDK headers
    #include "osapi.h"
    #include "user_interface.h"
}

/// Duration of half a clock cycle
static const unsigned int spanHalfClockUs = 1; 
/// Duration of sleep activation pulse
static const unsigned int spanSleepPulseUs = 60;

bool ICACHE_FLASH_ATTR DriverHx811::init(IfGpio::Pin pinClk, IfGpio::Pin pinData) {
    mPinClk = pinClk;
    mPinData = pinData;
    mGpio.setPinMode(mPinClk, IfGpio::MODE_OUT, false);
    mGpio.setPinMode(mPinData, IfGpio::MODE_IN);
    return true;
}

bool ICACHE_FLASH_ATTR DriverHx811::canUpdate() const {
    if (mPinData == IfGpio::FIRST_PIN_UNUSED) {
        os_printf("HX811: Driver not initialized\n");
        return false;
    }

    return !mGpio.getPin(mPinData);
}

bool ICACHE_FLASH_ATTR DriverHx811::update() {
    unsigned int bitNum = 0;
    
    if (mPinData == IfGpio::FIRST_PIN_UNUSED || mPinClk == IfGpio::FIRST_PIN_UNUSED) {
        os_printf("HX811: Driver not initialized\n");
        return false;
    }

    if (mInSleep) {
        // Wake up the HX811 from sleep. This will be slow.
        wakeFromSleep();
    }

    if (!canUpdate()) {
        os_printf("HX811: Not ready for reading! (wait %u ms between updates)\n", minSampleIntervalUs / 1000);
        return false;
    }
    // Set initial state for CLK
    mGpio.setPin(mPinClk, false);
    mTimers.delay(spanHalfClockUs);
    mBuffer = 0;
    // Loop and read out the sample
    while (bitNum < sensorMsgLenB * 8) {
        mGpio.setPin(mPinClk, true);
        mTimers.delay(spanHalfClockUs);
        mGpio.setPin(mPinClk, false);
        mTimers.delay(spanHalfClockUs);
        mBuffer <<= 1;
        mBuffer += mGpio.getPin(mPinData) ? 1 : 0;
        bitNum++;
    }
    // Last pulse needed to finish the transaction
    mGpio.setPin(mPinClk, true);
    mTimers.delay(spanHalfClockUs);
    mGpio.setPin(mPinClk, false);
    os_printf("HX811: Buffer 0x%08X, line %u\n", mBuffer, mGpio.getPin(mPinData) ? 1 : 0);
    if (mBuffer & 0x00800000) {
        mBuffer |= 0xFF000000;
    }
    mLoad = (int) mBuffer;
    return true;
}

void ICACHE_FLASH_ATTR DriverHx811::toSleep() {
    mGpio.setPin(mPinClk, true);
    mInSleep = true;
    mTimers.delay(spanSleepPulseUs);
}

void ICACHE_FLASH_ATTR DriverHx811::wakeFromSleep() {
    // Wake up the HX811 from sleep. This will be slow.
    IfTimers::Timespan ts = mTimers.beginStopwatch();
    mGpio.setPin(mPinClk, false);
    while (!canUpdate() && mTimers.readStopwatch(ts) < spanSettleOutput) {
        mTimers.delay(1);
    }
    mInSleep = false;
    os_printf("Sleep wakeup took %u us, %s\n", mTimers.readStopwatch(ts), canUpdate() ? "ready" : "timeout");
}