#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdarg.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef struct _rect {
    int x;
    int y;
    int width;
    int height;
} Rectangle;

typedef struct _color {
    u8 r;
    u8 g;
    u8 b;
} Color;

char* sdprintf(const char* format, ...);
char* vsdprintf(const char* format, va_list args);

#endif