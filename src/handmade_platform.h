#pragma once
#include <cstddef>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540

#define TARGET_FRAME_RATE 60

#define SAMPLE_SIZE sizeof(i16) * 2
#define MAX_SAMPLES_PER_UPDATE 4096 // about 1/10 of a second
#define SAMPLE_RATE 48000
#define MAX_SAMPLES_SECONDS 3

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

// size_t is for memory size and index, adaptive on
// different platforms, maximum 64. malloc return
// size_t type as system default.
typedef size_t memory_index;

typedef struct {
  u8 *writeCursor;
  u8 *readCursor;
  u8 *data;
  size_t sampleSize;
  size_t bufferSize;
  u32 sampleRate;
  u32 duration;
} RayLibSoundOutput;

typedef struct {
  size_t totalSize;

  FILE *writeInputStream;
  FILE *readInputStream;

  bool32 isRecording;
  bool32 isReplaying;

  void *gameMemoryBlock;

} RayLibState;