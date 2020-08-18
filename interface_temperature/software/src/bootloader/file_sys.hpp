/**
  * @file   file_sys.hpp
  * @brief  Handle filesystem
  * @author David DEVANT
  * @date   11/06/2020
  */

#ifndef BOOTLOADER_FILE_SYS_H
#define BOOTLOADER_FILE_SYS_H

#include <Arduino.h>

// File system
void file_sys_init(void);
void file_sys_end(void);

#ifndef ESP32
int fs_get_last_id(void);
void fs_print_flash_info(void);
void fs_list_dir(void);
String formatBytes(size_t bytes);
String fs_read(const char * path);
void fs_write(const char * path, const char * message);
void fs_append(const char * path, const char * message);
void fs_remove_file(String path);
void fs_clear_picc(void);
#endif /* !ESP32 */

#endif /* BOOTLOADER_FILE_SYS_H */
