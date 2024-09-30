#pragma once

#include <stdbool.h>
#include <stdint.h>

// private to the file (function)
#define internal static
// non updated varible in the function scope
#define local_persist static
// private to the file (global variable)
#define global_variable static

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float r32;
typedef double r64;
typedef int32_t bool32;

#define SAMPLE_SIZE 2
#define MAX_SAMPLES_PER_UPDATE 4096 // about 1/10 of a second
#define SAMPLE_RATE 44100
#define MAX_SAMPLES_SECONDS 3
#define PI 3.14159265359

typedef struct {
  i16 *writeCursorP;
  i16 *readCursorP;
  i16 *data;
  u32 size;
  u32 runningSampleIndex;
  u32 volume;
  r32 frequency;
} SoundBuffer;