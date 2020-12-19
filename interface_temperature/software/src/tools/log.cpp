/**
  * @file   log.cpp
  * @brief  Print logs on serial debug port
  * @author David DEVANT
  * @date   12/08/2017
  */

#include "log.hpp"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cmd/term.hpp"

static char         message[128];
static char         argsString[512];
static const char * level_names[] = {
	"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

void log_raw(const char * fmt, ...)
{
	va_list args;

	/* Log to console */

	va_start(args, fmt);
	vsprintf(argsString, fmt, args);
	va_end(args);

	sprintf(message, "%s", argsString);

	term_print(message);
}

void log_log(int level, const char * file, int line, const char * fmt, ...)
{
	va_list args;

	/* Log to console */

	va_start(args, fmt);
	vsprintf(argsString, fmt, args);
	va_end(args);

	sprintf(message, "%-5s [%s]:%d: %s\n", level_names[level], file, line, argsString);

	term_print(message);
}
