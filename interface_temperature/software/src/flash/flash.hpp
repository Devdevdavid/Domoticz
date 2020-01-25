#include "global.hpp"

#ifndef FLASH_FLASH_HPP
#define FLASH_FLASH_HPP

#ifdef MODULE_FLASH

/** EEPROM used size in bytes */
#define EEPROM_USED_SIZE                        5
#define EEPROM_NB_LED_ADDRESS                   0
#define EEPROM_COLOR_R_ADDRESS                  1
#define EEPROM_COLOR_G_ADDRESS                  2
#define EEPROM_COLOR_B_ADDRESS                  3
#define EEPROM_BRIGHTNESS_ADDRESS               4
#define EEPROM_CRC_ADDRESS                      5

void flash_init(void);
bool flash_is_ok(void);
void flash_write(void);

#endif /** MODULE_FLASH */
#endif /* FLASH_FLASH_HPP */