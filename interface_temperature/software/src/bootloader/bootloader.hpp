#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "global.hpp"

#define BOOTLOADER_VERSION          "Bootloader v1.1"
#define OTA_PORT                    8266
#define OTA_PWD                     "ThisIsSecureNow"

// ACESS POINT CONFIGURATION
#define WIFI_AP_CHANNEL             13                  // Wifi Channel [1-13] (Default = 1)
#define WIFI_AP_HIDDEN              false               // Tell if SSID is hidden
#define WIFI_AP_MAX_CO              1                   // Max connection supported by AP

/** Delay between two bootloader connection checks */
#define BOOTLOADER_CHECK_PERIOD     1000

// Bootloader
void bootloader_init(void);
void bootloader_main(void);

#endif /* BOOTLOADER_H */
