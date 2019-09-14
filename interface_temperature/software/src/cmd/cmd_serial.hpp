#ifndef CMD_CMD_SERIAL_H
#define CMD_CMD_SERIAL_H

#include "global.hpp"

#define ERROR_UNKNOWN_CMD     1
#define ERROR_WRONG_LENGTH    2
#define ERROR_BUFF_OVERRUN    3
#define ERROR_SPURIOUS        4
#define ERROR_WRONG_VALUE     5

#define SERIAL_RX_BUFFER_SIZE 64
#define SERIAL_END_CMD_1      '\n'
#define SERIAL_END_CMD_2      '\r'
#define SERIAL_ABORT_CMD      0x08  // Backspace

struct serial_t {
  char rxBuffer[SERIAL_RX_BUFFER_SIZE];
  uint16_t rxLength;
  bool isInCmdValid;
};

void cmd_serial_init(void);
void cmd_serial_main(void);

#endif /* CMD_CMD_SERIAL_H */
