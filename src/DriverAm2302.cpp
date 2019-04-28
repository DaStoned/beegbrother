/**
@file DriverAm2302.cpp
@brief Implement a driver for AM2302/DHT22 temperature and humidity sensor
*/

#include "DriverAm2302.hpp"
#include "IfGpio.hpp"

bool DriverAm2302::init(unsigned int port) {
    mPort = port;
    return true;
}

bool DriverAm2302::update() {
    return true;
}