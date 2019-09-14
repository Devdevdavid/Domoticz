#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "global.hpp"
#ifdef ESP32
#include <WiFi.h>
#include <mDNS.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "file_sys.hpp"

// Define wether the ESP is in Access point or in connected to a network
#undef IS_IN_AP_MODE

#define BOOTLOADER_VERSION          "Bootloader v1.1"
#define WIFI_SSID                   "Bbox-9D73DEEB"
#define WIFI_PWD                    "F3436D1346ED55127A61FFF1DE15EE"
#define OTA_PORT                    8266
#define OTA_PWD                     "ThisIsSecureNow"

// ACESS POINT CONFIGURATION
#define WIFI_AP_LOCAL_IP            192,168,1,1
#define WIFI_AP_GATEWAY             192,168,1,254
#define WIFI_AP_SUBNET              255,255,255,0
#define WIFI_AP_SSID                "RubanLed"
#define WIFI_AP_PWD                 "pascal2019"
#define WIFI_AP_CHANNEL             13                  // Wifi Channel [1-13] (Default = 1)
#define WIFI_AP_HIDDEN              false               // Tell if SSID is hidden
#define WIFI_AP_MAX_CO              1                   // Max connection supported by AP

/** Delay between two bootloader connection checks */
#define BOOTLOADER_CHECK_PERIOD     1000

// Bootloader
void bootloader_init(void);
void bootloader_main(void);

#endif /* BOOTLOADER_H */
