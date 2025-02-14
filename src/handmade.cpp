#include "handmade.h"
#include "handmade_platform.h"
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

internal void drawRectangle(GameOffscreenBuffer *imageBuffer, rectangle2 rec,
                            u32 color) {
  u32 minX = roundR32ToU32(rec.Min.x);
  u32 maxX = roundR32ToU32(rec.Max.x);
  u32 minY = roundR32ToU32(imageBuffer->height - rec.Max.y);
  u32 maxY = roundR32ToU32(imageBuffer->height - rec.Min.y);
  u32 *row = (u32 *)imageBuffer->memory;
  for (u32 j = minY; j < maxY; j++) {
    for (u32 i = minX; i < maxX; i++) {
      u32 *pixel = row + i + j * imageBuffer->pitch;
      *pixel = color;
      pixel++;
    }
  }
}

internal void RenderPlayer(GameOffscreenBuffer *imageBuffer, v2 playerPos,
                           i32 width, i32 height) {
  // color RGBA
  u32 color = 0xFFFFFFFF; // blue
  rectangle2 player = {{playerPos.x, playerPos.y},
                       {playerPos.x + width, playerPos.y + height}};
  drawRectangle(imageBuffer, player, color);
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
    gameState->playerPos = {10, 10};
    gameMemory->isInitialized = true;
  }

  // in pixel
  i32 screenWidth = 960;
  i32 screenHeight = 540;
  i32 leftOffsetX = 40;
  i32 leftOffsetY = 40;
  v2 screenOffset = V2i(leftOffsetX, leftOffsetY);

  // in meter
  r32 playerWidthMeter = 1.2;
  r32 playerHeightMeter = 1.8;
  r32 tileSizeMeter = 2.0f;

  r32 meterToPixel = 26;

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

  v2 nextPlayerPos;
  v2 unit = {0, 0}; // 初始化为零向量

  for (i32 GameControllerIdx = 0; GameControllerIdx < 4; GameControllerIdx++) {
    GameControllerInput *gameController = &input->Controller[GameControllerIdx];
    if (gameController->isConnected) {
      if (gameController->moveUp.EndedDown) {
        unit = {0, 1};
      }
      if (gameController->moveDown.EndedDown) {
        unit = {0, -1};
      }
      if (gameController->moveLeft.EndedDown) {
        unit = {-1, 0};
      }
      if (gameController->moveRight.EndedDown) {
        unit = {1, 0};
      }
    }
  }

  r32 playerSpeed = 10.0f;
  nextPlayerPos = gameState->playerPos + unit * playerSpeed;

  bool32 isOccupied = false;

  if (!isOccupied) {
    gameState->playerPos = nextPlayerPos;
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
  rectangle2 screenRect = {{0, 0}, {(r32)screenWidth, (r32)screenHeight}};
  drawRectangle(imageBuffer, screenRect, 0xFFFFFF00);

  // draw tiles in pixel
  i32 tileSizeInPixel = tileSizeMeter * meterToPixel;

  for (u32 y = 0; y < 9; y++) {
    for (u32 x = 0; x < 17; x++) {
      if (map[y][x] == 1) {
        rectangle2 tile = {{(r32)x * tileSizeInPixel, (r32)y * tileSizeInPixel},
                           {(r32)((x + 1) * tileSizeInPixel),
                            (r32)((y + 1) * tileSizeInPixel)}};
        drawRectangle(imageBuffer, tile + screenOffset, 0xFF00FFFF);
      }
    }
  }

  RenderPlayer(imageBuffer, gameState->playerPos,
               playerWidthMeter * meterToPixel,
               playerHeightMeter * meterToPixel);
}
