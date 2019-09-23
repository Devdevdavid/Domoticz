#ifndef CMD_CMD_H
#define CMD_CMD_H

#include "global.hpp"
#include "status_led/status_led.hpp"
#include "stripLed/stripLed.hpp"
#include "relay/relay.hpp"

void cmd_print_help(void);
void cmd_print_status(void);
void cmd_set_status_led(uint8_t isEnabled);
void cmd_set_brightness_auto(bool newValue);
void cmd_set_brightness(uint8_t newValue);
uint8_t cmd_get_brightness(void);
void cmd_get_status(void);
bool cmd_get_state(void);
int32_t cmd_set_state(bool state);
bool cmd_get_demo_mode(void);
int32_t cmd_set_demo_mode(bool isDemoMode);
uint8_t cmd_get_animation(void);
int32_t cmd_set_animation(uint8_t animID);

#endif /* CMD_CMD_H */
