/**
  * @file   web_server.hpp
  * @brief  Handle embedded web server
  * @author David DEVANT
  * @date   12/08/2018
  */

#ifndef WEB_WEB_SERVER_HPP
#define WEB_WEB_SERVER_HPP

#include <Arduino.h>

#define WEB_SERVER_HTTP_PORT 80 /**< Server port for web interface */

// Web Server
int  web_server_init(void);
void web_server_main(void);

#endif /* WEB_WEB_SERVER_HPP */
