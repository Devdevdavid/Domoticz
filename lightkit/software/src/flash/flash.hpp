/**
  * @file   flash.hpp
  * @brief  Flash write and read API
  * @author David DEVANT
  * @date   12/08/2018
  */

#ifndef FLASH_FLASH_HPP
#define FLASH_FLASH_HPP

#include "global.hpp"
#include "stripLed/stripLed.hpp"
#include "wifi/wifi.hpp"

/** EEPROM used size in bytes */
#define EEPROM_USED_SIZE 256

/** Increment this each time flash_settings_t is incompatible with previous version */
#define FLASH_STRUCT_VERSION 2

typedef struct {
	wifi_handle_t     wifiHandle;
	stripled_params_t stripledParams;

	// -- Used for flash purpose --
	uint8_t crc;
	uint8_t version;
} flash_settings_t;

static_assert(sizeof(flash_settings_t) < EEPROM_USED_SIZE, "flash_settings_t is bigger than EEPROM size");

int flash_use_default(void);
int flash_init(void);
int flash_write(void);

#ifndef FLASH_FLASH_CPP
extern flash_settings_t flashSettings;
#endif

#endif /* FLASH_FLASH_HPP */