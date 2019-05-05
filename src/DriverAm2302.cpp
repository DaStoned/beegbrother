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
#include <cstring>

// Detailed timings for the AM2302

// A tiny delay to let the high level propagate to sensor
const unsigned int preWakeupUs = 10;
// AM2302 requires a low wakeup pulse with duration 1-10 ms
const unsigned int wakeupPulseUs = 2000;
// AM2302 requires a post wakeup delay of 20-40 us
const unsigned int postWakeupUs = 30;
// Receiving a message can take up to 5 ms (excluding wakeup pulse)
// because max message duration is 2 * 80 + 40 * (50 + 70) = 4960 us
const unsigned int readSampleTimeoutUs = 10000;
// Duration of AM2302's "0" pulse is 26-28 us
const unsigned int thresholdZeroMinUs = 10;
const unsigned int thresholdZeroMaxUs = 40;
// Duration of AM2302's "1" pulse is 70 us
const unsigned int thresholdOneMinUs = 50;
const unsigned int thresholdOneMaxUs = 90;

bool ICACHE_FLASH_ATTR DriverAm2302::init(IfGpio::Pin pin) {
    mPin = pin;
    // Don't initialize the output pin or we'll activate the sensor
    // on every boot and take us out of phase for the first readout.
    return true;
}

/// Note that the first update after boot is likely to fail due to 
/// sensor being confused about when it's supposed to wake up
bool ICACHE_FLASH_ATTR DriverAm2302::update() {
    bool pinSt = false, pinStNew = false, retVal;
    unsigned int headerEdges = 0, readDuration = 0, bitDuration, bitNum = 0, glitchNum = 0;
    IfTimers::Timespan readCycleTimer, bitTimer;
    memset(&mBuffer, 0, sizeof(mBuffer));
    //os_printf("AM2302: Starting data transfer on pin %u\n", (unsigned int) mPin);
    // Wake the sensor and request data transfer
    mGpio.setPinMode(mPin, IfGpio::MODE_OUT, true);
    mTimers.delay(preWakeupUs);
    setPinWait(false, wakeupPulseUs);
    setPinWait(true, postWakeupUs);
    mGpio.setPinMode(mPin, IfGpio::MODE_IN_PULLUP);
    // AM2302 starts by pulling the line low for 80 us
    mTimers.delay(1);
    pinSt = false;
    pinStNew = pinSt;
    //os_printf("Read %u\n", pinSt ? 1 : 0);
    readCycleTimer = mTimers.beginStopwatch();
    bitTimer = mTimers.beginStopwatch();
    while (readDuration < readSampleTimeoutUs && bitNum < (sensorMsgLenB * 8)) {
        pinStNew = mGpio.getPin(mPin);
        readDuration = mTimers.readStopwatch(readCycleTimer);
        if (pinStNew == pinSt) {
            // No edges detected
            continue;
        }

        // Edge detected, process
        bitDuration = mTimers.readStopwatch(bitTimer);
        if (headerEdges < 2) {
            headerEdges++; // Skip the "header" pulse (high 80 us)
        } else {
            bitTimer = mTimers.beginStopwatch();
            if (!pinStNew) { // Falling edge
                if (bitDuration > thresholdZeroMinUs && bitDuration < thresholdZeroMaxUs) {
                    // Detected a '0' bit
                    mBuffer[bitNum / 8] = mBuffer[bitNum / 8] & ~(1 << (7 - (bitNum % 8)));
                    bitNum++;
                } else if (bitDuration > thresholdOneMinUs && bitDuration < thresholdOneMaxUs) {
                    // Detected a '1' bit
                    mBuffer[bitNum / 8] = mBuffer[bitNum / 8] | (1 << (7 - (bitNum % 8)));
                    bitNum++;
                } else {
                    // Glitch?
                    glitchNum++;
                }
            }
        }
        pinSt = pinStNew;
    }
    // Switch to output
    mGpio.setPinMode(mPin, IfGpio::MODE_OUT, true);
    mGpio.setPin(mPin, true);

    if (bitNum < (sensorMsgLenB * 8)) {
        os_printf("AM2302: Failed, read timed out after %u us! Got %u bits, %u glitches\n", readDuration, bitNum, glitchNum);
        retVal = false;
    } else {
        //os_printf("Read finished after %u us, %u bits, %u glitches\n", readDuration, bitNum, glitchNum);
        //os_printf("Buffer: %02X%02X %02X%02X %02X\n", mBuffer[0], mBuffer[1], mBuffer[2], mBuffer[3], mBuffer[4]);
        uint8_t calcCrc = (uint8_t)(mBuffer[0] + mBuffer[1] + mBuffer[2] + mBuffer[3]);
        if (calcCrc == mBuffer[4]) {
            mHumidity = (mBuffer[0] << 8) + mBuffer[1];
            mTemperature = (mBuffer[2] << 8) + mBuffer[3];
            retVal =  true;
        } else {
            os_printf("AM2302: Failed, checksum mismatch! Calc=0x%02X got=0x%02X\n", calcCrc, mBuffer[4]);
            retVal =  false;
        }
    }

    if (!retVal || glitchNum) {
        mDiag.readGlitches += glitchNum;
        if (!retVal) {
            mDiag.readFailures++;
        }
        os_printf("AM2302: Glitches: %u, buffer: 0x%02X%02X 0x%02X%02X 0x%02X\n"
            , glitchNum
            , mBuffer[0]
            , mBuffer[1]
            , mBuffer[2]
            , mBuffer[3]
            , mBuffer[4]
        );
    }
    return retVal;
}

void ICACHE_FLASH_ATTR DriverAm2302::setPinWait(bool value, uint32_t durationUs) const {
    mGpio.setPin(mPin, value);
    if (durationUs > 0) {
        mTimers.delay(durationUs);
    }
}