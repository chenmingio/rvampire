#pragma once

#include "vampire.h"

#include <stdbool.h>
#include <stddef.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define SAMPLE_SIZE sizeof(i16) * 2
#define MAX_SAMPLES_PER_UPDATE 4096 // about 1/10 of a second
#define SAMPLE_RATE 48000
#define MAX_SAMPLES_SECONDS 3

typedef struct {
  u8 *writeCursor;
  u8 *readCursor;
  u8 *data;
  size_t sampleSize;
  size_t bufferSize;
  u32 sampleRate;
  u32 duration;
} RayLibSoundOutput;
