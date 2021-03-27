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
#define ERROR_INTERNAL     6

#define TERM_RX_BUFFER_SIZE   64
#define TERM_END_CMD          '\n'
#define TERM_KEY_BACKSPACE    0x08 // Backspace
#define TERM_KEY_ENTER        0x0D
#define TERM_KEY_ESC          0x1B
#define TERM_KEY_OPEN_BRACKET 0x5B // '['
#define TERM_CODE_ARROW_UP    0x41 // 'A'
#define TERM_CODE_ARROW_DOWN  0x42 // 'B'
#define TERM_CODE_ARROW_RIGHT 0x43 // 'C'
#define TERM_CODE_ARROW_LEFT  0x44 // 'D'
#define TERM_ABORT_CMD        TERM_KEY_BACKSPACE

void term_rx(uint8_t byte);
void term_print(String str);
int  term_init(void);
void term_main(void);

#endif /* CMD_TERM_HPP */