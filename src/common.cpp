/**
@file common.cpp
@brief Frequently required macros and functions
*/

#include "common.hpp"

extern "C" {
    // Include the C-only SDK headers
    #include "osapi.h"
    #include "user_interface.h"
}

namespace common {

int ICACHE_FLASH_ATTR abs(int i) {
    return i < 0 ? -i : i;
}

char* ICACHE_FLASH_ATTR double_snprintf3(char* buf, unsigned int bufLen, double d) {
    os_snprintf(buf, bufLen, "%d.%03d", (int) d, abs((int) ((d - (int) d)*1000)));
    buf[bufLen - 1] = '\0';
    return buf;
}

} // namespace common
