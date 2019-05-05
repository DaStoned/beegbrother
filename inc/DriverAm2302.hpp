/**
@file DriverAm2302.hpp
@brief Declare a driver for AM2302/DHT22 temperature and humidity sensor
*/

#ifndef _DRIVER_AM2302_HPP_
#define _DRIVER_AM2302_HPP_

#include "IfSensorTempHumidity.hpp"
#include "IfGpio.hpp"

#include <stdint.h>

class IfTimers;

class DriverAm2302 : public IfSensorTempHumidity {
public:
    // The length of the AM2302 message in bytes
    static const unsigned int sensorMsgLenB = 5;

    DriverAm2302(IfGpio& gpio, IfTimers& timers) 
    : mPin(IfGpio::FIRST_PIN_UNUSED)
    , mGpio(gpio)
    , mTimers(timers)
    , mHumidity(0)
    , mTemperature(0)
    , mDiag{}
    , mBuffer{}
    { }
    bool init(IfGpio::Pin pin);
    virtual bool update();
    virtual unsigned int getHumidity() const { return mHumidity; }
    virtual int getTemperature() const { return mTemperature; }
    virtual void getDiagInfo(DiagInfo* pInfoOut) const { *pInfoOut = mDiag; }
private:
    void setPinWait(bool value, uint32_t durationUs = 0) const;
    IfGpio::Pin mPin;
    IfGpio& mGpio;
    IfTimers& mTimers;
    unsigned int mHumidity;
    int mTemperature;
    DiagInfo mDiag;
    uint8_t mBuffer[sensorMsgLenB];
};

#endif // _DRIVER_AM2302_HPP_