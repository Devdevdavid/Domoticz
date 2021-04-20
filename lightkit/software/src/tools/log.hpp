/**
  * @file   log.cpp
  * @brief  Print logs on serial debug port
  * @author David DEVANT
  * @date   12/08/2017
  */

#ifndef LOG_H
#define LOG_H

#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#define LOG_VERSION  "0.1.0s"
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

enum
{
	LOG_TRACE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL
};

#define log_trace(...) log_log(LOG_TRACE, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILENAME__, __LINE__, __VA_ARGS__)

extern "C" {
void log_raw(const char * fmt, ...);
}
void log_log(int level, const char * file, int line, const char * fmt, ...);

#endif
