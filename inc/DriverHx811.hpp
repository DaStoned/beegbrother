/**
@file DriverHx811.hpp
@brief Declare a driver for a load sensor implemented with HX711 24-bit ADC

We use GPIO bitbanging to implement the SPI like protocol
*/

#ifndef _DRIVER_HX811_HPP_
#define _DRIVER_HX811_HPP_

#include "IfSensorLoad.hpp"
#include "IfGpio.hpp"

#include <stdint.h>

class IfTimers;

class DriverHx811 : public IfSensorLoad {
public:
    static const unsigned int sensorMsgLenB = 3;
    static const unsigned int minSampleIntervalUs = 50000;
    /// HX811 output settling time is 400 ms at 10 Hz sampling interval.
    /// This is approximately the time it takes to wake up from sleep and 
    /// take a sample.
    static const unsigned int spanSettleOutput = 500000;

    DriverHx811(IfGpio& gpio, IfTimers& timers) 
    : mPinClk(IfGpio::FIRST_PIN_UNUSED)
    , mPinData(IfGpio::FIRST_PIN_UNUSED)
    , mGpio(gpio)
    , mTimers(timers)
    , mLoad(0)
    , mDiag{}
    , mBuffer(0)
    , mInSleep(false)
    { }
    bool init(IfGpio::Pin pinClk, IfGpio::Pin pinData);
    virtual bool canUpdate() const;
    virtual bool update();
    virtual int getLoad() const { return mLoad; }
    virtual void getDiagInfo(DiagInfo* pInfoOut) const { *pInfoOut = mDiag; }
    virtual void toSleep();
    virtual void wakeFromSleep();
private:
    void setPinWait(bool value, uint32_t durationUs = 0) const;
    IfGpio::Pin mPinClk, mPinData;
    IfGpio& mGpio;
    IfTimers& mTimers;
    int mLoad;
    DiagInfo mDiag;
    uint32_t mBuffer;
    bool mInSleep;
};

#endif // _DRIVER_HX811_HPP_