/**
@file IfGpio.hpp
@brief Interface to a GPIO
*/

#ifndef _IF_GPIO_HPP_
#define _IF_GPIO_HPP_



class IfGpio {
public:
    typedef enum {
        PIN0 = 0,
        PIN1,
        PIN2,
    } Pin;
    typedef enum {
        MODE_IN,
        MODE_OUT
    } Mode;
    virtual bool init();
    virtual void setPinMode(Pin pin, Mode mode);
    virtual Mode getPinMode(Pin pin) const;
    virtual void setPin(Pin pin);
    virtual bool getPin(Pin pin) const;
};

#endif // _IF_GPIO_HPP_