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
        PIN3,
        PIN4,
        PIN5,
        PIN6,
        PIN7,
        PIN8,
        PIN9,
        PIN10,
        PIN11,
        PIN12,
        PIN13,
        PIN14,
        PIN15,
        // PIN16, Seems to have a special purpose?

        FIRST_PIN_UNUSED ///< Must remain last in list
    } Pin;
    typedef enum {
        MODE_IN,        ///< Input, no pullup
        MODE_IN_PULLUP, ///< Input with pullup
        MODE_OUT        ///< Output
    } Mode;
    virtual bool init() = 0;
    virtual void setPinMode(Pin pin, Mode mode, bool defaultOut = false) = 0;
    virtual Mode getPinMode(Pin pin) const = 0;
    virtual void setPin(Pin pin, bool value) = 0;
    virtual bool getPin(Pin pin) const = 0;
};

#endif // _IF_GPIO_HPP_