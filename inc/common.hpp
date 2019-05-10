/**
@file common.hpp
@brief Frequently required macros and functions
*/

#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace common {

int abs(int i);
char* double_snprintf3(char* buf, unsigned int bufLen, double d);

} // namespace common
#endif // _COMMON_HPP_