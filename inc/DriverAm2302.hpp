/**
@file DriverAm2302.hpp
@brief Declare a driver for AM2302/DHT22 temperature and humidity sensor
*/

#ifndef _DRIVER_AM2302_HPP_
#define _DRIVER_AM2302_HPP_

#include "IfSensorTempHumidity.hpp"

class IfGpio;

class DriverAm2302 : public IfSensorTempHumidity {
public:
    DriverAm2302(IfGpio& gpio) 
    : mPort(0)
    , mGpio(gpio)
    , mHumidity(0)
    , mTemperature(0)
    { }
    bool init(unsigned int port);
    virtual bool update();
    virtual unsigned int getHumidity() const { return mHumidity; }
    virtual int getTemperature() const { return mTemperature; }
private:
    unsigned int mPort;
    IfGpio& mGpio;
    unsigned int mHumidity;
    int mTemperature;
};

#endif // _DRIVER_AM2302_HPP_