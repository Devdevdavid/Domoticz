/**
  * @file   stripled.hpp
  * @brief  Manage led animations on Neopixel strips
  * @author David DEVANT
  * @date   12/08/2019
  */

#ifndef STRIPLED_STRIPLED_H
#define STRIPLED_STRIPLED_H

#include <Arduino.h>

/** Data structure for a level */
struct brightLevel_t {
  uint8_t output;     /** Output intensity (O-255) */
  uint16_t low;       /** Low value of input luminosity (0-1023) */
  uint16_t high;      /** High value of input luminosity (0-1023) */
};

// StripLed
void brightness_set(uint8_t brightness);
void brightness_auto_set(void);
void nb_led_set(uint8_t nbLed);
void color_set(uint32_t color);
int32_t set_animation(uint8_t animID);
void stripled_set_demo_mode(bool isDemoModeEn);
void stripled_set_state(bool isOn);

void brightness_table_init(void);
void stripled_init(void);
void stripled_main(void);

#endif /* STRIPLED_STRIPLED_H */
