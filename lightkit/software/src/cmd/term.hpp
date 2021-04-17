/**
  * @file   term.hpp
  * @brief  Terminal management
  * @author David DEVANT
  * @date   19/12/2020
  */

#ifndef CMD_TERM_HPP
#define CMD_TERM_HPP

#include <Arduino.h>

void term_rx(uint8_t byte);
void term_print(String str);
int  term_init(void);

#endif /* CMD_TERM_HPP */