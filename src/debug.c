#pragma once

#include "debug.h"
#include "platform.h"
#include <raylib.h>

void DrawSoundWave(void *data, u32 size, u32 width, u32 height) {
  i16 *d = (i16 *)data;
  for (u32 x = 0; x < width; x++) {
    u32 i = (u32)((r32)x / width * size);
    i32 y = (i32)((r32)(d[i]) / 20000 * height / 16);

    DrawPixel(x, height / 2 - y, RED);
    DrawPixel(x, height / 2, WHITE);
  }
}

bool32 CheckClean(void *data, size_t size) {
  u8 *d = (u8 *)data;
  for (u32 i = 0; i < size; i++) {
    if (d[i] != 0) {
      return false;
    }
  }
  return true;
}
