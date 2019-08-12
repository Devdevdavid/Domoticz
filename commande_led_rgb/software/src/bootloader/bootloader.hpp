#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "global.hpp"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "file_sys.hpp"

#define BOOTLOADER_VERSION    "Bootloader v1.0"
#define WIFI_SSID             "Bbox-9D73DEEB"
#define WIFI_PWD              "F3436D1346ED55127A61FFF1DE15EE"
#define OTA_PWD               "macintosh"
#define OTA_PORT              8266

// Bootloader
void bootloader_init(void);
void bootloader_main(void);

#endif /* BOOTLOADER_H */
