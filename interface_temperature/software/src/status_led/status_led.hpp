#ifndef STATUS_LED_STATUS_LED_HPP
#define STATUS_LED_STATUS_LED_HPP

#include "global.hpp"

typedef struct status_led_state_t {
  uint8_t isOn:1;            // true: on, false: off
  uint8_t hasChanged:1;      // true: on, false: off
  uint8_t reserved:6;
} status_led_state_t;

struct status_led_t {
  uint32_t nextChange;  // date of the next changement
  uint32_t timeOn;      // time on in ms
  uint32_t timeOff;     // time off in ms
  char color;           // Only NeoPixel: Color of the LED
  uint8_t pin;          // Only Classic: GPIO number of the output
  status_led_state_t state;
};

void status_led_set_color(char color);
void status_led_turnoff(void);
void status_led_init(void);
void status_led_main(void);

#endif /* STATUS_LED_STATUS_LED_HPP */
