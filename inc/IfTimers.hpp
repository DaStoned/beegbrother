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
    /// Declare stopwatch context
    typedef uint32_t Timespan;
    /**
    Blocking delay
    @param us Delay duration in microseconds
    */
    virtual void delay(uint32_t us) const = 0;
    /**
    @return system time in microseconds (starts from 0 on boot)
    */
    virtual uint32_t getSystemTimeUs() const = 0;
    /**
    Begin measuring time using current system time
    @return Stopwatch context
    */
    virtual Timespan beginStopwatch() const = 0;
    /**
    Get time elapsed since measurement was started
    @param start Stopwatch context from beginStopwatch()
    @return Time elapsed in microseconds
    */
    virtual unsigned int readStopwatch(Timespan start) const = 0;
};

#endif // _IF_TIMERS_HPP_