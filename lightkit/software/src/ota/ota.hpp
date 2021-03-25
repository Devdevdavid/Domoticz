/**
  * @file   ota.hpp
  * @brief  Manage OTA
  * @author David DEVANT
  * @date   11/06/2020
  */

#ifndef OTA_H
#define OTA_H

#include "global.hpp"

#define OTA_PORT P_OTA_PORT
#define OTA_PWD  P_OTA_PWD

// OTA
int  ota_configure_mdns(void);
int  ota_init(void);
void ota_main(void);

#endif /* OTA_H */
