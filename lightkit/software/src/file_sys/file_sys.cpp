/**
  * @file   file_sys.cpp
  * @brief  Handle filesystem
  * @author David DEVANT
  * @date   11/06/2020
  */

#include "file_sys.hpp"

int file_sys_init(void)
{
	// This part is exclusive to LittleFS
#ifdef FS_IS_LITTLEFS
	LittleFSConfig config(false); // false: no auto format

	if (!LittleFS.setConfig(config)) {
		log_error("LittleFS failed to set config");
		goto retError;
	}
#endif

	if (!G_FileSystem.begin()) {
		log_error("FileSystem failed to start");
		goto retError;
	}

	return 0;
retError:
	_set(STATUS_APPLI, STATUS_APPLI_FILESYSTEM);
	return -1;
}

uint32_t file_sys_get_max_size(void)
{
#if defined(FS_IS_LITTLEFS)

	FSInfo info;
	G_FileSystem.info(info);
	return info.totalBytes;

#elif defined(FS_IS_SPIFFS)

	return G_FileSystem.totalBytes();

#else

	return 0;

#endif
}

bool file_sys_exist(String & path)
{
	return G_FileSystem.exists(path);
}

File file_sys_open(String & path, const char * mode)
{
	return G_FileSystem.open(path, mode);
}

void file_sys_end()
{
	G_FileSystem.end();
}
