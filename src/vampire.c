#include "vampire.h"
#include "platform.h"
#include <math.h>
#include <raylib.h>

internal void GameOutputSound(GameSoundOutputBuffer *soundBuffer, u32 toneHz) {
  local_persist u32 sinIdx = 0;
  u32 volume = 20000;
  u32 wavePeriod = soundBuffer->samplesPerSecond / toneHz;

  i16 *sampleOut = soundBuffer->samples;
  for (u32 i = 0; i < soundBuffer->sampleCount; i++) {
    i16 value = (i16)(sinf(2.0 * PI * sinIdx++ / wavePeriod) * volume);
    *sampleOut++ = value;
    *sampleOut++ = value;
  }
}

internal void RenderWeirdGradient(GameOffscreenBuffer *imageBuffer, i32 xOffset,
                                  i32 yOffset) {
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
    row += imageBuffer->pitch;
  }
}

void GameUpdateAndRender(GameMemory *gameMemory,
                         GameOffscreenBuffer *imageBuffer,
                         GameSoundOutputBuffer *soundBuffer, GameInput *input,
                         r32 timeSpan) {

  GameState *gameState = (GameState *)gameMemory->permanentStorage;
  if (!gameMemory->isInitialized) {
    gameState->xOffset = 0;
    gameState->yOffset = 0;
    gameState->toneHz = 440;
    gameMemory->isInitialized = true;
  }

  for (i32 GameControllerIdx = 0; GameControllerIdx < 4; GameControllerIdx++) {
    GameControllerInput *gameController = &input->Controller[GameControllerIdx];
    if (gameController->connected) {
      if (gameController->up.EndedDown) {
        gameState->yOffset += 1;
      }
      if (gameController->down.EndedDown) {
        gameState->yOffset -= 1;
      }
      if (gameController->left.EndedDown) {
        gameState->xOffset -= 1;
      }
      if (gameController->right.EndedDown) {
        gameState->xOffset += 1;
      }
    }
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

  GameOutputSound(soundBuffer, gameState->toneHz);
  RenderWeirdGradient(imageBuffer, gameState->xOffset, gameState->yOffset);
}
