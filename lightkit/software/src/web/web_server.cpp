/**
  * @file   web_server.cpp
  * @brief  Handle embedded web server
  * @author David DEVANT
  * @date   12/08/2018
  */

#ifdef ESP32
#include <ESP32WebServer.h>
#include <Update.h>
#else
#include <ESP8266WebServer.h>
#endif
#include <string>
#include <ArduinoJson.h>

#include "wifi/wifi.hpp"
#include "cmd/cmd.hpp"
#include "file_sys/file_sys.hpp"
#include "global.hpp"
#include "io/inputs.hpp"
#include "script/script.hpp"
#include "stripled/stripled.hpp"
#include "web_server.hpp"

#ifdef MODULE_WEBSERVER

G_WebServer server(80); //Server on port 80

int web_server_init(void)
{
	server.begin();

	// --- Firmware Upload ---
	server.on(
	"/firmware_upload", HTTP_POST, []() { handle_firmware_upload(); }, []() { handle_firmware_data(); });
	server.on("/firmware_config", HTTP_GET, []() {
		handle_firmware_config();
	});

	// --- Get/Set interface ---
	server.on("/get_version", HTTP_GET, []() {
		handle_get_version();
	});
	server.on("/get_animation", HTTP_GET, []() {
		handle_get_animation();
	});
	server.on("/set_animation", HTTP_GET, []() {
		handle_set_animation();
	});
	server.on("/get_demo_mode", HTTP_GET, []() {
		handle_get_demo_mode();
	});
	server.on("/set_demo_mode", HTTP_GET, []() {
		handle_set_demo_mode();
	});
	server.on("/get_state", HTTP_GET, []() {
		handle_get_state();
	});
	server.on("/set_state", HTTP_GET, []() {
		handle_set_state();
	});
	server.on("/get_brightness", HTTP_GET, []() {
		handle_get_brightness();
	});
	server.on("/set_brightness", HTTP_GET, []() {
		handle_set_brightness();
	});
	server.on("/get_nb_led", HTTP_GET, []() {
		handle_get_nb_led();
	});
	server.on("/set_nb_led", HTTP_GET, []() {
		handle_set_nb_led();
	});
	server.on("/get_color", HTTP_GET, []() {
		handle_get_color();
	});
	server.on("/set_color", HTTP_GET, []() {
		handle_set_color();
	});
	server.on("/get_display_info", HTTP_GET, []() {
		handle_get_display_info();
	});
	server.on("/get_wifi_settings", HTTP_GET, []() {
		handle_get_wifi_settings();
	});
	server.on("/set_wifi_settings", HTTP_GET, []() {
		handle_set_wifi_settings();
	});

	// --- File management ---
	server.onNotFound([]() {
		if (!handle_file_read(server.uri())) {
			server.send(404, "text/plain", "File Not Found");
		}
	});

	log_info("HTTP server started");
	return 0;
}

void web_server_main(void)
{
	server.handleClient();
}

String getContentType(String filename)
{
	if (server.hasArg("download"))
		return "application/octet-stream";
	else if (filename.endsWith(".htm"))
		return "text/html";
	else if (filename.endsWith(".html"))
		return "text/html";
	else if (filename.endsWith(".css"))
		return "text/css";
	else if (filename.endsWith(".js"))
		return "application/javascript";
	else if (filename.endsWith(".png"))
		return "image/png";
	else if (filename.endsWith(".gif"))
		return "image/gif";
	else if (filename.endsWith(".jpg"))
		return "image/jpeg";
	else if (filename.endsWith(".ico"))
		return "image/x-icon";
	else if (filename.endsWith(".xml"))
		return "text/xml";
	else if (filename.endsWith(".pdf"))
		return "application/x-pdf";
	else if (filename.endsWith(".zip"))
		return "application/x-zip";
	else if (filename.endsWith(".gz"))
		return "application/x-gzip";
	return "text/plain";
}

bool handle_file_read(String path)
{
	log_info("Received : %s", path.c_str());
	if (path.endsWith("/")) {
		path += "index.html";
	}
	String contentType = getContentType(path);
	if (file_sys_exist(path)) {
		File file = file_sys_open(path, "r");
		server.streamFile(file, contentType);
		file.close();
		return true;
	}
	return false;
}

void handle_bad_parameter(void)
{
	server.send(200, "text/plain", "Bad parameter");
}

void handle_firmware_upload(void)
{
	String msg = String(Update.getError());
	server.sendHeader("Connection", "close");
	server.send(200, "text/plain", msg);
}

void handle_firmware_data(void)
{
	HTTPUpload & upload = server.upload();

	if (upload.status == UPLOAD_FILE_START) {
		log_info("upload.totalSize = %d", upload.totalSize);

		// Warning : contentLength is not the file length :
		// size of entire post request, file size + headers and other request data.
		// But it gives a good estimation of the amount of data needed.
		// Using max size will overwrite filesystem and therefore break the webserver
		// log_info("Starting update with: %s (%d kB)", upload.filename.c_str(), upload.contentLength / 1000);
		// if (!Update.begin(upload.contentLength)) {
		// 	Update.printError(Serial);
		// }
	} else if (upload.status == UPLOAD_FILE_WRITE) {
		// Write the buffer (2048 bytes max)
		// if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
		// 	Update.printError(Serial);
		// }
	} else if (upload.status == UPLOAD_FILE_END) {
		if (Update.end(true)) { // true: to set the size to the current progress
			log_info("Firmware update DONE");

			// Reset to apply in 1s to let time to send HTTP response
			script_delayed_reset(1000);
		} else {
			Update.printError(Serial);
		}
	}
}

void handle_firmware_config(void)
{
}

/**
 * Send 0 if LED are disabled and 1 if LED are enabled
 */
void handle_get_version(void)
{
	server.send(200, "text/plain", FIRMWARE_VERSION);
}

/**
 * Send 0 if LED are disabled and 1 if LED are enabled
 */
void handle_get_animation(void)
{
	server.send(200, "text/plain", String(cmd_get_animation()));
}

/**
 * Set the current animation
 */
void handle_set_animation(void)
{
	int32_t  ret;
	uint16_t animID;

	if (!server.hasArg("v")) {
		handle_bad_parameter();
		return;
	}

	animID = server.arg("v").toInt();

	ret = cmd_set_animation(animID);
	if (ret != 0) {
		log_error("cmd_set_animation() failed: ret = %d", ret);
	}

	server.send(200, "text/plain", "");
}

/**
 * Send 0 if LED are disabled and 1 if LED are enabled
 */
void handle_get_demo_mode(void)
{
	server.send(200, "text/plain", String(cmd_get_demo_mode() ? 1 : 0));
}

/**
 * If arg "state" is 1, turn LED on, if not, turn led off
 */
void handle_set_demo_mode(void)
{
	if (!server.hasArg("v")) {
		handle_bad_parameter();
		return;
	}

	cmd_set_demo_mode(server.arg("v").toInt() == 1);

	handle_get_demo_mode();
}

/**
 * Send 0 if LED are disabled and 1 if LED are enabled
 */
void handle_get_state(void)
{
	server.send(200, "text/plain", String(cmd_get_state() ? 1 : 0));
}

/**
 * If arg "state" is 1, turn LED on, if not, turn led off
 */
void handle_set_state(void)
{
	if (!server.hasArg("v")) {
		handle_bad_parameter();
		return;
	}

	cmd_set_state(server.arg("v").toInt() == 1);

	handle_get_state();
}

/**
 * Send the brightness value [0; 100]
 */
void handle_get_brightness(void)
{
	server.send(200, "text/plain", String(cmd_get_brightness()));
}

/**
 * Get the arg "v" and set the current brightness level [0; 100]
 */
void handle_set_brightness(void)
{
	if (!server.hasArg("v")) {
		handle_bad_parameter();
		return;
	}

	uint8_t level = server.arg("v").toInt();

	cmd_set_brightness(level);

	handle_get_brightness();
}

/**
 * Send the nb of LED value [1; STRIP_LED_MAX_NB_PIXELS]
 */
void handle_get_nb_led(void)
{
	server.send(200, "text/plain", String(cmd_get_nb_led()));
}

/**
 * Get the arg "v" and set the number of LED [0; STRIP_LED_MAX_NB_PIXELS]
 */
void handle_set_nb_led(void)
{
	if (!server.hasArg("v")) {
		handle_bad_parameter();
		return;
	}

	uint8_t nbLed = server.arg("v").toInt();

	cmd_set_nb_led(nbLed);
	handle_get_nb_led();
}

/**
 * Send the color currently configured
 */
void handle_get_color(void)
{
	uint32_t color    = cmd_get_color();
	String   hexColor = String(color, HEX);

	// Add leading zeros
	while (hexColor.length() < 6) {
		hexColor = "0" + hexColor;
	}

	server.send(200, "text/plain", hexColor);
}

/**
 * Get the arg "v" and set the color of some LED animation
 */
void handle_set_color(void)
{
	if (!server.hasArg("v")) {
		handle_bad_parameter();
		return;
	}

	uint32_t color = (uint32_t) strtol(server.arg("v").c_str(), 0, 16);

	cmd_set_color(color);
	handle_get_color();
}

/**
 * Send the information about display
 */
void handle_get_display_info(void)
{
	/**
   *  0x01: Hide all but keep color configuration (used by BOARD_RING)
   *  0x02 to 0x80: unused
   **/
	uint8_t displayInfo = 0x00;

#if defined(BOARD_RING)
	if (is_input_low(INPUTS_OPT_WEB_SERVER_DISPLAY)) {
		_set(displayInfo, 0x01);
	}
#endif

	server.send(200, "text/plain", String(displayInfo));
}

/**
 * @brief Send all wifi settings
 */
void handle_get_wifi_settings(void)
{
	wifi_handle_t * wifiHandle;
	DynamicJsonDocument json(1024);
	String jsonString;

	wifiHandle = wifi_get_handle();

	json["mode"] = wifiHandle->mode;
	json["ap"]["ssid"] = wifiHandle->ap.ssid;
	json["ap"]["password"] = wifiHandle->ap.password;
	json["ap"]["channel"] = wifiHandle->ap.channel;
	json["ap"]["maxConnection"] = wifiHandle->ap.maxConnection;
	json["ap"]["isHidden"] = wifiHandle->ap.isHidden == 1;
	json["ap"]["ip"] = wifiHandle->ap.ip.toString();
	json["ap"]["gateway"] = wifiHandle->ap.gateway.toString();
	json["ap"]["subnet"] = wifiHandle->ap.subnet.toString();
	json["client"]["ssid"] = wifiHandle->client.ssid;
	json["client"]["password"] = wifiHandle->client.password;
	json["client"]["delayBeforeAPFallbackMs"] = wifiHandle->client.delayBeforeAPFallbackMs;

	serializeJson(json, jsonString);
	server.send(200, "text/plain", jsonString);
}

/**
 * @brief Receive new wifi settings
 */
void handle_set_wifi_settings(void)
{
	String reason = "";
	int32_t ret = 0;
	wifi_handle_t * wifiHandle;
	wifi_handle_t wifiHandleTmp;
	DynamicJsonDocument json(1024);

	wifiHandle = wifi_get_handle();

	if (!server.hasArg("v")) {
		handle_bad_parameter();
		return;
	}

	deserializeJson(json, server.arg("v"));

	if (json["use_default"] == true) {
		ret = wifi_use_default_settings(reason);
	} else {
		// Copy data
		wifiHandleTmp.mode = json["mode"];
		strncpy(wifiHandleTmp.ap.ssid, json["ap"]["ssid"].as<char*>(), WIFI_SSID_MAX_LEN);
		strncpy(wifiHandleTmp.ap.password, json["ap"]["password"].as<char*>(), WIFI_PASSWORD_MAX_LEN);
		wifiHandleTmp.ap.channel = json["ap"]["channel"];
		wifiHandleTmp.ap.maxConnection = json["ap"]["maxConnection"];
		wifiHandleTmp.ap.isHidden = json["ap"]["isHidden"].as<bool>();
		wifiHandleTmp.ap.ip.fromString(json["ap"]["ip"].as<char*>());
		wifiHandleTmp.ap.gateway.fromString(json["ap"]["gateway"].as<char*>());
		wifiHandleTmp.ap.subnet.fromString(json["ap"]["subnet"].as<char*>());
		strncpy(wifiHandleTmp.client.ssid, json["client"]["ssid"].as<char*>(), WIFI_PASSWORD_MAX_LEN);
		strncpy(wifiHandleTmp.client.password, json["client"]["password"].as<char*>(), WIFI_SSID_MAX_LEN);
		wifiHandleTmp.client.delayBeforeAPFallbackMs = json["client"]["delayBeforeAPFallbackMs"];

		ret = wifi_use_new_settings(&wifiHandleTmp, reason);
	}

	if (ret == 0) {
		server.send(200, "text/plain", "ok");
	} else {
		server.send(200, "text/plain", reason);
	}
}

#endif /* MODULE_WEBSERVER */