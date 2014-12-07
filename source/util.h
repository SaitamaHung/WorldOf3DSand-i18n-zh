#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdarg.h>

char* sdprintf(const char* format, ...);
char* vsdprintf(const char* format, va_list args);

#endif