/**
  * @file   ota.cpp
  * @brief  Manage OTA
  * @author David DEVANT
  * @date   11/06/2020
  */

#include <ArduinoOTA.h>

#include "file_sys/file_sys.hpp"
#include "flash/flash.hpp"
#include "global.hpp"
#include "ota.hpp"

#ifdef ESP32
#include <ESPmDNS.h>
#else
#include <ESP8266mDNS.h>
#endif

extern uint32_t tick;
uint32_t        otaTick = 0;

/**
 * @brief Set the hostname for MDNS
 * @return 0: OK, -1: error
 */
int ota_configure_mdns(void)
{
	// Check if module name is set
	if (flashSettings.moduleName[0] == '\0') {
		return -1;
	}

	log_info("Using \"%s\" as module name", flashSettings.moduleName);

	// Clear MDNS before starting
#ifdef ESP32
	MDNS.end();
#else
	MDNS.close();
#endif

	// Start the service
	if (!MDNS.begin(flashSettings.moduleName)) {
		log_error("mDNS failed to start !");
		return -1;
	}
	MDNS.enableArduino(OTA_PORT);
	MDNS.addService("http", "tcp", 80);
	return 0;
}

int ota_init(void)
{
	ArduinoOTA.setPort(OTA_PORT);
	ArduinoOTA.setPassword(OTA_PWD);

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		} else {
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
		unsigned int        percent = (progress / (total / 100));

		if (abs(percentOld - percent) < 10) {
			return;
		}
		percentOld = percent;

		log_info("Progress: %u%%\r", percent);
	});

	ArduinoOTA.onError([](ota_error_t error) {
		log_error("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR)
			log_error("Auth Failed");
		else if (error == OTA_BEGIN_ERROR)
			log_error("Begin Failed");
		else if (error == OTA_CONNECT_ERROR)
			log_error("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR)
			log_error("Receive Failed");
		else if (error == OTA_END_ERROR)
			log_error("End Failed");
	});

#ifdef ESP32
	ArduinoOTA.setMdnsEnabled(false);
	ArduinoOTA.begin();
#else
	// false : don't do mdns, we do it ourself
	ArduinoOTA.begin(false);
#endif

	return 0;
}

void ota_main(void)
{
	ArduinoOTA.handle();

#if !defined(ESP32)
	MDNS.update();
#endif
}
