/**
  * @file   stripled.hpp
  * @brief  Manage led animations on Neopixel strips
  * @author David DEVANT
  * @date   12/08/2019
  */

#ifndef STRIPLED_STRIPLED_H
#define STRIPLED_STRIPLED_H

#include <Arduino.h>

/** Constants for refreshPeriod (The lower the faster the refresh will be) */
#define STRIPLED_MAX_REFRESH_PERIOD      0
#define STRIPLED_LOWPOWER_REFRESH_PERIOD (10 * 1000)

/** Data structure for a level */
struct brightLevel_t {
	uint8_t  output; /** Output intensity (O-255) */
	uint16_t low;    /** Low value of input luminosity (0-1023) */
	uint16_t high;   /** High value of input luminosity (0-1023) */
};

typedef struct {
	uint8_t a; // MSB
	uint8_t r;
	uint8_t g;
	uint8_t b; // LSB
} rgba_t;

typedef union {
	rgba_t   rgba;
	uint32_t u32;
} rgba_u;

typedef struct {
	uint8_t nbLed;        /**< Led count on the strip [1; STRIPLED_NB_PIXELS] */
	rgba_u  color;        /**< Color of the strip in plain color mode */
	uint8_t brightness;   /**< Brightness of the strip [0; 255] */
	uint8_t animID;       /**< Animation index [0; ws2812fx.getModeCount()-1] */
	bool    isInDemoMode; /**< Tell if demo mode is enabled */
	bool    isOn;         /**< Tell if stripled is turned on */
} stripled_params_t;

// StripLed
void    brightness_set(uint8_t brightness);
void    brightness_auto_set(void);
void    nb_led_set(uint8_t nbLed);
void    color_set(const rgba_u * color);
int32_t set_animation(uint8_t animID);
void    stripled_set_demo_mode(bool isDemoModeEn);
void    stripled_set_state(bool isOn);

int  stripled_init(void);
void stripled_main(void);

#endif /* STRIPLED_STRIPLED_H */
