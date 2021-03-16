/**
  * @file   file_sys.hpp
  * @brief  Handle filesystem
  * @author David DEVANT
  * @date   11/06/2020
  */

#ifndef FILE_SYS_FILE_SYS_H
#define FILE_SYS_FILE_SYS_H

#include <Arduino.h>
#include <FS.h>

#include "global.hpp"

#ifdef FS_IS_SPIFFS
#include "SPIFFS.h"
#else
#include "LittleFS.h"
#endif

// File system
int      file_sys_init(void);
uint32_t file_sys_get_max_size(void);
bool     file_sys_exist(String & path);
File     file_sys_open(String & path, const char * mode);
void     file_sys_end(void);

#endif /* FILE_SYS_FILE_SYS_H */
