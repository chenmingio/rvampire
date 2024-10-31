#include "vampire.h"
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
  u32 *row = (u32 *)imageBuffer->memory;
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

// GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub) {}
// GAME_GET_SOUND_SAMPLES(GameGetSoundSamplesStub) {}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
  GameState *gameState = (GameState *)gameMemory->permanentStorage;
  Assert(sizeof(GameState) <= gameMemory->permanentStorageSize);
  if (!gameMemory->isInitialized) {
#if HANDMADE_INTERNAL
    const char *filename = "Warrior_Red.png";
    debug_read_file_result file =
        gameMemory->DebugPlatformReadEntireFile(filename);
    if (file.contents) {
      const char *outFilename = "test.out";
      DebugPlatformWriteEntireFile(outFilename, file.contentsSize,
                                   file.contents);
      DebugPlatformFreeFileMemory(file.contents);
    }
#endif

    gameState->xOffset = 0;
    gameState->yOffset = 0;
    gameState->toneHz = 512;
    gameMemory->isInitialized = true;
  }

  for (i32 GameControllerIdx = 0; GameControllerIdx < 4; GameControllerIdx++) {
    GameControllerInput *gameController = &input->Controller[GameControllerIdx];
    if (gameController->isConnected) {
      if (gameController->moveUp.EndedDown) {
        gameState->yOffset += 1;
      }
      if (gameController->moveDown.EndedDown) {
        gameState->yOffset -= 1;
      }
      if (gameController->moveLeft.EndedDown) {
        gameState->xOffset -= 1;
      }
      if (gameController->moveRight.EndedDown) {
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

  RenderWeirdGradient(imageBuffer, gameState->xOffset, gameState->yOffset);
}
