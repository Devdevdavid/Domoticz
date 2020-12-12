/**
  * @file   web_server.cpp
  * @brief  Handle embedded web server
  * @author David DEVANT
  * @date   12/08/2018
  */

#ifdef ESP32
#include "SPIFFS.h"
#include <ESP32WebServer.h>
#else
#include <ESP8266WebServer.h>
#endif
#include <FS.h>
#include <string>

#include "bootloader/file_sys.hpp"
#include "cmd/cmd.hpp"
#include "global.hpp"
#include "io/inputs.hpp"
#include "stripled/stripled.hpp"
#include "web_server.hpp"

#ifdef MODULE_WEBSERVER

G_WebServer server(80); //Server on port 80

void web_server_init(void)
{
	server.begin();

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

	server.onNotFound([]() {
		if (!handle_file_read(server.uri())) {
			server.send(404, "text/plain", "File Not Found");
		}
	});

	log_info("HTTP server started");
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
	if (SPIFFS.exists(path)) {
		File file = SPIFFS.open(path, "r");
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
	uint32_t color = cmd_get_color();
	String hexColor = String(color, HEX);

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

#endif /* MODULE_WEBSERVER */