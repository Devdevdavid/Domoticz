#include "Arduino.h"
#include "tools/tools.hpp"
#include "tools/log.hpp"
#include "status.hpp"

#define FIRMWARE_VERSION      "LightKit v1.5"

/* PERIPH DEFINITION (-1: Not used) */
#define LED_POWER_PIN       -1
#define LED_FAULT_PIN       -1
#define BUTTON_PIN          -1
#define LIGHT_SENSOR_PIN    -1
#define STRIP_LED_PIN       2

/* PARAMETERS */
/** Hold time in ms for long press on button */
#define BTN_LONG_HOLD_TIME       3000  // in ms
/** Brightness level used at startup [0-255] */
#define DEFAULT_BRIGHTNESS_VALUE 32
/** ID of the animation used at startup */
#define DEFAULT_ANIMATION_ID     10
/** Delay betwwen two animation in demo mode (in ms) */
#define DEMO_MODE_PERIOD          5000
/** Maximum number of LED allowed */
#define STRIP_LED_MAX_NB_PIXELS   100
/** EEPROM used size in bytes */
#define EEPROM_USED_SIZE          4

/* MACRO */
#if (LED_POWER_PIN != -1)
#define LED_FAULT_ON        digitalWrite(LED_FAULT_PIN, LOW);
#define LED_FAULT_OFF       digitalWrite(LED_FAULT_PIN, HIGH);
#define LED_POWER_ON        digitalWrite(LED_POWER_PIN, LOW);
#define LED_POWER_OFF       digitalWrite(LED_POWER_PIN, HIGH);
#else
#define LED_FAULT_ON        
#define LED_FAULT_OFF       
#define LED_POWER_ON        
#define LED_POWER_OFF       
#endif

#ifdef ESP32
#define G_WebServer ESP32WebServer
#else
#define G_WebServer ESP8266WebServer
#endif

// CONSTANTS
#define EEPROM_NB_LED_ADDRESS        0
#define EEPROM_COLOR_R_ADDRESS       1
#define EEPROM_COLOR_G_ADDRESS       2
#define EEPROM_COLOR_B_ADDRESS       3

// Only for shortcut
#define IS_LED_VISU_EN      ((STATUS_APPLI & STATUS_APPLI_LED_VISU) != 0)
