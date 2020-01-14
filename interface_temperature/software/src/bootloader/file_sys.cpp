#include "global.hpp"
#include "file_sys.hpp"
#include "FS.h"
#ifdef ESP32
#include "SPIFFS.h"
#endif

void file_sys_init(void)
{
  if(!SPIFFS.begin()) {
    log_error("SPIFFS failed to launch");
  }
}

void file_sys_end()
{
  SPIFFS.end();
}

#ifndef ESP32

int fs_get_last_id(void)
{
  int id = 0;
  Dir dir = SPIFFS.openDir("/PICC/");
  while (dir.next()) {
    id++;
  }
  return id;
}

void fs_print_flash_info(void)
{
  FSInfo fs_info;
  String str;

  SPIFFS.info(fs_info);
  str = "  > Memory : " + formatBytes(fs_info.usedBytes)
    + " / " + formatBytes(fs_info.totalBytes)
    + " (" + String((fs_info.usedBytes * 100) / fs_info.totalBytes)
    + "%)";
  log_info("Filesystem informations :\n%s", str.c_str());
}

void fs_list_dir(void)
{
  Dir dir = SPIFFS.openDir("/");

  log_info("Listing files in flash :");
  while (dir.next()) {
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    log_info("  %s, size: %s", fileName.c_str(), formatBytes(fileSize).c_str());
  }
}

String formatBytes(size_t bytes)
{
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String fs_read(const char * path, char buffer[])
{
  String content;

  File f = SPIFFS.open(path, "r");
  if (!f) {
    log_error("Failed to open file for read");
    content = "";
  } else {
    content = f.read(); // TODO Give him a buffer and a len
    f.close();
  }
  log_info("fs_read : %s", content.c_str());
  return content;
}

void fs_write(const char * path, const char * message)
{
  File file = SPIFFS.open(path, "w+");
  if (!file) {
    log_error("Failed to open file for writing");
    return;
  }
  if (!file.print(message)) {
    log_error("Write failed");
  }
}

void fs_append(const char * path, const char * message)
{
  File file = SPIFFS.open(path, "a+");
  if (!file) {
    log_error("Failed to open file for appending");
    return;
  }
  if (!file.print(message)) {
    log_error("Append failed");
  }
}

void fs_remove_file(String path)
{
  SPIFFS.remove(path);
}

void fs_clear_picc(void)
{
  Dir dir = SPIFFS.openDir("/PICC/");
  while (dir.next()) {
    String fileName = dir.fileName();
    SPIFFS.remove(fileName);
  }
}

#endif /* !ESP32 */
