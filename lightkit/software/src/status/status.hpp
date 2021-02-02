/**
  * @file   status.hpp
  * @brief  Handle statuses of the board
  * @author David DEVANT
  * @date   12/08/2019
  */

#ifndef STATUS_H
#define STATUS_H

#include "global.hpp"

#define NB_STATUS 5

#ifdef STATUS_CPP
uint8_t boardStatus[NB_STATUS];
#else
extern uint8_t boardStatus[NB_STATUS];
#endif

/* STATUS LIST */
#define STATUS_APPLI      boardStatus[0] /** Flag on application status */
#define STATUS_WIFI       boardStatus[1] /** Flag on wifi connection  */
#define STATUS_BRIGHT_LVL boardStatus[2] /** Level of brightness currently set */
#define STATUS_SCRIPT     boardStatus[3] /** Script values */
#define STATUS_TEMP       boardStatus[4] /** Flag for temp module */

/* STATUS_APPLI */
#define STATUS_APPLI_STATUS_LED  0x01
#define STATUS_APPLI_ERROR       0x02
#define STATUS_APPLI_AUTOLUM     0x04
#define STATUS_APPLI_FILESYSTEM  0x08
#define STATUS_APPLI_LED_IS_ON   0x20
#define STATUS_APPLI_DEMO_MODE   0x40
#define STATUS_APPLI_RELAY_FAULT 0x80

/* STATUS_WIFI */
#define STATUS_WIFI_IS_CO             0x01
#define STATUS_WIFI_DEVICE_CO         0x02
#define STATUS_WIFI_USING_FORCED_MODE 0x04
#define STATUS_WIFI_DOMOTICZ_FAULT    0x08

/* STATUS_TEMP */
#define STATUS_TEMP_1_FAULT 0x01
#define STATUS_TEMP_2_FAULT 0x02

/* STATUS_SCRIPT */
#define STATUS_SCRIPT_IN_ALERT 0x01

int status_init(void);

#endif /* STATUS_H */
