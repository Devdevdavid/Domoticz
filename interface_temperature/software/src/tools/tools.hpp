/*
 * tools.h
 *
 *  Created on: Oct 3, 2018
 *      Author: David
 */

#ifndef SRC_TOOLS_HPP_
#define SRC_TOOLS_HPP_

#include <cstdint>
#include <cstdlib>

#define OK      0

#define POW_2(x)		(x * x)
#define POW_3(x)		(x * x * x)
#define POW_4(x)		(POW_2(x) * POW_2(x))

#define _set(base, flag)    (base) |= (flag);
#define _unset(base, flag)  (base) &= ~(flag);
#define _isset(base, flag)  (((base) & (flag)) != 0)

uint16_t sToU16(const char * str, uint16_t length);

#endif /* SRC_TOOLS_HPP_ */
