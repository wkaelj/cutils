#pragma once

#include <stdint.h>

#define CGLM_DEFINE_PRINTS

#include <cglm/cglm.h>

#define array_length(array) ((size_t)sizeof(array) / (size_t)sizeof(array[0]))

#define NONULL(...) __attribute__((nonnull(__VA_ARGS__)))

typedef enum Result
{
    RS_SUCCESS = 0,
    RS_FAILURE,
} Result;

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

#ifdef DEBUG
#include "messenger.h"
#define db_assert_msg(statement, msg)                                   \
    do                                                                  \
    {                                                                   \
        if (!(statement))                                               \
        {                                                               \
            log_fatal("Assert (%s) failed, \"%s\"", #statement, (msg)); \
            abort();                                                    \
        }                                                               \
    }
#define db_assert(statement)                             \
    do                                                   \
    {                                                    \
        if (!(statement))                                \
        {                                                \
            log_fatal("Assert (%s) failed", #statement); \
            abort();                                     \
        }                                                \
    }

#else
#define db_assert_msg(statement, msg) ((void)0)
#define db_assert(statement) ((void)0)
#endif
