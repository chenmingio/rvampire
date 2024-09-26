#include "vampire.h"
#include "platform.h"
#include <assert.h>
#include <math.h>
#include <raylib.h>

bool32 isValidSoundBuffer(SoundBuffer *soundBuffer) {
  return (soundBuffer->writeCursorP >= soundBuffer->data) &&
         (soundBuffer->writeCursorP <= soundBuffer->data + soundBuffer->size) &&
         (soundBuffer->readCursorP >= soundBuffer->data) &&
         (soundBuffer->readCursorP <= soundBuffer->data + soundBuffer->size);
}

void drawSoundWave(void *data, u32 size, u32 width, u32 height) {
  for (u32 x = 0; x < width; x++) {
    u32 i = (u32)((r32)x / width * size);
    i16 *d = (i16 *)data;
    i32 y = (i32)((r32)(d[i]) / 20000 * height / 16);

    DrawPixel(x, height / 2 - y, RED);
    DrawPixel(x, height / 2, WHITE);
  }
}

void fillSoundBuffer(SoundBuffer *soundBuffer, r32 timeSpan) {
  DrawText(TextFormat("Frequency: %f, Volume: %d", soundBuffer->frequency,
                      soundBuffer->volume),
           10, 100, 20, WHITE);

  u32 samplesPerPeriod = SAMPLE_RATE / soundBuffer->frequency;

  u32 bytesToWrite = timeSpan * SAMPLE_RATE * SAMPLE_SIZE;
  // if (bytesToWrite > soundBuffer->size) {
  //   bytesToWrite = soundBuffer->size;
  // }
  assert(bytesToWrite <= soundBuffer->size);
  u32 region1Size;
  u32 region2Size = 0;

  if (soundBuffer->writeCursorP + bytesToWrite >
      soundBuffer->data + soundBuffer->size) {
    region1Size =
        soundBuffer->data + soundBuffer->size - soundBuffer->writeCursorP;
    region2Size = bytesToWrite - region1Size;
  } else {
    region1Size = bytesToWrite;
  }

  u32 region1SizeSamples = region1Size / SAMPLE_SIZE;
  for (u32 i = 0; i < region1SizeSamples; i++) {
    assert(isValidSoundBuffer(soundBuffer));
    *soundBuffer->writeCursorP++ =
        (i16)(soundBuffer->volume *
              sinf(2.0 * PI * soundBuffer->runningSampleIndex++ /
                   samplesPerPeriod));
  }

  if (region2Size > 0) {
    u32 region2SizeSamples = region2Size / SAMPLE_SIZE;
    soundBuffer->writeCursorP = soundBuffer->data;
    for (u32 i = 0; i < region2SizeSamples; i++) {
      assert(isValidSoundBuffer(soundBuffer));
      *soundBuffer->writeCursorP++ =
          (i16)(soundBuffer->volume *
                sinf(2 * PI * soundBuffer->runningSampleIndex++ /
                     samplesPerPeriod));
    }
  }
}

void UpdateAndRender(Image *imageBuffer, SoundBuffer *soundBuffer,
                     GameController input[4], r32 timeSpan) {
  local_persist i32 xOffset = 0;
  local_persist i32 yOffset = 0;

  for (i32 GameControllerIdx = 0; GameControllerIdx < 4; GameControllerIdx++) {
    GameController *gameController = &input[GameControllerIdx];
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

  // u32 *row = buffer->data;
  // for (u32 y = 0; y < buffer->height; y++) {
  //   u32 *pixel = row;
  //   for (u32 x = 0; x < buffer->width; x++) {
  //     u8 green = (u8)(x + xOffset);
  //     u8 blue = (u8)(y + yOffset);
  //     u32 color = 0xFF << 24 | (green << 16) | (blue << 8) | 0xFF;
  //     *pixel = color;
  //     pixel++;
  //   }
  //   row += buffer->width;
  // }

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

  fillSoundBuffer(soundBuffer, timeSpan);

  drawSoundWave(soundBuffer->data, soundBuffer->size, imageBuffer->width,
                imageBuffer->height);
}
