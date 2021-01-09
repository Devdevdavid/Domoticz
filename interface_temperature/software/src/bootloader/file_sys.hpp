/**
  * @file   file_sys.hpp
  * @brief  Handle filesystem
  * @author David DEVANT
  * @date   11/06/2020
  */

#ifndef BOOTLOADER_FILE_SYS_H
#define BOOTLOADER_FILE_SYS_H

#include <Arduino.h>
#include <FS.h>

// File system
void file_sys_init(void);
bool file_sys_exist(String &path);
File file_sys_open(String &path, const char * mode);
void file_sys_end(void);


#endif /* BOOTLOADER_FILE_SYS_H */
