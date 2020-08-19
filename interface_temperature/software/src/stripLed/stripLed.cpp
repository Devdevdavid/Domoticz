/**
  * @file   stripled.cpp
  * @brief  Manage led animations on Neopixel strips
  * @author David DEVANT
  * @date   12/08/2019
  */

#include "stripled.hpp"
#include "flash/flash.hpp"
#include "global.hpp"
#include "io/inputs.hpp"
#include <WS2812FX.h>
#ifdef ESP32
#include "ESP32_RMT_Driver.hpp"
#endif
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

	// Save settings in flash
	flash_write();
}

void nb_led_set(uint8_t nbLed)
{
	STATUS_NB_LED = nbLed;

	// Clear all the pixel before changing length
	ws2812fx.strip_off();

	// Apply new value
	ws2812fx.setLength(STATUS_NB_LED);

	// Save settings in flash
	flash_write();
}

void color_set(uint32_t color)
{
	STATUS_COLOR_R = (color >> 16) & 0xFF;
	STATUS_COLOR_G = (color >> 8) & 0xFF;
	STATUS_COLOR_B = color & 0xFF;

	log_info("Setting color to %02X - %02X - %02X", STATUS_COLOR_R, STATUS_COLOR_G, STATUS_COLOR_B);

	// Apply new value
	ws2812fx.setColor(color);

	// Save settings in flash
	flash_write();
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
void stripled_set_demo_mode(bool isDemoModeEn)
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
void stripled_set_state(bool isOn)
{
	log_info("Stripled is now %s", isOn ? "on" : "off");
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
		brightTable[index].low    = (index * 1023) / (STRIPLED_NB_BRIGHT_LEVEL - 1);
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
 * Custom show functions which will use the RMT hardware to drive the LEDs.
 */
#ifdef ESP32
void stripled_rmt_show(void)
{
	uint8_t * pixels = ws2812fx.getPixels();
	// numBytes is one more than the size of the ws2812fx's *pixels array.
	// the extra byte is used by the driver to insert the LED reset pulse at the end.
	uint16_t numBytes = ws2812fx.getNumBytes() + 1;
	rmt_write_sample(RMT_CHANNEL_0, pixels, numBytes, false);
}
#endif

/**
 * Init the strip led
 */
void stripled_init(void)
{
	brightness_table_init();

	/** Init the led driver */
	ws2812fx.init();
	ws2812fx.setSpeed(500);
	// When working with ESP32, use the available hardware to get better performances
#ifdef ESP32
	rmt_tx_int(RMT_CHANNEL_0, ws2812fx.getPin()); // assign ws2812fx1 to RMT channel 0
	ws2812fx.setCustomShow(stripled_rmt_show);    // set the custom show function to overwrite the NeoPixel's
#endif
	ws2812fx.setColor(STATUS_COLOR_R, STATUS_COLOR_G, STATUS_COLOR_B);
	ws2812fx.setLength(STATUS_NB_LED);
	ws2812fx.setBrightness(STATUS_BRIGHTNESS);

	// Brightness
	autoBrightTick = tick;

#if defined(BOARD_RING)
	stripled_set_state(false);
	set_animation(0); // 0: Static
	stripled_set_demo_mode(false);
#else
	// Go into demo mode at startup
	stripled_set_state(true);
	set_animation(STRIPLED_DEFAULT_ANIMATION_ID);
	stripled_set_demo_mode(true);
#endif

	ws2812fx.start();
}

/**
 * Main function of stripled
 * Should be executed every 1ms
 */
void stripled_main(void)
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