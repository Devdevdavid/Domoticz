#ifndef STATUS_H
#define STATUS_H

#include "global.hpp"

#define NB_STATUS               7

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
#define STATUS_SCRIPT                   boardStatus[6]  /** Script values */

/* STATUS_APPLI */
#define STATUS_APPLI_STATUS_LED         0x01
#define STATUS_APPLI_ERROR              0x02
#define STATUS_APPLI_AUTOLUM            0x04
#define STATUS_APPLI_TEMP_1_FAULT       0x08
#define STATUS_APPLI_TEMP_2_FAULT       0x10
#define STATUS_APPLI_LED_IS_ON          0x20
#define STATUS_APPLI_DEMO_MODE          0x40
#define STATUS_APPLI_RELAY_FAULT        0x80

/* STATUS_WIFI */
#define STATUS_WIFI_IS_CO               0x01
#define STATUS_WIFI_DEVICE_CO           0x02
#define STATUS_WIFI_DOMOTICZ_FAULT      0x04

/* STATUS_BUTTON */
#define STATUS_BUTTON_RISING            0x01
#define STATUS_BUTTON_FALLING           0x02
#define STATUS_BUTTON_LONG_HOLD         0x04

/* STATUS_SCRIPT */
#define STATUS_SCRIPT_IN_ALERT          0x01

void status_init(void);

#endif /* STATUS_H */
