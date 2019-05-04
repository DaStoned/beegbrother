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

// Read usually takes 4-5 ms
#define READ_TIME_OUT_US    50000
// Duration of "0" pulse is 26-28 us
#define TRESHOLD_ZERO_MIN_US    10
#define TRESHOLD_ZERO_MAX_US    40
// Duration of "1" pulse is 70 us
#define TRESHOLD_ONE_MIN_US    60
#define TRESHOLD_ONE_MAX_US    90

bool ICACHE_FLASH_ATTR DriverAm2302::init(IfGpio::Pin pin) {
    mPin = pin;
    // Don't initialize the output pin or we'll activate the sensor
    // on every boot and take us out of phase for the first readout.
    return true;
}

bool ICACHE_FLASH_ATTR DriverAm2302::update() {
    bool pinSt = false, pinStNew = false;
    unsigned int headerEdges = 0, readDuration = 0, bitDuration, bitNum = 0, glitchNum = 0;
    IfTimers::Timespan readCycleTimer, bitTimer;
    os_printf("Starting data transfer on pin %u\n", (unsigned int) mPin);
    // Wake the sensor and request data transfer
    mGpio.setPinMode(mPin, IfGpio::MODE_OUT);
    setPinWait(true, 50);
    setPinWait(false, 2000);
    setPinWait(true, 40);
    mGpio.setPinMode(mPin, IfGpio::MODE_IN_PULLUP);
    pinSt = true;
    //os_printf("Read %u\n", pinSt ? 1 : 0);
    readCycleTimer = mTimers.beginStopwatch();
    bitTimer = mTimers.beginStopwatch();
    pinStNew = pinSt;
    while (readDuration < READ_TIME_OUT_US && bitNum < (AM2302_MSG_LEN_B * 8)) {
        pinStNew = mGpio.getPin(mPin);
        if (pinStNew != pinSt) {
            bitDuration = mTimers.readStopwatch(bitTimer);
            // Skip the header
            if (headerEdges < 3) {
                headerEdges++;
            } else {
                bitTimer = mTimers.beginStopwatch();
                if (!pinStNew) { // Falling edge
                    if (bitDuration > TRESHOLD_ZERO_MIN_US && bitDuration < TRESHOLD_ZERO_MAX_US) {
                        // Detected a '0' bit
                        mBuffer[bitNum / 8] = mBuffer[bitNum / 8] & ~(1 << (7 - (bitNum % 8)));
                        bitNum++;
                    } else if (bitDuration > TRESHOLD_ONE_MIN_US && bitDuration < TRESHOLD_ONE_MAX_US) {
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
        readDuration = mTimers.readStopwatch(readCycleTimer);
    }
    // Switch to output
    mGpio.setPinMode(mPin, IfGpio::MODE_OUT);
    mGpio.setPin(mPin, true);

    if (readDuration >= READ_TIME_OUT_US) {
        os_printf("Read timed out after %u us, %u bits, %u glitches\n", readDuration, bitNum, glitchNum);
        os_printf("Buffer: %02X%02X %02X%02X %02X\n", mBuffer[0], mBuffer[1], mBuffer[2], mBuffer[3], mBuffer[4]);
        return false;
    } else {
        os_printf("Read finished after %u us, %u bits, %u glitches\n", readDuration, bitNum, glitchNum);
        os_printf("Buffer: %02X%02X %02X%02X %02X\n", mBuffer[0], mBuffer[1], mBuffer[2], mBuffer[3], mBuffer[4]);
        if ((uint8_t)(mBuffer[0] + mBuffer[1] + mBuffer[2] + mBuffer[3]) == mBuffer[4]) {
            mHumidity = (mBuffer[0] << 8) + mBuffer[1];
            mTemperature = (mBuffer[2] << 8) + mBuffer[3];
            return true;
        } else {
            os_printf("Checksum mismatch\n");
            return false;
        }
    }
}

void ICACHE_FLASH_ATTR DriverAm2302::setPinWait(bool value, uint32_t durationUs) const {
    mGpio.setPin(mPin, value);
    if (durationUs > 0) {
        mTimers.delay(durationUs);
    }
}