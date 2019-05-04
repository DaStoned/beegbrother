/**
@file DriverAm2302.hpp
@brief Declare a driver for AM2302/DHT22 temperature and humidity sensor
*/

#ifndef _DRIVER_AM2302_HPP_
#define _DRIVER_AM2302_HPP_

#include "IfSensorTempHumidity.hpp"
#include "IfGpio.hpp"

#include <stdint.h>

// The length of the AM2302 message in bytes
#define AM2302_MSG_LEN_B  5

class IfTimers;

class DriverAm2302 : public IfSensorTempHumidity {
public:
    DriverAm2302(IfGpio& gpio, IfTimers& timers) 
    : mPin(IfGpio::FIRST_PIN_UNUSED)
    , mGpio(gpio)
    , mTimers(timers)
    , mHumidity(0)
    , mTemperature(0)
    , mBuffer{}
    { }
    bool init(IfGpio::Pin pin);
    virtual bool update();
    virtual unsigned int getHumidity() const { return mHumidity; }
    virtual int getTemperature() const { return mTemperature; }
private:
    void setPinWait(bool value, uint32_t durationUs = 0) const;
    IfGpio::Pin mPin;
    IfGpio& mGpio;
    IfTimers& mTimers;
    unsigned int mHumidity;
    int mTemperature;
    uint8_t mBuffer[AM2302_MSG_LEN_B];
};

#endif // _DRIVER_AM2302_HPP_