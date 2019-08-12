#ifndef LED_VISU_LED_VISU_H
#define LED_VISU_LED_VISU_H

#include "global.hpp"

#define NB_LED_VISU     2

struct ledVisu_t {
  uint32_t nextChange;  // date of the next changement
  uint32_t timeOn;      // time on in ms
  uint32_t timeOff;     // time off in ms
  bool isOn;            // true: on, false: off
  uint8_t pin;          // GPIO number of the output
};

void ledVisu_init(void);
void ledVisu_main(void);

#endif /* LED_VISU_LED_VISU_H */
