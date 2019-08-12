#include "cmd.hpp"

void cmd_print_help(void)
{
  Serial.println("=== " FIRMWARE_VERSION " ===");
  Serial.println("H                         This help");
  Serial.println("D                         Print board status");
  Serial.println("L{State}                  Toggle or set Led Visu Visibility");
  Serial.println("B[Value]                  Set brightness, no value = Auto");
  Serial.println("S[1/0: State]             Set the LED ON or OFF");
  Serial.println("A<ID> <Period> [Params]   Set animation");
  Serial.println("> 0: Therm <Nb LED>");
  Serial.println("> 1: ShotGun");
  Serial.println("> 2: Equalizer");
  Serial.println("> 3: Full Color <Red> <Green> <Blue>");
}

void cmd_print_status(void)
{
  Serial.printf("APPLI:   0x%02X\n", STATUS_APPLI);
  Serial.printf("WIFI:    0x%02X\n", STATUS_WIFI);
  Serial.printf("BRIGHT:  %d%%", (STATUS_BRIGHTNESS * 100) / 255);
  if (_isset(STATUS_APPLI, STATUS_APPLI_AUTOLUM)) {
    Serial.printf(" AUTO\n");
  } else {
    Serial.printf("\n");
  }
  Serial.printf("BUTTON:  0x%02X\n", STATUS_BUTTON);
  Serial.printf("LEVEL:   %d\n", STATUS_BRIGHT_LVL);
  Serial.printf("ANIM:    %d\n", STATUS_ANIM);
}

void cmd_set_led_visu(uint8_t isEnabled)
{
  if (isEnabled) {
    _set(STATUS_APPLI, STATUS_APPLI_LED_VISU);
  } else {
    _unset(STATUS_APPLI, STATUS_APPLI_LED_VISU);
  }
}

/**
 * Set/Reset brightness in automayic mode
 * @param newValue [description]
 */
void cmd_set_brightness_auto(bool newValue)
{
  if (newValue) {
    _set(STATUS_APPLI, STATUS_APPLI_AUTOLUM);
  } else {
    _unset(STATUS_APPLI, STATUS_APPLI_AUTOLUM);
  }
}

/**
 * Set the output brightness value and
 * disable auto bright if set
 * @param newValue [0-100]
 */
void cmd_set_brightness(uint8_t newValue)
{
  if (newValue >= 0 && newValue <= 100) {
    brightness_set(((uint16_t) (newValue * 255)) / 100);
    cmd_set_brightness_auto(false);
  }
}

/**
 * Get the output brightness value
 * @param newValue [0-100]
 */
uint8_t cmd_get_brightness(void)
{
  return ((uint16_t) (STATUS_BRIGHTNESS * 100)) / 255;
}

/**
 * Get the LED State
 * @return  0: LED are OFF, 1: LED are ON
 */
bool cmd_get_state(void)
{
  return _isset(STATUS_APPLI, STATUS_APPLI_LED_IS_ON);
}

/**
 * Set the state of LED
 * @param  state    [description]
 * @return          [description]
 */
int32_t cmd_set_state(bool state)
{
  stripLed_set_state(state);
  return 0;
}

/**
 * Get the demo mode
 * @return  0: Is not in demo, 1: Is in demo
 */
bool cmd_get_demo_mode(void)
{
  return _isset(STATUS_APPLI, STATUS_APPLI_DEMO_MODE);
}

/**
 * Set the demo mode of LED
 * @param  isDemoMode    bool
 * @return          [description]
 */
int32_t cmd_set_demo_mode(bool isDemoMode)
{
  stripLed_set_demo_mode(isDemoMode);
  return 0;
}

/**
 * Call get_animation()
 * @return  animID The animation ID to use
 */
uint8_t cmd_get_animation(void)
{
  return STATUS_ANIM;
}

/**
 * Call set_animation()
 * @param  animID The animation ID to use
 * @return          See set_animation()
 */
int32_t cmd_set_animation(uint8_t animID)
{
  return set_animation(animID);
}
