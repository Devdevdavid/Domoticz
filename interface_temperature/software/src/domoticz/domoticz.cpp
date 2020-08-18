/**
  * @file   domoticz.cpp
  * @brief  Handle domoticz server communication
  * @author David DEVANT
  * @date   12/08/2019
  */

#include "global.hpp"
#include "domoticz.hpp"
#include "status_led/status_led.hpp"

#ifdef MODULE_DOMOTICZ

#include <ESP8266HTTPClient.h>

HTTPClient http;

uint8_t domoticz_send(String url)
{
#ifdef MODULE_STATUS_LED
      // Force led to blue until next status_led_main() call
      status_led_set_color('b');
#endif
    http.begin(DOMOTICZ_HOST, DOMOTICZ_PORT, url);
    http.setTimeout(DOMOTICZ_TIMEOUT_MS);
    int httpCode = http.GET();
    http.end();

    if (httpCode == 200) {
        _unset(STATUS_WIFI, STATUS_WIFI_DOMOTICZ_FAULT);
        return 0;
    } else {
        log_error("Domoticz bad reply (http code = %d)", httpCode);
        log_error("Request was: %s", url.c_str());
        _set(STATUS_WIFI, STATUS_WIFI_DOMOTICZ_FAULT);
        return 1;
    }
}

void domoticz_send_temperature(uint8_t sensorID, float degreesValue)
{
    // Format JSON à respecter pour l'API Domoticz - Domoticz JSON API
    // /json.htm?type=command&param=udevice&idx=IDX&nvalue=0&svalue=TEMP
    // https://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Temperature
    String url = "/json.htm?type=command&param=udevice&idx=";
    url += String(sensorID);
    url += "&nvalue=0&svalue=";
    url += String(degreesValue);

    if (domoticz_send(url) == 0) {
        log_info("Temperature sent for sensor %d", sensorID);
    }
}

#endif