#include "Arduino.h"
#include "tools/tools.hpp"
#include "tools/log.hpp"
#include "status.hpp"

#define FIRMWARE_VERSION      "LightKit v1.6"

/* MODULE DEFINITION (Use #define or #undef) */
#define MODULE_TEMPERATURE   
#undef MODULE_DOMOTICZ   
#define MODULE_STATUS_LED       
#define MODULE_BUTTON    
#undef MODULE_RELAY            
#undef MODULE_BUZZER           
#undef MODULE_WEBSERVER        
#undef MODULE_STRIPLED         
#define MODULE_CMD_SERIAL           

/* MODULE TEMPERATURE */
#define TEMP_1_WIRE_PIN                         2
#define TEMP_SENSOR_RESOLUTION                  10
#define TEMP_MAX_SENSOR_SUPPORTED               2
#define TEMP_POLLING_PERIOD_MS                  1*60*1000        

/* MODULE_DOMOTICZ */
#define DOMOTICZ_HOST                           "192.168.0.32"
#define DOMOTICZ_PORT                           8080
#define DOMOTICZ_TIMEOUT_MS                     3*1000
#define DOMOTICZ_SENSOR_ID_INSIDE               3
#define DOMOTICZ_SENSOR_ID_OUTSIDE              4

/* MODULE_STATUS_LED */
#define STATUS_LED_TYPE                         STATUS_LED_TYPE_NEOPIXEL  /** LED Type, see constant below */
#define STATUS_LED_TYPE_CLASSIC                 0                     /** Constant for STATUS_LED_TYPE */
#define STATUS_LED_TYPE_NEOPIXEL                1                     /** Constant for STATUS_LED_TYPE */

#if STATUS_LED_TYPE == STATUS_LED_TYPE_NEOPIXEL
#define STATUS_LED_NEOPIXEL_PIN                 12                    /** If Neopixel, indicates the NeoPixel Pin */
#define STATUS_LED_NB_LED                       1                     /** Number of led in the neopixel chain */
#define STATUS_LED_NEOPIXEL_BRIGHTNESS          100                   /** Brightness of the neopixel LED [0-255] */
#define STATUS_LED_NEOPIXEL_RED                 255                   /** Brightness of red led */
#define STATUS_LED_NEOPIXEL_GREEN               255                   /** Brightness of green led */
#define STATUS_LED_NEOPIXEL_BLUE                255                   /** Brightness of blue led */
#elif STATUS_LED_TYPE == STATUS_LED_TYPE_NEOPIXEL
#define STATUS_LED_POWER_PIN                    -1                    /** If Classic, indicates the POWER Pin */
#define STATUS_LED_FAULT_PIN                    -1                    /** If Classic, indicates the FAULT Pin */
#endif

/* MODULE_BUTTON */
#define BUTTON_PIN                              0                    /** Define the pin of the button */
#define BUTTON_LONG_HOLD_TIME                   3*1000               /** Hold time in ms for long press on button */

/* MODULE_RELAY */
#define RELAY_FEEDBACK_PIN                      0                     /** Define the input to use to get the feedback information */
#define RELAY_CHECK_PERIOD                      60*1000               /** Delays between 2 relay checks in ms */
#define RELAY_CHECK_BEFORE_ERROR                3                     /** Try n times to resend commmand before going to error */
#undef RELAY_IS_BISTABLE                                              /** Define this if the relay is bistable, undef it for monostable */
#ifdef RELAY_IS_BISTABLE
#define RELAY_BISTABLE_ON_TIME_MS               30                    /** Duration of the active state for bistable relay */
#define RELAY_CMD_PIN_1                         2                     /** Define the output to use for bistable relay command */
#define RELAY_CMD_PIN_2                         2                     /** Define the output to use for bistable relay command */
#else
#define RELAY_CMD_PIN                           2                     /** Define the output to use for monostable relay command */
#endif

/* MODULE_STRIPLED */
#define STRIPLED_PIN                            2                     /** Number of Pixel on the strip */
#define STRIPLED_NB_PIXELS                      59                    /** Number of Pixel on the strip */
#define STRIPLED_NB_BRIGHT_LEVEL                10                    /** Number of level in the brightness table */
#define STRIPLED_BRIGHT_HYSTERESIS              10                    /** Hysteresis value between two levels (Base 1023) */
#define STRIPLED_AUTO_BRIGHT_UPDATE_PERIOD      2000                  /** Period of update of the auto brightness (in ms) */
#define STRIPLED_DEFAULT_BRIGHTNESS_VALUE       32                    /** Brightness level used at startup [0-255] */
#define STRIPLED_DEFAULT_ANIMATION_ID           10                    /** ID of the animation used at startup */
#define STRIPLED_DEMO_MODE_PERIOD               5000                  /** Delay betwwen two animation in demo mode (in ms) */
#define LIGHT_SENSOR_PIN                        -1                    /** Pin for Light sensor used to ajust output light level (-1 if not used) */

/** SCRIPT */
#define SCRIPT_DOMOTICZ_UPT_PERIOD              10*60*1000
#define SCRIPT_TEMP_CHECK_PERIOD                10*1000
#define SCRIPT_TEMP_ALERT_HYSTERESIS            1
#define SCRIPT_TEMP_ALERT_FRIDGE                27
#define SCRIPT_TEMP_ALERT_FREEZER               30
#define SCRIPT_RELAY_IMPULSION_DURATION         3*1000                /** Duration of the impulsion when the alert is triggered */

#ifdef ESP32
#define G_WebServer ESP32WebServer
#else
#define G_WebServer ESP8266WebServer
#endif
