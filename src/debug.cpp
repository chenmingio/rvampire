#include "debug.h"
#include "platform.h"

void DrawSoundWave(void *data, size_t fullX, u32 lines) {
  i16 *d = (i16 *)data;
  i16 fullXData = fullX / sizeof(i16);
  for (u32 x = 0; x < SCREEN_WIDTH; x++) {
    // screen x mapping to data index
    u32 i = (u32)((r32)x / SCREEN_WIDTH * fullXData);
    // data value mapping to screen y
    i32 y = (i32)((r32)(d[i]) / 20000 * SCREEN_HEIGHT / 16);

    u32 baseY = SCREEN_HEIGHT / 16 * (lines * 3 + 1);
    DrawPixel(x, baseY - y, RED);
    DrawPixel(x, baseY, WHITE);
  }
}

void DrawCursor(void *start, void *cursor, size_t fullX, u32 lines,
                Color color) {
  size_t distance = (u8 *)cursor - (u8 *)start;
  // distance/fullX mapping to screen x
  u32 x = (u32)((r32)distance / fullX * SCREEN_WIDTH);

  u32 baseY = SCREEN_HEIGHT / 16 * (lines * 3 + 1);
  DrawRectangle(x, baseY, 4, 8, color);
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
