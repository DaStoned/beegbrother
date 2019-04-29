/**
@file IfTimers.hpp
@brief Interface to system timers
*/

#ifndef _IF_TIMERS_HPP_
#define _IF_TIMERS_HPP_

#include <stdint.h>

class IfTimers {
public:
    typedef uint32_t Timespan;
    /**
    Blocking delay
    @param us Delay duration in microseconds
    */
    virtual void delay(uint32_t us) const;
    virtual uint32_t getSystemTime() const;
    virtual Timespan beginStopwatch() const;
    virtual Timespan endStopwatch(Timespan start) const;
};

#endif // _IF_TIMERS_HPP_