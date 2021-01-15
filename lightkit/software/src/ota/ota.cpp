/**
  * @file   ota.cpp
  * @brief  Manage OTA
  * @author David DEVANT
  * @date   11/06/2020
  */

#include <ArduinoOTA.h>

#include "file_sys/file_sys.hpp"
#include "global.hpp"
#include "ota.hpp"

extern uint32_t tick;
uint32_t        otaTick = 0;

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
	ArduinoOTA.begin();

	return 0;
}

void ota_main(void)
{
	ArduinoOTA.handle();
}
