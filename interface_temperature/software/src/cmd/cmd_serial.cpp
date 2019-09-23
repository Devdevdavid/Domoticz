#include "cmd_serial.hpp"
#include "cmd.hpp"

#ifdef MODULE_CMD_SERIAL

struct serial_t serialPort;

#define _cmd_serial_ack(errorCode) cmd_serial_ack(#errorCode, errorCode)
void cmd_serial_ack(const char errorCodeStr[], uint8_t errorCode)
{
  if (errorCode == OK) {
    Serial.println("CMD OK");
  } else {
    Serial.printf("CMD ERROR: Code = %d (%s)\n", errorCode, errorCodeStr);
  }
}

void cmd_serial_echo(char outChar)
{
  Serial.write(outChar);
}

void cmd_serial_abort(void)
{
  Serial.println("\nABORT");
  cmd_serial_init();
}

void add_char_to_rxBuffer(char inChar) {
  if (serialPort.rxLength < SERIAL_RX_BUFFER_SIZE) {
    serialPort.rxBuffer[serialPort.rxLength++] = inChar;
    cmd_serial_echo(inChar);
  } else {
    _cmd_serial_ack(ERROR_BUFF_OVERRUN);
    cmd_serial_abort();
  }
}

void cmd_serial_receive(void)
{
  char inChar;

  while (Serial.available() > 0) {
    inChar = toupper(Serial.read());

    if ((inChar == SERIAL_END_CMD_1) || (inChar == SERIAL_END_CMD_2)) {
      // Launch an help is nothing received before an end cmd
      if (serialPort.rxLength == 0) {
        add_char_to_rxBuffer('H');
      }
      serialPort.isInCmdValid = true;
    } else if (inChar == SERIAL_ABORT_CMD) {
      cmd_serial_abort();
    } else {
      add_char_to_rxBuffer(inChar);
    }
  }
}

void cmd_serial_execute(void)
{
  // Get a new line for awnser
  Serial.println("");
  switch (serialPort.rxBuffer[0]) {
    case 'H':
      cmd_print_help();
      break;
    case 'T':
      cmd_print_status();
      break;
    case 'L':
      if (serialPort.rxLength == 1) {
        cmd_set_status_led(!_isset(STATUS_APPLI, STATUS_APPLI_STATUS_LED));
      } else if (serialPort.rxLength == 2) {
        cmd_set_status_led(sToU16(&serialPort.rxBuffer[1], 1));
      } else {
        _cmd_serial_ack(ERROR_WRONG_LENGTH);
        break;
      }
      _cmd_serial_ack(OK);
      break;
#ifdef MODULE_STRIPLED
    case 'B':
      if (serialPort.rxLength < 1) {
        _cmd_serial_ack(ERROR_WRONG_LENGTH);
      } else if (serialPort.rxLength == 1) {
        cmd_set_brightness_auto(true);
        _cmd_serial_ack(OK);
      } else {
        uint8_t brightness = sToU16(&serialPort.rxBuffer[1], serialPort.rxLength - 1);
        if (brightness <= 100) {
          cmd_set_brightness(brightness);
          _cmd_serial_ack(OK);
        } else {
          _cmd_serial_ack(ERROR_WRONG_VALUE);
        }
      }
      break;
#endif
#ifdef MODULE_STRIPLED
    /** SWITCH: ON/OFF */
    case 'S':
      if (serialPort.rxLength < 1) {
        _cmd_serial_ack(ERROR_WRONG_LENGTH);
      } else if (serialPort.rxLength == 1) {
        /** TOGGLE STATE */
        cmd_set_state(!cmd_get_state());
        _cmd_serial_ack(OK);
      } else {
        if (serialPort.rxBuffer[1] == '1') {
          /** SET to ON */
          cmd_set_state(true);
          _cmd_serial_ack(OK);
        } else if (serialPort.rxBuffer[1] == '0') {
          /** SET to OFF */
          cmd_set_state(false);
          _cmd_serial_ack(OK);
        } else {
          _cmd_serial_ack(ERROR_WRONG_VALUE);
        }
      }
      break;
#endif
#ifdef MODULE_STRIPLED
    /** DEMO MODE: ON/OFF */
    case 'D':
      if (serialPort.rxLength < 1) {
        _cmd_serial_ack(ERROR_WRONG_LENGTH);
      } else if (serialPort.rxLength == 1) {
        /** TOGGLE STATE */
        cmd_set_demo_mode(!cmd_get_demo_mode());
        _cmd_serial_ack(OK);
      } else {
        if (serialPort.rxBuffer[1] == '1') {
          /** SET to ON */
          cmd_set_demo_mode(true);
          _cmd_serial_ack(OK);
        } else if (serialPort.rxBuffer[1] == '0') {
          /** SET to OFF */
          cmd_set_demo_mode(false);
          _cmd_serial_ack(OK);
        } else {
          _cmd_serial_ack(ERROR_WRONG_VALUE);
        }
      }
      break;
#endif
#ifdef MODULE_STRIPLED  
    /** ANIMATION */
    case 'A':
      if (serialPort.rxLength <= 1) {
        _cmd_serial_ack(ERROR_WRONG_LENGTH);
      } else {
        uint16_t animID = sToU16(&serialPort.rxBuffer[1], serialPort.rxLength - 1);
        if (cmd_set_animation(animID) == OK) {
          _cmd_serial_ack(OK);
        } else {
          _cmd_serial_ack(ERROR_WRONG_VALUE);
        }
      }
      break;
#endif
#ifdef MODULE_RELAY
    case 'R':
      if (serialPort.rxLength == 1) {
        relay_set_state(!relay_get_state());
      } else if (serialPort.rxLength == 2) {
        relay_set_state(sToU16(&serialPort.rxBuffer[1], 1));
      } else {
        _cmd_serial_ack(ERROR_WRONG_LENGTH);
        break;
      }
      _cmd_serial_ack(OK);
      break;
#endif
    default:
      _cmd_serial_ack(ERROR_UNKNOWN_CMD);
      break;
  }
  cmd_serial_init();
}

void cmd_serial_init(void)
{
  serialPort.rxLength = 0;
  serialPort.isInCmdValid = 0;
}

void cmd_serial_main(void)
{
  if (Serial.available() > 0) {
    cmd_serial_receive();

    if (serialPort.isInCmdValid) {
      cmd_serial_execute();
    }
  }
}

#endif
