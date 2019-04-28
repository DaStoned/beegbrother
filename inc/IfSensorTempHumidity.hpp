/**
@file IfSensorTempHumidity.hpp
@brief Interface to a temperature and humidity sensor
*/

#ifndef _IF_SENSOR_TEMP_HUMIDITY_HPP_
#define _IF_SENSOR_TEMP_HUMIDITY_HPP_

class IfSensorTempHumidity {
public:
    virtual bool update();
    virtual unsigned int getHumidity() const;
    virtual int getTemperature() const;
};

#endif // _IF_SENSOR_TEMP_HUMIDITY_HPP_