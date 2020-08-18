/**
  * @file   tools.cpp
  * @brief  Store useful functions and macros
  * @author David DEVANT
  * @date   03/07/2018
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
