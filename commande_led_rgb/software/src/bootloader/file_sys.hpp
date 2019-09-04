#ifndef BOOTLOADER_FILE_SYS_H
#define BOOTLOADER_FILE_SYS_H

#include "global.hpp"
#include "FS.h"

#ifdef ESP32
#include "SPIFFS.h"
#endif

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
