/**
  * @file   flash.cpp
  * @brief  Flash write and read API
  * @author David DEVANT
  * @date   12/08/2018
  */

#define FLASH_FLASH_CPP

#include "flash.hpp"
#include "global.hpp"
#include <EEPROM.h>

// Internals
flash_settings_t flashSettings;

// Externals
extern wifi_handle_t defaultWifiSettings;

static uint8_t flash_get_crc(uint8_t * pData, uint32_t size)
{
	uint32_t i;
	uint8_t  crc = 0xFF;

	for (i = 0; i < size; i++) {
		crc = crc ^ (*pData++);
	}

	return crc;
}

static bool flash_is_ok(void)
{
	uint8_t crcFlash;

	// Save crc
	crcFlash          = flashSettings.crc;
	flashSettings.crc = 0;

	// Compute
	flashSettings.crc = flash_get_crc((uint8_t *) &flashSettings, sizeof(flash_settings_t));

	if (flashSettings.crc != crcFlash) {
		return false;
	}

	return (flashSettings.version == FLASH_STRUCT_VERSION);
}

int flash_use_default(void)
{
	memset(&flashSettings, 0, sizeof(flash_settings_t));
	memcpy(&flashSettings.wifiHandle, &defaultWifiSettings, sizeof(defaultWifiSettings));
#ifdef MODULE_STRIPLED
	flashSettings.stripNbLed     = STRIPLED_NB_PIXELS;
	flashSettings.lastColor.r    = 255;
	flashSettings.lastBrightness = 128;
#endif
	return flash_write();
}

int flash_init(void)
{
	uint32_t  i;
	uint8_t * pFlashSettings = (uint8_t *) &flashSettings;

	// Read flash
	EEPROM.begin(sizeof(flash_settings_t));
	for (i = 0; i < sizeof(flash_settings_t); i++) {
		*pFlashSettings++ = EEPROM.read(i);
	}
	EEPROM.end();

	if (flash_is_ok()) {
		log_info("Flash is valid: crc = 0x%02X (v%d)", flashSettings.crc, flashSettings.version);
	} else {
		log_error("Flash have been corrupted ! Using default values.");
		flash_use_default();
	}

	// Read data from flash only at boot
	STATUS_NB_LED     = flashSettings.stripNbLed;
	STATUS_COLOR_R    = flashSettings.lastColor.r;
	STATUS_COLOR_G    = flashSettings.lastColor.g;
	STATUS_COLOR_B    = flashSettings.lastColor.b;
	STATUS_BRIGHTNESS = flashSettings.lastBrightness;

	return 0;
}

int flash_write(void)
{
	uint8_t * pFlashSettings = (uint8_t *) &flashSettings;
	uint32_t  i;

	flashSettings.version = FLASH_STRUCT_VERSION;

	// Clear crc and compute the new one
	flashSettings.crc = 0;
	flashSettings.crc = flash_get_crc((uint8_t *) &flashSettings, sizeof(flash_settings_t));

	// Write data
	EEPROM.begin(sizeof(flash_settings_t));
	for (i = 0; i < sizeof(flash_settings_t); i++) {
		EEPROM.write(i, *pFlashSettings++);
	}
	EEPROM.end();

	return 0;
}
