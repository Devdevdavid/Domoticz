#if (LED_POWER_PIN != -1)

#include "ledVisu.hpp"

struct ledVisu_t ledVisu[NB_LED_VISU];
extern uint32_t tick;

void ledVisu_init(void)
{
  ledVisu[0].pin = LED_POWER_PIN;
  ledVisu[1].pin = LED_FAULT_PIN;

  for (uint8_t index = 0; index < NB_LED_VISU; index++) {
    ledVisu[index].nextChange = 0;
    ledVisu[index].timeOn = 500;
    ledVisu[index].timeOff = 500;
    ledVisu[index].isOn = true;
    pinMode(ledVisu[index].pin, OUTPUT);
    digitalWrite(ledVisu[index].pin, LOW);
  }
}

void ledVisu_main(void)
{
  uint8_t index;

  /* POWER LED */
  /*if (STATUS_APPLI & STATUS_APPLI_) {

  } else*/ {
    ledVisu[0].timeOn = 100;
    ledVisu[0].timeOff = 0;
  }

  /* FAULT LED */
  if (!_isset(STATUS_WIFI, STATUS_WIFI_IS_CO)) {
    ledVisu[1].timeOn = 300;
    ledVisu[1].timeOff = 500;
  } else if (_isset(STATUS_APPLI, STATUS_APPLI_TEMP_FAULT)) {
    ledVisu[1].timeOn = 200;
    ledVisu[1].timeOff = 1800;
  } else if (_isset(STATUS_APPLI, STATUS_APPLI_ERROR)) {
    ledVisu[1].timeOn = 100;
    ledVisu[1].timeOff = 0;
  } else {
    ledVisu[1].timeOn = 0;
    ledVisu[1].timeOff = 100;
  }

  for (index = 0; index < NB_LED_VISU; index++) {
    if (tick >= ledVisu[index].nextChange) {
      if (ledVisu[index].isOn) {
        if (ledVisu[index].timeOff != 0) {
          ledVisu[index].nextChange = tick + ledVisu[index].timeOff;
          ledVisu[index].isOn = false;
        } else {
          ledVisu[index].nextChange = tick + ledVisu[index].timeOn;
        }
      } else {
        if (ledVisu[index].timeOn != 0) {
          ledVisu[index].nextChange = tick + ledVisu[index].timeOn;
          ledVisu[index].isOn = true;
        } else {
          ledVisu[index].nextChange = tick + ledVisu[index].timeOff;
        }
      }
    }
  }

  for (index = 0; index < NB_LED_VISU; index++) {
    digitalWrite(ledVisu[index].pin,
      (ledVisu[index].isOn && IS_LED_VISU_EN) ? LOW : HIGH);
  }
}

#endif