/**
@file Scale.cpp
@brief Implement a simple scale using the HX711 ADC and load sensors in H bridge
*/

#include "Scale.hpp"
extern "C" {
    // Include the C-only SDK headers
    #include "osapi.h"
    #include "user_interface.h"
}

void ICACHE_FLASH_ATTR Scale::tare() { 
    setTare(readLoadSensor()); 
    os_printf("Taring at %d\n", getTare());
}

double ICACHE_FLASH_ATTR Scale::getWeight() const {
    return (double) (readLoadSensor() - mTareOffset) / mCalib;
}

double ICACHE_FLASH_ATTR Scale::readLoadSensor() const {
    while (!mAdc.canUpdate()) {
        //os_printf(".");
    }
    if (!mAdc.update()) {
        //os_printf("Scale: Failed to update ADC!\n");
    } else {
        //os_printf("Scale: ADC value %d\n", mAdc.getLoad());
    }
    return mAdc.getLoad();
}