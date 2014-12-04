#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

const int defaultBufferSize = 100;
static char defaultBuffer[defaultBufferSize];

char* sdprintf(const char* format, ...) {
	va_list args;
	va_start(args, format);
	char* ret = vsdprintf(format, args);
	va_end(args);
	return ret;
}

char* vsdprintf(const char* format, va_list args) {
	va_list copy;
	va_copy(copy, args);

	char* ret = NULL;
	int len = vsnprintf(defaultBuffer, defaultBufferSize, format, args);
	if(len >= defaultBufferSize) {
		char buffer[len];
		vsnprintf(buffer, (size_t) len, format, copy);
		ret = buffer;
	} else {
		ret = strdup(defaultBuffer);
	}

	va_end(copy);
	return ret;
}