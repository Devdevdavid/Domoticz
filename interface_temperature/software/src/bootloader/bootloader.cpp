/**
  * @file   bootloader.cpp
  * @brief  Manage wifi and OTA
  * @author David DEVANT
  * @date   11/06/2020
  */

#ifdef ESP32
#include <WiFi.h>
#include <mDNS.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "global.hpp"
#include "bootloader.hpp"
#include "file_sys.hpp"

extern uint32_t tick;
uint32_t bootloaderTick = 0;

void bootloader_init(void)
{
  log_info("Starting %s", BOOTLOADER_VERSION);

#ifdef WIFI_IS_IN_AP_MODE
  IPAddress localIp(WIFI_AP_LOCAL_IP);
  IPAddress gateway(WIFI_AP_GATEWAY);
  IPAddress subnet(WIFI_AP_SUBNET);

  WiFi.mode(WIFI_AP);

  if (WiFi.softAP(WIFI_SSID, WIFI_PWD, WIFI_AP_CHANNEL, WIFI_AP_HIDDEN, WIFI_AP_MAX_CO)) {
    log_info("Access point is now available at %s", WIFI_SSID);
  } else {
    log_error("Failed to launch Access Point");
  }
  /** This line does magic, keep it here */
  WiFi.persistent(false);
  WiFi.softAPConfig(localIp, gateway, subnet);
  log_info("AP IP address: %s", WiFi.softAPIP().toString().c_str());
  log_info("AP MAC address: %s", WiFi.softAPmacAddress().c_str());
#else
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  log_info("Trying to connect to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PWD);

  uint8_t timeout = 100;
  while (WiFi.status() != WL_CONNECTED && (--timeout > 0)) {
    delay(100);
  }
  if (timeout <= 0) {
    log_error("Timeout occured during first Wifi connection");
  } else {
    log_info("ESP IP address: %s", WiFi.localIP().toString().c_str());
  }
#endif

  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setPassword(OTA_PWD);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    file_sys_end();
    log_info("Start updating %s", type.c_str());
  });

  ArduinoOTA.onEnd([]() {
    log_info("Update completed !");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static unsigned int percentOld;
    unsigned int percent = (progress / (total / 100));

    if (abs(percentOld - percent) < 10) {
      return;
    }
    percentOld = percent;

    log_info("Progress: %u%%\r", percent);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    log_error("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) log_error("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) log_error("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) log_error("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) log_error("Receive Failed");
    else if (error == OTA_END_ERROR) log_error("End Failed");
  });
  ArduinoOTA.begin();
}

void bootloader_main(void)
{
  ArduinoOTA.handle();

  // Periodically check wifi status
  if (tick > bootloaderTick) {
    bootloaderTick = tick + BOOTLOADER_CHECK_PERIOD;
#ifdef WIFI_IS_IN_AP_MODE
    if (WiFi.softAPgetStationNum() <= 0) {
      _unset(STATUS_WIFI, STATUS_WIFI_DEVICE_CO);
    } else {
      _set(STATUS_WIFI, STATUS_WIFI_DEVICE_CO);
    }
#else
    if (WiFi.status() != WL_CONNECTED) {
      _unset(STATUS_WIFI, STATUS_WIFI_IS_CO);
    } else {
      _set(STATUS_WIFI, STATUS_WIFI_IS_CO);
    }
#endif
  }
}
