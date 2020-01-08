#ifndef STATUS_H
#define STATUS_H

#include "global.hpp"

#define NB_STATUS               10

#ifdef MAIN_C
uint8_t boardStatus[NB_STATUS];
#else
extern uint8_t boardStatus[NB_STATUS];
#endif

/* STATUS LIST */
#define STATUS_APPLI                    boardStatus[0]  /** Flag on application status */
#define STATUS_WIFI                     boardStatus[1]  /** Flag on wifi connection  */
#define STATUS_BRIGHTNESS               boardStatus[2]  /** Value of brightness currently set */
#define STATUS_BUTTON                   boardStatus[3]  /** Flag on button states */
#define STATUS_BRIGHT_LVL               boardStatus[4]  /** Level of brightness currently set */
#define STATUS_ANIM                     boardStatus[5]  /** ID of the anim currently used */
#define STATUS_NB_LED                   boardStatus[6]  /** RAM copy of EEPROM_NB_LED_ADDRESS */
#define STATUS_COLOR_R                  boardStatus[7]  /** RAM copy of EEPROM_NB_COLOR_R */
#define STATUS_COLOR_G                  boardStatus[8]  /** RAM copy of EEPROM_NB_COLOR_G */
#define STATUS_COLOR_B                  boardStatus[9]  /** RAM copy of EEPROM_NB_COLOR_B */

/* STATUS_APPLI */
#define STATUS_APPLI_LED_VISU           0x01
#define STATUS_APPLI_ERROR              0x02
#define STATUS_APPLI_AUTOLUM            0x04
#define STATUS_APPLI_TEMP_FAULT         0x08
#define STATUS_APPLI_LED_IS_ON          0x10
#define STATUS_APPLI_DEMO_MODE          0x20

/* STATUS_WIFI */
#define STATUS_WIFI_IS_CO               0x01
#define STATUS_WIFI_DEVICE_CO           0x02

/* STATUS_BUTTON */
#define STATUS_BUTTON_RISING            0x01
#define STATUS_BUTTON_FALLING           0x02
#define STATUS_BUTTON_LONG_HOLD         0x04

void status_init(void);

#endif /* STATUS_H */
