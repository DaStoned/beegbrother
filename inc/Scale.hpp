/**
@file Scale.hpp
@brief Declare a simple scale using the HX711 ADC and load sensors in H bridge
*/

#ifndef _SCALE_HPP_
#define _SCALE_HPP_

#include "DriverHx711.hpp"

class Scale {
public:
    Scale(DriverHx711& adc, int calib, int tareOffset = 0)
        : mAdc(adc)
        , mCalib(calib)
        , mTareOffset(tareOffset)
    {}
    void setCalib(int calib) { mCalib = calib; }
    int getCalib() const { return mCalib; }
    void setTare(int tare) { mTareOffset = tare; }
    int getTare() const { return mTareOffset; }
    void tare();
    double getWeight(void) const;
private:
    int readLoadSensor() const;
    DriverHx711& mAdc;
    int mCalib;
    int mTareOffset;
};

#endif // _SCALE_HPP_
