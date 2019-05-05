/**
@file IfSensorLoad.hpp
@brief Interface to a H-bridge load sensor
*/

#ifndef _IF_SENSOR_LOAD_HPP_
#define _IF_SENSOR_LOAD_HPP_

class IfSensorLoad {
public:
    /**
    Diagnostic information about the performance of this 
    driver instance
    */
    typedef struct {
        /// Count of all read failures
        unsigned int readFailures;
        /// Count of all bit read glitches (pulses with invalid timing)
        unsigned int readGlitches;
    } DiagInfo;
    /**
    There's a frequency limitation to how often the sensor is allowed to sample
    @return true if we can update the sensor readings
    */
    virtual bool canUpdate() const = 0;
    /**
    Sample the sensor and update stored readings
    @return true if read was successful, false otherwise
    */
    virtual bool update() = 0;
    /**
    @return Weight in grams
    */
    virtual int getLoad() const = 0;
    /**
    Retrieve diagnostic information from the driver
    @param pInfoOut Pointer to structure which receives the diagnostic info
    */
    virtual void getDiagInfo(DiagInfo* pInfoOut) const = 0;
};

#endif // _IF_SENSOR_LOAD_HPP_