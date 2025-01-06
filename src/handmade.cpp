#include "handmade.h"
#include <math.h>

internal u32 roundR32ToU32(r32 value) {
  u32 result = (u32)(value + 0.5f);
  return result;
}

internal void GameOutputSound(GameSoundOutputBuffer *soundBuffer, u32 toneHz) {
  local_persist u32 sinIdx = 0;
  u32 volume = 0;
  u32 wavePeriod = soundBuffer->samplesPerSecond / toneHz;

  i16 *sampleOut = soundBuffer->samples;
  for (u32 i = 0; i < soundBuffer->sampleCount; i++) {
    i16 value = (i16)(sinf(2.0 * PI * sinIdx++ / wavePeriod) * volume);
    *sampleOut++ = value;
    *sampleOut++ = value;
  }
}

internal void drawRectangle(GameOffscreenBuffer *imageBuffer, r32 x, r32 y,
                            r32 width, r32 height, u32 color) {
  u32 minX = roundR32ToU32(x);
  u32 minY = roundR32ToU32(y);
  u32 maxX = roundR32ToU32(x + width);
  u32 maxY = roundR32ToU32(y + height);
  u32 *row = (u32 *)imageBuffer->memory;
  for (u32 j = minY; j < maxY; j++) {
    for (u32 i = minX; i < maxX; i++) {
      u32 *pixel = row + i + j * imageBuffer->pitch;
      *pixel = color;
      pixel++;
    }
  }
}

internal void RenderPlayer(GameOffscreenBuffer *imageBuffer, i32 xOffset,
                           i32 yOffset, i32 width, i32 height) {
  // color RGBA
  u32 color = 0xFFFFFFFF; // blue
  drawRectangle(imageBuffer, xOffset, yOffset, width, height, color);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples) {
  GameState *gameState = (GameState *)gameMemory->permanentStorage;
  Assert(sizeof(GameState) <= gameMemory->permanentStorageSize);
  GameOutputSound(soundBuffer, gameState->toneHz);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
  GameState *gameState = (GameState *)gameMemory->permanentStorage;
  Assert(sizeof(GameState) <= gameMemory->permanentStorageSize);

  // initialize game setting
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
    gameState->playerX = 10;
    gameState->playerY = 10;
    gameMemory->isInitialized = true;
  }
  i32 screenWidth = 960;
  i32 screenHeight = 540;
  i32 leftOffsetX = 40;
  i32 leftOffsetY = 40;

  r32 playerWidth = 1.2;
  r32 playerHeight = 1.8;
  i32 tileSize = 2;

  r32 meterToPixel = 20;

  u32 map[9][17] = {
      {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
      {1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
      {1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1},
      {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
      {0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
      {1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1},
      {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
      {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
      {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
  };

  for (i32 GameControllerIdx = 0; GameControllerIdx < 4; GameControllerIdx++) {
    GameControllerInput *gameController = &input->Controller[GameControllerIdx];
    if (gameController->isConnected) {
      if (gameController->moveUp.EndedDown) {
        gameState->playerY -= 10;
      }
      if (gameController->moveDown.EndedDown) {
        gameState->playerY += 10;
      }
      if (gameController->moveLeft.EndedDown) {
        gameState->playerX -= 10;
      }
      if (gameController->moveRight.EndedDown) {
        gameState->playerX += 10;
      }
    }
  }

  if (gameState->playerX < 0) {
    gameState->playerX = 0;
  }

  if (gameState->playerY < 0) {
    gameState->playerY = 0;
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

  // RenderWeirdGradient(imageBuffer, gameState->xOffset, gameState->yOffset);

  // AABBGGRR
  // drawRectangle(imageBuffer, 100, 100, 100, 100, 0xFFFFFFFF);
  // drawRectangle(imageBuffer, 100, 200, 100, 100, 0x00FFFFFF);
  // drawRectangle(imageBuffer, 100, 300, 100, 100, 0xFF0000FF);
  // drawRectangle(imageBuffer, 100, 400, 100, 100, 0xFFFF0000);
  // drawRectangle(imageBuffer, 100, 500, 100, 100, 0xFF00FF00);
  drawRectangle(imageBuffer, 0, 0, screenWidth, screenHeight, 0xFFFFFF00);

  // draw tiles in pixel
  i32 tileSizeInPixel = tileSize * meterToPixel;
  u32 count = 0;
  for (u32 y = 0; y < 9; y++) {
    for (u32 x = 0; x < 17; x++) {
      if (map[y][x] == 1) {
        drawRectangle(imageBuffer, leftOffsetX + x * tileSizeInPixel,
                      leftOffsetY + y * tileSizeInPixel, tileSizeInPixel,
                      tileSizeInPixel, 0xFF00FFFF);
      }
    }
  }

  RenderPlayer(imageBuffer, gameState->playerX, gameState->playerY,
               playerWidth * meterToPixel, playerHeight * meterToPixel);
}
