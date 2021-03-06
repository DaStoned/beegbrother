/**
@file DriverHx711.cpp
@brief Implement a driver for HX711 load sensor
*/

#include "DriverHx711.hpp"
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

bool ICACHE_FLASH_ATTR DriverHx711::init(IfGpio::Pin pinClk, IfGpio::Pin pinData, ChGain chGain) {
    mPinClk = pinClk;
    mPinData = pinData;
    mChGain = chGain;
    mGpio.setPinMode(mPinClk, IfGpio::MODE_OUT, false);
    mGpio.setPinMode(mPinData, IfGpio::MODE_IN);
    return true;
}

bool ICACHE_FLASH_ATTR DriverHx711::canUpdate() const {
    if (mPinData == IfGpio::FIRST_PIN_UNUSED) {
        os_printf("HX711: Driver not initialized\n");
        return false;
    }

    return !mGpio.getPin(mPinData);
}

bool ICACHE_FLASH_ATTR DriverHx711::update() {
    unsigned int bitNum = 0, chGainPulses = 0;
    
    if (mPinData == IfGpio::FIRST_PIN_UNUSED || mPinClk == IfGpio::FIRST_PIN_UNUSED) {
        os_printf("HX711: Driver not initialized\n");
        return false;
    }

    if (mInSleep) {
        // Wake up the HX711 from sleep. This will be slow.
        wakeFromSleep();
    }

    if (!canUpdate()) {
        os_printf("HX711: Not ready for reading! (wait %u ms between updates)\n", minSampleIntervalUs / 1000);
        return false;
    }
    // Set initial state for CLK
    mGpio.setPin(mPinClk, false);
    mTimers.delay(spanHalfClockUs);
    mBuffer = 0;
    // Read out the conversion
    while (bitNum < sensorMsgLenB * 8) {
        mGpio.setPin(mPinClk, true);
        mTimers.delay(spanHalfClockUs);
        mGpio.setPin(mPinClk, false);
        mTimers.delay(spanHalfClockUs);
        mBuffer <<= 1;
        mBuffer += mGpio.getPin(mPinData) ? 1 : 0;
        bitNum++;
    }
    // Last pulse(s) needed to finish the transaction and select channel and
    // gain for the next conversion
    while (chGainPulses <= (unsigned int) mChGain) {
        mGpio.setPin(mPinClk, true);
        mTimers.delay(spanHalfClockUs);
        mGpio.setPin(mPinClk, false);
        mTimers.delay(spanHalfClockUs);
        chGainPulses++;
    }
    //os_printf("HX711: Buffer 0x%08X, line %u, ch/gain %u\n", mBuffer, mGpio.getPin(mPinData) ? 1 : 0, (unsigned int) mChGain);
    if (mBuffer & 0x00800000) {
        mBuffer |= 0xFF000000;
    }
    mLoad = (int) mBuffer;
    return true;
}

void ICACHE_FLASH_ATTR DriverHx711::toSleep() {
    mGpio.setPin(mPinClk, true);
    mInSleep = true;
    mTimers.delay(spanSleepPulseUs);
}

void ICACHE_FLASH_ATTR DriverHx711::wakeFromSleep() {
    // Wake up the HX711 from sleep. This will be slow - around 410 ms
    IfTimers::Timespan ts = mTimers.beginStopwatch();
    mGpio.setPin(mPinClk, false);
    while (!canUpdate() && mTimers.readStopwatch(ts) < spanSettleOutput) {
        mTimers.delay(1);
    }
    mInSleep = false;
    os_printf("Sleep wakeup took %u us, %s\n", mTimers.readStopwatch(ts), canUpdate() ? "ready" : "timeout");
}