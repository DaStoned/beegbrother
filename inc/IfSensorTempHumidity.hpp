/**
 * @file IfSensorTempHumidity.hpp
 * @brief Interface to a temperature and humidity sensor
*/

#ifndef _IF_SENSOR_TEMP_HUMIDITY_HPP_
#define _IF_SENSOR_TEMP_HUMIDITY_HPP_

class IfSensorTempHumidity {
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
    Sample the sensor and update stored readings
    @return true if read was successful, false otherwise
    */
    virtual bool update() = 0;
    /**
    @return relative humidity (in units of 1/10 percent) from most recent update
    */
    virtual unsigned int getHumidity() const = 0;
    /**
    @return temperature (in units of 1/10 C) from most recent update
    */
    virtual int getTemperature() const = 0;
    /**
    Retrieve diagnostic information from the driver
    @param pInfoOut Pointer to structure which receives the diagnostic info
    */
    virtual void getDiagInfo(DiagInfo* pInfoOut) const = 0;
};

#endif // _IF_SENSOR_TEMP_HUMIDITY_HPP_