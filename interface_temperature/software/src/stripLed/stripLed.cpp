#include "stripLed.hpp"
#include <WS2812FX.h>

#ifdef MODULE_STRIPLED

// Get the tick from main
extern uint32_t tick;

// Instanciate the library
WS2812FX ws2812fx = WS2812FX(STRIPLED_NB_PIXELS, STRIPLED_PIN, NEO_GRB + NEO_KHZ800);

// Create the brightness table and animation Table
struct brightLevel_t brightTable[STRIPLED_NB_BRIGHT_LEVEL];

// Next tick of animation change and brightness update
uint32_t demoTick;
uint32_t autoBrightTick;

/********************************
 *          Tools
 ********************************/

void brightness_set(uint8_t brightness)
{
  STATUS_BRIGHTNESS = brightness;

  // Apply new value
  ws2812fx.setBrightness(brightness);
}

#if (LIGHT_SENSOR_PIN != -1)
/**
 * Read the sensor value and determine which
 * brightness level is approriate
 */
void brightness_auto_set(void)
{
	uint32_t val;

  val = 1023 - analogRead(LIGHT_SENSOR_PIN);

  while (val > brightTable[STATUS_BRIGHT_LVL].high) {
    ++STATUS_BRIGHT_LVL;
  }
  while (val < brightTable[STATUS_BRIGHT_LVL].low) {
    --STATUS_BRIGHT_LVL;
  }

  brightness_set(brightTable[STATUS_BRIGHT_LVL].output);
}
#endif

/**
 * Define the animation to show with specific PARAMETERS
 * @param  animID   ID of the animation to Show
 * @return          0: OK, -1: Wrong AnimID
 */
int32_t set_animation(uint8_t animID)
{
  // Check animID
  if (animID >= ws2812fx.getModeCount()) {
    return -1; // Wrong animID
  }

  log_info("Setting mode %d: %s", animID, ws2812fx.getModeName(animID));
  ws2812fx.setMode(animID);

  // Define this animation as current
  STATUS_ANIM = animID;

  return OK;
}

/**
 * Enable/Disable demo mode
 * @param isDemoModeEn : bool
 */
void stripLed_set_demo_mode(bool isDemoModeEn)
{
  if (isDemoModeEn) {
    _set(STATUS_APPLI, STATUS_APPLI_DEMO_MODE);
    // Force refresh to now
    demoTick = 0;
  } else {
    _unset(STATUS_APPLI, STATUS_APPLI_DEMO_MODE);
  }
}

/**
 * Define the state of the LED strip (ON or OFF)
 */
void stripLed_set_state(bool isOn)
{
  if (isOn) {
    _set(STATUS_APPLI, STATUS_APPLI_LED_IS_ON);
  } else {
    _unset(STATUS_APPLI, STATUS_APPLI_LED_IS_ON);
    ws2812fx.strip_off();
  }
}

/********************************
 *          Init
 ********************************/

/**
 * Init the brightness table
 */
void brightness_table_init(void)
{
  for (uint8_t index = 0; index < STRIPLED_NB_BRIGHT_LEVEL; index++) {
    brightTable[index].output = 10 + (index * 245) / (STRIPLED_NB_BRIGHT_LEVEL - 1);
    brightTable[index].low = (index * 1023) / (STRIPLED_NB_BRIGHT_LEVEL - 1);
    if (index > 0) {
      brightTable[index].low -= STRIPLED_BRIGHT_HYSTERESIS / 2;
    }
    brightTable[index].high = ((1 + index) * 1023) / (STRIPLED_NB_BRIGHT_LEVEL - 1);
    if (index < (STRIPLED_NB_BRIGHT_LEVEL - 1)) {
      brightTable[index].high += STRIPLED_BRIGHT_HYSTERESIS / 2;
    }
  }
  STATUS_BRIGHT_LVL = 0;
}

/**
 * Init the strip led
 */
void stripLed_init(void)
{
  brightness_table_init();

  /** Init the led driver */
  ws2812fx.init();
  ws2812fx.setSpeed(500);
  ws2812fx.setColor(255, 255, 255);
  ws2812fx.start();

  autoBrightTick = tick;

  // Go into demo mode at startup
  set_animation(STRIPLED_DEFAULT_ANIMATION_ID);
  brightness_set(STRIPLED_DEFAULT_BRIGHTNESS_VALUE);
  stripLed_set_state(true);
  stripLed_set_demo_mode(true);
}

/**
 * Main function of stripLed
 * Should be executed every 1ms
 */
void stripLed_main(void)
{
  if (_isset(STATUS_APPLI, STATUS_APPLI_LED_IS_ON)) {
    // Refresh strip display
    ws2812fx.service();
  }

  // Update brightness level
  if (_isset(STATUS_APPLI, STATUS_APPLI_DEMO_MODE)) {
    if (tick >= demoTick) {
      demoTick = tick + STRIPLED_DEMO_MODE_PERIOD;

      if (STATUS_ANIM == (ws2812fx.getModeCount() - 1)) {
        set_animation(0);
      } else {
        set_animation(STATUS_ANIM + 1);
      }
    }
  }

#if (LIGHT_SENSOR_PIN != -1)
  // Update brightness level
  if (tick >= autoBrightTick) {
    autoBrightTick = tick + AUTO_BRIGHT_UPDATE_PERIOD;

    // Update the brightness if AUTOLUMIN is set
    if (_isset(STATUS_APPLI, STATUS_APPLI_AUTOLUM)) {
      brightness_auto_set();
    }
  }
#endif
}

#endif /* MODULE_STRIPLED */