/**
@file Timers.hpp
@brief Declare a module for accessing timers
*/

#ifndef _TIMERS_HPP_
#define _TIMERS_HPP_

#include "IfTimers.hpp"

class Timers : public IfTimers {
public:
    Timers() 
    { }
    virtual void delay(uint32_t us) const;
    virtual uint32_t getSystemTimeUs() const;
    virtual Timespan beginStopwatch() const;
    virtual Timespan readStopwatch(Timespan start) const;
private:

};

#endif // _TIMERS_HPP_