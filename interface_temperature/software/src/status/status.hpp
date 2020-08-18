/**
  * @file   status.hpp
  * @brief  Handle statuses of the board
  * @author David DEVANT
  * @date   12/08/2019
  */

#ifndef STATUS_H
#define STATUS_H

#include "global.hpp"

#define NB_STATUS 11

#ifdef STATUS_CPP
uint8_t boardStatus[NB_STATUS];
#else
extern uint8_t boardStatus[NB_STATUS];
#endif

/* STATUS LIST */
#define STATUS_APPLI      boardStatus[0]  /** Flag on application status */
#define STATUS_WIFI       boardStatus[1]  /** Flag on wifi connection  */
#define STATUS_BRIGHTNESS boardStatus[2]  /** Value of brightness currently set */
#define STATUS_BRIGHT_LVL boardStatus[3]  /** Level of brightness currently set */
#define STATUS_ANIM       boardStatus[4]  /** ID of the anim currently used */
#define STATUS_SCRIPT     boardStatus[5]  /** Script values */
#define STATUS_NB_LED     boardStatus[6]  /** Number of LED on the ledstrip */
#define STATUS_TEMP       boardStatus[7]  /** Flag for temp module */
#define STATUS_COLOR_R    boardStatus[8]  /** Default animation color (red) */
#define STATUS_COLOR_G    boardStatus[9]  /** Default animation color (green) */
#define STATUS_COLOR_B    boardStatus[10] /** Default animation color (blue) */

/* STATUS_APPLI */
#define STATUS_APPLI_STATUS_LED  0x01
#define STATUS_APPLI_ERROR       0x02
#define STATUS_APPLI_AUTOLUM     0x04
#define STATUS_APPLI_LED_IS_ON   0x20
#define STATUS_APPLI_DEMO_MODE   0x40
#define STATUS_APPLI_RELAY_FAULT 0x80

/* STATUS_WIFI */
#define STATUS_WIFI_IS_CO          0x01
#define STATUS_WIFI_DEVICE_CO      0x02
#define STATUS_WIFI_DOMOTICZ_FAULT 0x04

/* STATUS_TEMP */
#define STATUS_TEMP_1_FAULT 0x01
#define STATUS_TEMP_2_FAULT 0x02

/* STATUS_SCRIPT */
#define STATUS_SCRIPT_IN_ALERT 0x01

void status_init(void);

#endif /* STATUS_H */
