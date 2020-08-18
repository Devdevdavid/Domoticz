/**
  * @file   flash.cpp
  * @brief  Flash write and read API
  * @author David DEVANT
  * @date   12/08/2018
  */

#define FLASH_FLASH_CPP

#include <EEPROM.h>
#include "global.hpp"
#include "flash.hpp"

#ifdef MODULE_FLASH

void flash_init(void)
{
    EEPROM.begin(EEPROM_USED_SIZE);

    if (flash_is_ok()) {
        // Read data from flash only at boot
        STATUS_NB_LED = EEPROM.read(EEPROM_NB_LED_ADDRESS);
        STATUS_COLOR_R = EEPROM.read(EEPROM_COLOR_R_ADDRESS);
        STATUS_COLOR_G = EEPROM.read(EEPROM_COLOR_G_ADDRESS);
        STATUS_COLOR_B = EEPROM.read(EEPROM_COLOR_B_ADDRESS);
        STATUS_BRIGHTNESS = EEPROM.read(EEPROM_BRIGHTNESS_ADDRESS);
    } else {
        log_error("Flash have been corrupted ! Using default values.");
#ifdef MODULE_STRIPLED
        STATUS_NB_LED = STRIPLED_NB_PIXELS;
        STATUS_BRIGHTNESS = STRIPLED_DEFAULT_BRIGHTNESS_VALUE;
#else
        STATUS_NB_LED = 0;
        STATUS_BRIGHTNESS = 0;
#endif
        STATUS_COLOR_R = 255;   // White color
        STATUS_COLOR_G = 255;
        STATUS_COLOR_B = 255;
        flash_write();
    }
}

bool flash_is_ok(void)
{
    uint8_t crcFlash = EEPROM.read(EEPROM_CRC_ADDRESS);
    uint8_t crc = 0xFF;

    // -1: do not include CRC in the computation
    for (uint8_t i = 0; i < EEPROM_USED_SIZE - 1; i++) {
        crc = crc ^ EEPROM.read(i);
    }

    return (crc == crcFlash);
}

#define flash_hdlc_write(addr, value) { EEPROM.write(addr, value); crc ^= value; }

void flash_write(void)
{
    uint8_t crc = 0xFF;

    // Write data
    flash_hdlc_write(EEPROM_NB_LED_ADDRESS, STATUS_NB_LED);
    flash_hdlc_write(EEPROM_COLOR_R_ADDRESS, STATUS_COLOR_R);
    flash_hdlc_write(EEPROM_COLOR_G_ADDRESS, STATUS_COLOR_G);
    flash_hdlc_write(EEPROM_COLOR_B_ADDRESS, STATUS_COLOR_B);
    flash_hdlc_write(EEPROM_BRIGHTNESS_ADDRESS, STATUS_BRIGHTNESS);

    // Write the ending CRC
    EEPROM.write(EEPROM_CRC_ADDRESS, crc);
    EEPROM.commit();
}

#endif /* MODULE_FLASH */