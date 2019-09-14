#ifndef STRIPLED_STRIPLED_H
#define STRIPLED_STRIPLED_H

#include "global.hpp"

/** Data structure for a level */
struct brightLevel_t {
  uint8_t output;     /** Output intensity (O-255) */
  uint16_t low;       /** Low value of input luminosity (0-1023) */
  uint16_t high;      /** High value of input luminosity (0-1023) */
};

// StripLed
void brightness_set(uint8_t brightness);
void brightness_auto_set(void);
int32_t set_animation(uint8_t animID);
void stripLed_set_demo_mode(bool isDemoModeEn);
void stripLed_set_state(bool isOn);

void brightness_table_init(void);
void stripLed_init(void);
void stripLed_main(void);

#endif /* STRIPLED_STRIPLED_H */
