/**
@file DriverGpio.hpp
@brief Declare a driver for GPIO
*/

#ifndef _DRIVER_GPIO_HPP_
#define _DRIVER_GPIO_HPP_

#include "IfGpio.hpp"

class DriverGpio : public IfGpio {
public:
    DriverGpio() 
    { }
    virtual bool init();
    virtual void setPinMode(Pin pin, Mode mode);
    virtual Mode getPinMode(Pin pin) const;
    virtual void setPin(Pin pin, bool value);
    virtual bool getPin(Pin pin) const;
private:

};

#endif // _DRIVER_GPIO_HPP_