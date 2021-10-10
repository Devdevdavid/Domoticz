#ifndef BUZZER_HPP
#define BUZZER_HPP

#include "global.hpp"

#ifdef MODULE_BUZZER

int  buzzer_init(void);
void buzzer_stop(uint8_t buzzerId);
void buzzer_set_melody(uint8_t buzzerId, uint8_t melodyId, bool isRepeatEnabled);
void buzzer_main(void);

#endif /* MODULE_BUZZER */
#endif /* BUZZER_HPP */