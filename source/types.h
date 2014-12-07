#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef struct _color {
    u8 r;
    u8 g;
    u8 b;
} Color;

#endif