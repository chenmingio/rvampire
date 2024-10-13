#include "vampire.h"
#include "platform.h"
#include <math.h>
#include <raylib.h>

internal void OutputSound(GameSoundOutputBuffer *soundBuffer) {
  local_persist u32 sinIdx = 0;
  u32 toneHz = 440.0;
  u32 volume = 20000;
  u32 wavePeriod = soundBuffer->samplesPerSecond / toneHz;

  i16 *sampleOut = soundBuffer->samples;
  for (u32 i = 0; i < soundBuffer->sampleCount; i++) {
    i16 value = (i16)(sinf(2.0 * PI * sinIdx++ / wavePeriod) * volume);
    *sampleOut++ = value;
    *sampleOut++ = value;
  }

#if 0
  i16 *sampleOut = soundBuffer->samples;
  for (u32 i = 0; i < soundBuffer->bufferSize / 4; i++) {
    i16 value = 10000;
    *sampleOut++ = value;
    *sampleOut++ = value;
  }
#endif
}

void UpdateAndRenderWithSound(GameOffscreenBuffer *imageBuffer,
                              GameSoundOutputBuffer *soundBuffer,
                              GameInput *input, r32 timeSpan) {
  local_persist i32 xOffset = 0;
  local_persist i32 yOffset = 0;

  for (i32 GameControllerIdx = 0; GameControllerIdx < 4; GameControllerIdx++) {
    GameControllerInput *gameController = &input->Controller[GameControllerIdx];
    if (gameController->connected) {
      if (gameController->up) {
        yOffset += 1;
      }
      if (gameController->down) {
        yOffset -= 1;
      }
      if (gameController->left) {
        xOffset -= 1;
      }
      if (gameController->right) {
        xOffset += 1;
      }
      if (gameController->x) {
        // do something
      }
      if (gameController->y) {
        // do something
      }
      if (gameController->a) {
        // do something
      }
      if (gameController->b) {
        // do something
      }
      if (gameController->start) {
        // do something
      }
      DrawText(TextFormat("Gamepad %d with offsets: %d, %d", GameControllerIdx,
                          xOffset, yOffset),
               10, 10 + GameControllerIdx * 20, 20, WHITE);
    }
  }

  u32 *row = imageBuffer->memory;
  for (u32 y = 0; y < imageBuffer->height; y++) {
    u32 *pixel = row;
    for (u32 x = 0; x < imageBuffer->width; x++) {
      u8 green = (u8)(x + xOffset);
      u8 blue = (u8)(y + yOffset);
      u32 color = 0xFF << 24 | (green << 16) | (blue << 8) | 0xFF;
      *pixel = color;
      pixel++;
    }
    row += imageBuffer->width;
  }

  //       // draw warrior on buffer
  //   local_persist Image warriorRaw = LoadImage("Warrior_Red.png");
  //   u32 warriorNumbers = 8;
  //   int warriorWidth = warriorRaw.width / 6;
  //   int warriorHeight = warriorRaw.height / warriorNumbers;
  //   u32 warriorIdx = 0;
  //   u32 warriorRefreshFrames = 16;

  //   Image warrior =
  //         ImageFromImage(warriorRaw, (Rectangle){0, warriorHeight *
  //         ((warriorIdx / warriorRefreshFrames) % warriorNumbers),
  //                                                warriorWidth,
  //                                                warriorHeight});
  //     ImageDraw(&buffer, warrior,
  //               (Rectangle){0, 0, warrior.width, warrior.height},
  //               (Rectangle){0, 0, warrior.width, warrior.height}, WHITE);

  //     warriorIdx++;
  //     if (warriorIdx > warriorRefreshFrames * warriorNumbers) {
  //       warriorIdx %= warriorRefreshFrames * warriorNumbers;
  //     }

  OutputSound(soundBuffer);
#if 0
  DrawSoundWave(soundBuffer->data,
                soundBuffer->samplesToUpdate * soundBuffer->sampleSize,
                imageBuffer->width, imageBuffer->height);
#endif
}
