#include "Arduino.h"
#include "tools/tools.hpp"
#include "tools/log.hpp"
#include "status.hpp"

#define FIRMWARE_VERSION      "LightKit v1.4"

/* PERIPH DEFINITION (-1: Not used) */
#define LED_POWER_PIN       -1
#define LED_FAULT_PIN       -1
#define BUTTON_PIN          -1
#define LIGHT_SENSOR_PIN    -1
#define STRIP_LED_PIN       2

/* PARAMETERS */
#define BTN_LONG_HOLD_TIME  3000  // in ms

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

// Only for shortcut
#define IS_LED_VISU_EN      ((STATUS_APPLI & STATUS_APPLI_LED_VISU) != 0)
