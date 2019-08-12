#ifndef WEB_WEB_SERVER_H
#define WEB_WEB_SERVER_H

#include "global.hpp"
#include <ESP8266WebServer.h>
#include <FS.h>
#include "bootloader/file_sys.hpp"
#include "stripLed/stripLed.hpp"
#include "cmd/cmd.hpp"

// Web Server
void web_server_init(void);
void web_server_main(void);
String getContentType(String filename);
bool handle_file_read(String path);
void handle_bad_parameter(void);
void handle_get_animation(void);
void handle_set_animation(void);
void handle_get_demo_mode(void);
void handle_set_demo_mode(void);
void handle_get_state(void);
void handle_set_state(void);
void handle_get_brightness(void);
void handle_set_brightness(void);

#endif /* WEB_WEB_SERVER_H */
