/*
 * tools.c
 *
 *  Created on: Oct 3, 2018
 *      Author: David
 */

#include "tools.hpp"

uint16_t sToU16(const char * str, uint16_t length)
{
  char tmpStr[11];
  uint16_t index;

  for (index = 0; index < length; index++) {
    tmpStr[index] = str[index];
  }
  tmpStr[length] = '\0';

  return strtol(tmpStr, NULL, 10);
}
