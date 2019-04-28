/**
@file main.c
@brief Entry point for beegbrother application
*/

#include "DriverAm2302.hpp"

bool DriverAm2302::init(unsigned int port) {
    mPort = port;
    return true;
}