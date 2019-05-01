/**
@file IfSensorTempHumidity.hpp
@brief Interface to a temperature and humidity sensor
*/

#ifndef _IF_SENSOR_TEMP_HUMIDITY_HPP_
#define _IF_SENSOR_TEMP_HUMIDITY_HPP_

class IfSensorTempHumidity {
public:
    virtual bool update() = 0;
    virtual unsigned int getHumidity() const = 0;
    virtual int getTemperature() const = 0;
};

#endif // _IF_SENSOR_TEMP_HUMIDITY_HPP_