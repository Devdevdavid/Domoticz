/**
  * @file   prompt.hpp
  * @brief  Prompt management for terminal
  * @author David DEVANT
  * @date   26/03/2021
  */

#ifndef CMD_PROMPT_HPP
#define CMD_PROMPT_HPP

#include <Arduino.h>

#define PROMPT_LINE_BUFFER_SIZE 64
#define PROMPT_END_CMD          '\n'
#define PROMPT_KEY_BACKSPACE    0x08 // Backspace
#define PROMPT_KEY_ENTER_UNIX   0x0D
#define PROMPT_KEY_ENTER_WIN    0x0A
#define PROMPT_KEY_ESC          0x1B
#define PROMPT_KEY_OPEN_BRACKET 0x5B // '['
#define PROMPT_CODE_ARROW_UP    0x41 // 'A'
#define PROMPT_CODE_ARROW_DOWN  0x42 // 'B'
#define PROMPT_CODE_ARROW_RIGHT 0x43 // 'C'
#define PROMPT_CODE_ARROW_LEFT  0x44 // 'D'

String  prompt_get_line(void);
uint8_t prompt_get_cursor_pos(void);
int32_t prompt_rx(uint8_t byte);
void    prompt_init(void);

#endif /* CMD_PROMPT_HPP */