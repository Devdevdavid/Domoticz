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

// Internals
// Pointer is constant but value isn't
stripled_params_t * const stripledParams = &flashSettings.stripledParams;

// Instanciate the library
WS2812FX ws2812fx = WS2812FX(STRIPLED_NB_PIXELS, STRIPLED_PIN, NEO_GRB + NEO_KHZ800);

// Next tick of animation change and brightness update
uint32_t refreshTick;
uint32_t demoTick;
uint32_t autoBrightTick;

/** Indicate the period in tick between two stripled refresh */
uint32_t refreshPeriod = STRIPLED_MAX_REFRESH_PERIOD;

// Externals
extern uint32_t tick;

/********************************
 *          Tools
 ********************************/

static inline void refresh_now(void)
{
	// Refresh now
	refreshTick = 0;
}

void brightness_set(uint8_t brightness)
{
	stripledParams->brightness = brightness;

	// Apply new value
	ws2812fx.setBrightness(brightness);

	// Save settings in flash
	flash_write();

	refresh_now();
}

void nb_led_set(uint8_t nbLed)
{
	stripledParams->nbLed = nbLed;

	// Clear all the pixel before changing length
	ws2812fx.strip_off();

	// Apply new value
	ws2812fx.setLength(nbLed);

	// Save settings in flash
	flash_write();

	refresh_now();
}

void color_set(const rgba_u * color)
{
	stripledParams->color = *color;

	log_info("Setting color to %02X - %02X - %02X", color->rgba.r, color->rgba.g, color->rgba.b);

	// Apply new value without alpha channel
	ws2812fx.setColor(color->u32 & 0x00FFFFFF);

	// Save settings in flash
	flash_write();

	// No need for speed here
	refreshPeriod = STRIPLED_LOWPOWER_REFRESH_PERIOD;
	refresh_now();
}

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
	stripledParams->animID = animID;

	// Save settings in flash only if demo mode is disabled
	if (stripledParams->isInDemoMode == false) {
		flash_write();
	}

	// High refresh needed for smooth animations
	refreshPeriod = STRIPLED_MAX_REFRESH_PERIOD;
	refresh_now();

	return 0;
}

/**
 * Enable/Disable demo mode
 * @param isDemoModeEn : bool
 */
void stripled_set_demo_mode(bool isDemoModeEn)
{
	// Force refresh to now
	demoTick = 0;

	stripledParams->isInDemoMode = isDemoModeEn;

	// Save in flash
	flash_write();

	refresh_now();
}

/**
 * Define the state of the LED strip (ON or OFF)
 */
void stripled_set_state(bool isOn)
{
	log_info("Stripled is now %s", isOn ? "on" : "off");

	// We don't flash_write() since this setting
	// is forced to true on startup
	stripledParams->isOn = isOn;
	if (isOn == false) {
		ws2812fx.strip_off();
	}
	refresh_now();
}

/********************************
 *          Init
 ********************************/

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
int stripled_init(void)
{
	/** Init the led driver */
	ws2812fx.init();
	ws2812fx.setSpeed(500);
	// When working with ESP32, use the available hardware to get better performances
#ifdef ESP32
	rmt_tx_int(RMT_CHANNEL_0, ws2812fx.getPin()); // assign ws2812fx1 to RMT channel 0
	ws2812fx.setCustomShow(stripled_rmt_show);    // set the custom show function to overwrite the NeoPixel's
#endif
	ws2812fx.setColor(stripledParams->color.u32);
	ws2812fx.setLength(stripledParams->nbLed);
	ws2812fx.setBrightness(stripledParams->brightness);

	// Brightness
	autoBrightTick = tick;

	// Configure demo mode
	stripled_set_state(true);
	set_animation(stripledParams->animID);
	stripled_set_demo_mode(stripledParams->isInDemoMode);

	ws2812fx.start();

	return 0;
}

/**
 * Main function of stripled
 * Should be executed every 1ms
 */
void stripled_main(void)
{
	if (stripledParams->isOn) {
		if (tick >= refreshTick) {
			// Program the next refresh
			refreshTick = tick + refreshPeriod;

			// Refresh strip display
			ws2812fx.service();
		}
	}

	// Update demo mode
	if (stripledParams->isInDemoMode) {
		if (tick >= demoTick) {
			demoTick = tick + STRIPLED_DEMO_MODE_PERIOD;

			if (stripledParams->animID == (ws2812fx.getModeCount() - 1)) {
				set_animation(0);
			} else {
				set_animation(stripledParams->animID + 1);
			}
		}
	}
}

#endif /* MODULE_STRIPLED */