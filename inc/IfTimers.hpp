/**
@file IfTimers.hpp
@brief Interface to system timers
*/

#ifndef _IF_TIMERS_HPP_
#define _IF_TIMERS_HPP_

#include <cstdint>
#include <climits>

class IfTimers {
public:
    typedef uint32_t Timespan;
    /**
    Blocking delay
    @param us Delay duration in microseconds
    */
    virtual void delay(uint32_t us) const = 0;
    virtual uint32_t getSystemTimeUs() const = 0;
    virtual Timespan beginStopwatch() const = 0;
    virtual Timespan readStopwatch(Timespan start) const = 0;
};

#endif // _IF_TIMERS_HPP_