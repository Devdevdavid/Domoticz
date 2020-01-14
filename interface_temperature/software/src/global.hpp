#include <Arduino.h>
#include "tools/tools.hpp"
#include "tools/log.hpp"
#include "status.hpp"

#define LIGHTKIT_VERSION      "LightKit v1.7.0"

/**
 * This is the configuration for the board all the boards before RING
 * This exists for compatibility reasons
 */
#ifdef BOARD_TEMP_DOMOTICZ

#if defined(BOARD_TEMP_DOMOTICZ_RELAY)
#define FIRMWARE_VERSION      LIGHTKIT_VERSION" - TEMP. DOMOTICZ RELAY"
#elif defined(BOARD_TEMP_DOMOTICZ_BUZZER)
#define FIRMWARE_VERSION      LIGHTKIT_VERSION" - TEMP. DOMOTICZ BUZZER"
#endif

/* MODULE DEFINITION (Use #define or #undef) */
#define MODULE_TEMPERATURE   
#define MODULE_DOMOTICZ   
#define MODULE_STATUS_LED       
#define MODULE_INPUTS
// 2 version of this BOARD_TEMP_DOMOTICZ
// One with a buzzer
#ifdef BOARD_TEMP_DOMOTICZ_BUZZER     
    #define MODULE_OUTPUTS
#else
    #undef MODULE_OUTPUTS
#endif       
// The other with a Relay
#ifdef BOARD_TEMP_DOMOTICZ_RELAY
    #define MODULE_RELAY
#else
    #undef MODULE_RELAY
#endif
#undef MODULE_WEBSERVER        
#undef MODULE_STRIPLED         
#undef MODULE_CMD_SERIAL           

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
#define STATUS_LED_NEOPIXEL_PIN                 14                    /** If Neopixel, indicates the NeoPixel Pin */
#define STATUS_LED_NB_LED                       1                     /** Number of led in the neopixel chain */
#define STATUS_LED_NEOPIXEL_BRIGHTNESS          100                   /** Brightness of the neopixel LED [0-255] */
#define STATUS_LED_NEOPIXEL_RED                 255                   /** Brightness of red led */
#define STATUS_LED_NEOPIXEL_GREEN               255                   /** Brightness of green led */
#define STATUS_LED_NEOPIXEL_BLUE                255                   /** Brightness of blue led */

/* MODULE_INPUTS */
#define INPUTS_COUNT                           1                     /** Number of inputs managed by the module */
#define INPUTS_PINS                            {0}                   /** Define the pin of the input with following format: {x, y, z} */
#define INPUTS_LONG_HOLD_TIME                  3*1000                /** Hold time in ms for long press on input */
// Aliases
#define INPUTS_OPT_RELAY_IMPULSION_MODE        0                     /** Option jumper JP1 is set as the first button in the module */

/* MODULE_OUTPUTS */
#define OUTPUTS_COUNT                           2                    /** Number of outputs managed by the module */
#define OUTPUTS_PINS                            {12}                 /** Define the pin of the output with following format: {x, y, z} */
// Aliases
#define OUTPUTS_BUZZER                          0                    /** Output for the buzzer is the first output declared above */

/* MODULE_RELAY */
#define RELAY_FEEDBACK_PIN                      16                    /** Define the input to use to get the feedback information */
#define RELAY_CHECK_PERIOD                      60*1000               /** Delays between 2 relay checks in ms */
#define RELAY_CHECK_BEFORE_ERROR                3                     /** Try n times to resend commmand before going to error */
#undef RELAY_IS_BISTABLE                                              /** Define this if the relay is bistable, undef it for monostable */
#define RELAY_CMD_PIN                           12                    /** Define the output to use for monostable relay command */

/* MODULE_BUZZER */
#define BUZZER_PIN                              12                    /** Pin of the connected buzzer */

/** SCRIPT */
#define SCRIPT_DOMOTICZ_UPT_PERIOD              10*60*1000
#define SCRIPT_TEMP_CHECK_PERIOD                10*1000
#define SCRIPT_TEMP_ALERT_HYSTERESIS            1
#define SCRIPT_TEMP_ALERT_FRIDGE                27
#define SCRIPT_TEMP_ALERT_FREEZER               30
#define SCRIPT_RELAY_IMPULSION_DURATION         3*1000                /** Duration of the impulsion when the alert is triggered */
#define SCRIPT_RELAY_MS_BEFORE_2ND_IMPULSION    2*1000                /** Duration in ms before sending a second impulsion when alert is turning on (Set to 0 to disable 2nd impulsion) */

/** Compilation checks */
#if defined(BOARD_TEMP_DOMOTICZ_RELAIS) && defined(BOARD_TEMP_DOMOTICZ_BUZZER)
    #error BOARD_TEMP_DOMOTICZ_RELAIS cannot be defined with BOARD_TEMP_DOMOTICZ_BUZZER
#endif

#endif /* BOARD_TEMP_DOMOTICZ */

/**
 * This is the configuration for the board 
 * RING
 * with neopixel support and some options pin
 */
#ifdef BOARD_RING

#define FIRMWARE_VERSION      LIGHTKIT_VERSION" - RING"

/* MODULE DEFINITION (Use #define or #undef) */
#undef MODULE_TEMPERATURE   
#undef MODULE_DOMOTICZ   
#undef MODULE_STATUS_LED       
#define MODULE_INPUTS
#undef MODULE_OUTPUTS
#undef MODULE_RELAY            
#define MODULE_WEBSERVER        
#define MODULE_STRIPLED         
#undef MODULE_CMD_SERIAL           

/* MODULE_STRIPLED */
#define STRIPLED_PIN                            2                     /** Output pin for the strip command */
#define STRIPLED_NB_PIXELS                      59                    /** Number of Pixel on the strip */
#define STRIPLED_NB_BRIGHT_LEVEL                10                    /** Number of level in the brightness table */
#define STRIPLED_BRIGHT_HYSTERESIS              10                    /** Hysteresis value between two levels (Base 1023) */
#define STRIPLED_AUTO_BRIGHT_UPDATE_PERIOD      2000                  /** Period of update of the auto brightness (in ms) */
#define STRIPLED_DEFAULT_BRIGHTNESS_VALUE       32                    /** Brightness level used at startup [0-255] */
#define STRIPLED_DEFAULT_ANIMATION_ID           10                    /** ID of the animation used at startup */
#define STRIPLED_DEMO_MODE_PERIOD               5000                  /** Delay betwwen two animation in demo mode (in ms) */
#define STRIPLED_MAX_NB_PIXELS                  100                   /** Maximum number of LED that can be supported */
#define LIGHT_SENSOR_PIN                        -1                    /** Pin for Light sensor used to ajust output light level (-1 if not used) */

/* MODULE_INPUTS */
#define INPUTS_COUNT                           2                     /** Number of inputs managed by the module */
#define INPUTS_PINS                            {0, 4}                /** Define the pin of the input with following format: {x, y, z} */
#define INPUTS_LONG_HOLD_TIME                  3*1000                /** Hold time in ms for long press on input */
// Aliases
#define INPUTS_OPT_WEB_SERVER_DISPLAY          0                     /** Option jumper JP1 is set as the first button in the module */
#define INPUTS_PIR_DETECTOR                    1                     /** The PIR detector is set as the second button in the module */

/** CUSTOM DEFINE for this board */
#define DETECTOR_ENABLED                       1                    /** Enable (1) or disbale (0) the PIR Detector option */
#define DETECTOR_ON_DURATION_MIN               30                   /** Duration of the ON State after detector found someone (in minute) */
#define DETECTOR_INVERSE_POLARITY              0                    /** Inverse the polarity of the detector input: 0 -> goes HIGH on trigger, 1 -> goes LOW on trigger */                   

/** Compilation checks */
#if !defined(MODULE_INPUTS)
    #error BOARD_RING require MODULE_INPUTS
#endif

#endif /* BOARD_RING */

/**
 * This is the configuration for the board all the boards before RING
 * This exists for compatibility reasons
 */
#ifdef BOARD_ANY

#define FIRMWARE_VERSION      LIGHTKIT_VERSION" - ANY"

/* MODULE DEFINITION (Use #define or #undef) */
#define MODULE_TEMPERATURE   
#undef MODULE_DOMOTICZ   
#define MODULE_STATUS_LED       
#define MODULE_INPUTS  
#undef MODULE_OUTPUTS           
#undef MODULE_RELAY            
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
#define STATUS_LED_TYPE                         STATUS_LED_TYPE_NEOPIXEL  /** LED Type, STATUS_LED_TYPE_NEOPIXEL or STATUS_LED_TYPE_CLASSIC */

#if STATUS_LED_TYPE == STATUS_LED_TYPE_NEOPIXEL
#define STATUS_LED_NEOPIXEL_PIN                 14                    /** If Neopixel, indicates the NeoPixel Pin */
#define STATUS_LED_NB_LED                       1                     /** Number of led in the neopixel chain */
#define STATUS_LED_NEOPIXEL_BRIGHTNESS          100                   /** Brightness of the neopixel LED [0-255] */
#define STATUS_LED_NEOPIXEL_RED                 255                   /** Brightness of red led */
#define STATUS_LED_NEOPIXEL_GREEN               255                   /** Brightness of green led */
#define STATUS_LED_NEOPIXEL_BLUE                255                   /** Brightness of blue led */
#elif STATUS_LED_TYPE == STATUS_LED_TYPE_CLASSIC
#define STATUS_LED_POWER_PIN                    -1                    /** If Classic, indicates the POWER Pin */
#define STATUS_LED_FAULT_PIN                    -1                    /** If Classic, indicates the FAULT Pin */
#endif

/* MODULE_INPUTS */
#define INPUTS_COUNT                           2                     /** Number of inputs managed by the module */
#define INPUTS_PINS                            {0}                   /** Define the pin of the input with following format: {x, y, z} */
#define INPUTS_LONG_HOLD_TIME                  3*1000                /** Hold time in ms for long press on input */
// Aliases
#define INPUTS_OPT_RELAY_IMPULSION_MODE        0                     /** Option jumper JP1 is set as the first button in the module */

/* MODULE_OUTPUTS */
#define OUTPUTS_COUNT                           2                    /** Number of outputs managed by the module */
#define OUTPUTS_PINS                            {12}                 /** Define the pin of the output with following format: {x, y, z} */

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

/* MODULE_BUZZER */
#define BUZZER_PIN                              12                    /** Pin of the connected buzzer */

/* MODULE_STRIPLED */
#define STRIPLED_PIN                            2                     /** Number of Pixel on the strip */
#define STRIPLED_NB_PIXELS                      59                    /** Number of Pixel on the strip */
#define STRIPLED_NB_BRIGHT_LEVEL                10                    /** Number of level in the brightness table */
#define STRIPLED_BRIGHT_HYSTERESIS              10                    /** Hysteresis value between two levels (Base 1023) */
#define STRIPLED_AUTO_BRIGHT_UPDATE_PERIOD      2000                  /** Period of update of the auto brightness (in ms) */
#define STRIPLED_DEFAULT_BRIGHTNESS_VALUE       32                    /** Brightness level used at startup [0-255] */
#define STRIPLED_DEFAULT_ANIMATION_ID           10                    /** ID of the animation used at startup */
#define STRIPLED_DEMO_MODE_PERIOD               5000                  /** Delay betwwen two animation in demo mode (in ms) */
#define STRIPLED_MAX_NB_PIXELS                  100                   /** Maximum number of LED that can be supported */
#define LIGHT_SENSOR_PIN                        -1                    /** Pin for Light sensor used to ajust output light level (-1 if not used) */

/** SCRIPT */
#define SCRIPT_DOMOTICZ_UPT_PERIOD              10*60*1000
#define SCRIPT_TEMP_CHECK_PERIOD                10*1000
#define SCRIPT_TEMP_ALERT_HYSTERESIS            1
#define SCRIPT_TEMP_ALERT_FRIDGE                27
#define SCRIPT_TEMP_ALERT_FREEZER               30
#define SCRIPT_RELAY_IMPULSION_DURATION         3*1000                /** Duration of the impulsion when the alert is triggered */
#define SCRIPT_RELAY_MS_BEFORE_2ND_IMPULSION    2*1000                /** Duration in ms before sending a second impulsion when alert is turning on (Set to 0 to disable 2nd impulsion) */

#endif /* BOARD_ANY */

/* 
 * COMMON TO ALL BOARDS 
 */
#ifdef ESP32
    #define G_WebServer ESP32WebServer
#else
    #define G_WebServer ESP8266WebServer
#endif

/** Types of Led status */
#define STATUS_LED_TYPE_CLASSIC                 0                     /** Constant for STATUS_LED_TYPE */
#define STATUS_LED_TYPE_NEOPIXEL                1                     /** Constant for STATUS_LED_TYPE */


/** EEPROM used size in bytes */
#define EEPROM_USED_SIZE                        4
#define EEPROM_NB_LED_ADDRESS                   0
#define EEPROM_COLOR_R_ADDRESS                  1
#define EEPROM_COLOR_G_ADDRESS                  2
#define EEPROM_COLOR_B_ADDRESS                  3