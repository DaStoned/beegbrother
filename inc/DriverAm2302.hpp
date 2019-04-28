/**
@file driver_am2302.hpp
@brief Declarations for AM2302/DHT22 driver
*/

#ifndef _DRIVER_AM2302_HPP_
#define _DRIVER_AM2302_HPP_

class GpioInterface;

class DriverAm2302 {
public:
    DriverAm2302(GpioInterface& gpio) 
    : mPort(0)
    , mGpio(gpio)
    { }
    bool init(unsigned int port);
private:
    unsigned int mPort;
    GpioInterface& mGpio;
};

#endif // _DRIVER_AM2302_HPP_