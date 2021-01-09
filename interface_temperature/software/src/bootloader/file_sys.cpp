/**
  * @file   file_sys.cpp
  * @brief  Handle filesystem
  * @author David DEVANT
  * @date   11/06/2020
  */

#include "file_sys.hpp"
#include "global.hpp"

#ifdef ESP32
#include "SPIFFS.h"
#else
#include "LittleFS.h"
#endif

void file_sys_init(void)
{
	if (!G_FileSystem.begin()) {
		log_error("G_FileSystem failed to launch");
	}
}

bool file_sys_exist(String &path)
{
	return G_FileSystem.exists(path);
}

File file_sys_open(String &path, const char * mode)
{
	return G_FileSystem.open(path, mode);
}

void file_sys_end()
{
	G_FileSystem.end();
}
