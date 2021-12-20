/**
  * @file   tools.hpp
  * @brief  Store useful functions and macros
  * @author David DEVANT
  * @date   03/07/2018
  */

#ifndef SRC_TOOLS_HPP_
#define SRC_TOOLS_HPP_

#include <cstdint>
#include <cstdlib>

#define OK 0

#define TIME_SECOND (1000)
#define TIME_MINUTE (60 * TIME_SECOND)
#define TIME_HOUR   (60 * TIME_MINUTE)
#define TIME_DAY    (24 * TIME_HOUR)

#define POW_2(x) (x * x)
#define POW_3(x) (x * x * x)
#define POW_4(x) (POW_2(x) * POW_2(x))

#define _set(base, flag)     (base) |= (flag);
#define _unset(base, flag)   (base) &= ~(flag);
#define _isset(base, flag)   (((base) & (flag)) != 0)
#define _isunset(base, flag) (((base) & (flag)) == 0)

#define _strncmp(str1, strCst) strncmp(argv[0], strCst, sizeof(strCst))

#define CHECK_CALL(call)              \
	if (call) {                       \
		log_error(#call " failed()"); \
		return -1;                    \
	}

uint16_t sToU16(const char * str, uint16_t length);

#endif /* SRC_TOOLS_HPP_ */
