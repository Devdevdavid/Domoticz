/**
  * @file   web_server.hpp
  * @brief  Handle embedded web server
  * @author David DEVANT
  * @date   12/08/2018
  */

#ifndef WEB_WEB_SERVER_HPP
#define WEB_WEB_SERVER_HPP

#include <Arduino.h>

// Web Server
int    web_server_init(void);
void   web_server_main(void);
String getContentType(String filename);
bool   handle_file_read(String path);
void   handle_bad_parameter(void);

void handle_firmware_upload(void);
void handle_firmware_data(void);
void handle_firmware_config(void);

void handle_get_version(void);
void handle_get_animation(void);
void handle_set_animation(void);
void handle_get_demo_mode(void);
void handle_set_demo_mode(void);
void handle_get_state(void);
void handle_set_state(void);
void handle_get_brightness(void);
void handle_set_brightness(void);
void handle_get_nb_led(void);
void handle_set_nb_led(void);
void handle_get_color(void);
void handle_set_color(void);
void handle_get_display_info(void);
void handle_get_wifi_settings(void);
void handle_set_wifi_settings(void);

#endif /* WEB_WEB_SERVER_HPP */
