#include "bootloader.hpp"

extern uint32_t tick;
uint32_t bootloaderTick = 0;

void bootloader_init(void)
{
  log_info("Starting %s", BOOTLOADER_VERSION);

#ifdef IS_IN_AP_MODE
  IPAddress localIp(192,168,1,1);
  IPAddress gateway(192,168,1,254);
  IPAddress subnet(255,255,255,0);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(localIp, gateway, subnet);

  if (WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PWD)) {
    log_info("Access point is now available at %s", WIFI_AP_SSID);
  } else {
    log_error("Failed to launch Access Point");
  }
#else
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  log_info("Trying to connect to %s...", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
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
    log_info("Progress: %u%%\r", (progress / (total / 100)));
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

#ifdef IS_IN_AP_MODE
  log_info("OTA Ready ! IP address: %s", WiFi.softAPIP().toString().c_str());
#else
  log_info("OTA Ready ! IP address: %s", WiFi.localIP().toString().c_str());
#endif
}

void bootloader_main(void)
{
  ArduinoOTA.handle();

#ifndef IS_IN_AP_MODE
  // If not in AP mode, periodically check wifi status
  if (tick > bootloaderTick) {
    bootloaderTick = tick + BOOTLOADER_CHECK_PERIOD;

    if (WiFi.status() != WL_CONNECTED) {
      _unset(STATUS_WIFI, STATUS_WIFI_IS_CO);
    } else {
      _set(STATUS_WIFI, STATUS_WIFI_IS_CO);
    }
  }
#endif
}
