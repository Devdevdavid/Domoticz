/**
  * @file   term.hpp
  * @brief  Terminal management
  * @author David DEVANT
  * @date   19/12/2020
  */

#ifndef CMD_TERM_HPP
#define CMD_TERM_HPP

#include <Arduino.h>

#define ERROR_UNKNOWN_CMD  1
#define ERROR_WRONG_LENGTH 2
#define ERROR_BUFF_OVERRUN 3
#define ERROR_SPURIOUS     4
#define ERROR_WRONG_VALUE  5

#define TERM_RX_BUFFER_SIZE 64
#define TERM_END_CMD        '\n'
#define TERM_ABORT_CMD      0x08 // Backspace

void term_rx(uint8_t byte);
void term_print(String str);
void term_init(void);
void term_main(void);

#endif /* CMD_TERM_HPP */