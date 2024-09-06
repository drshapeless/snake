#ifndef DEFINES_H
#define DEFINES_H

#include <SDL3/SDL.h>
#include <stdbool.h>

typedef Uint8 u8;
typedef Uint16 u16;
typedef Uint32 u32;
typedef Uint64 u64;
typedef Sint8 i8;
typedef Sint16 i16;
typedef Sint32 i32;
typedef Sint64 i64;
typedef float f32;
typedef double f64;

#define max(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })

#define min(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })

#define CLAMP(x, upper, lower) (min(upper, max(x, lower)))

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#endif /* DEFINES_H */
