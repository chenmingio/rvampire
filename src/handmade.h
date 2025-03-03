#pragma once
#include "handmade_math.h"
#include "handmade_platform.h"
#include <MacTypes.h>
#include <assert.h>
#include <stddef.h>

typedef struct {
  u8 *base; // use u8 * to support ptr++ operation. void * needs casting
  memory_index used;
  memory_index size; // used for assertion
} GameArena;

// base use void * to support any type input
inline void initializeArena(GameArena *arena, memory_index size, void *base) {
  arena->base = (u8 *)base;
  arena->used = 0;
  arena->size = size;
}

// private to the file (function)
#define internal static
// non updated varible in the function scope
#define local_persist static
// private to the file (global variable)
#define global_variable static

#define PI 3.14159265359

#if DEBUG
#define Assert(Expression) assert(Expression);
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

// function definition in header will be copied into calling files multiple
// times function definition in cpp file will be used as reference function
// pointer inline will inline all function definitions copies
inline void *pushSize(GameArena *arena, memory_index size) {
  Assert(arena->used + size <= arena->size);
  void *result = arena->base + arena->used;
  arena->used += size;
  return result;
}

#define PushStruct(arena, type) (type *)pushSize(arena, sizeof(type))
#define PushStructArray(arena, type, count)                                    \
  (type *)pushSize(arena, sizeof(type) * count)

inline u32 SafeTruncateUInt64(u64 Value) {
  // TODO(casey): Defines for maximum values
  Assert(Value <= 0xFFFFFFFF);
  u32 Result = (u32)Value;
  return (Result);
}

typedef struct {
  bool32 HalfTransitionCount;
  bool32 EndedDown;
} GameButtonState;

typedef struct {
  bool32 isConnected;
  bool32 isAnalog; // stick is analog, dpad is not

  r32 stickAverageX;
  r32 stickAverageY;

  union {
    GameButtonState buttons[13];
    struct {
      GameButtonState moveUp;
      GameButtonState moveDown;
      GameButtonState moveLeft;
      GameButtonState moveRight;

      GameButtonState actionLeft;
      GameButtonState actionUp;
      GameButtonState actionDown;
      GameButtonState actionRight;

      GameButtonState leftShoulder;
      GameButtonState rightShoulder;

      GameButtonState start;
      GameButtonState back;

      // NOTE(casey): All buttons must be added above this line
      GameButtonState terminator;
    };
  };
} GameControllerInput;

typedef struct {
  i16 *samples;
  u32 samplesPerSecond;
  u32 sampleCount;
} GameSoundOutputBuffer;

typedef struct {
  GameControllerInput Controller[5];
} GameInput;

// pixel size = 32 bits
typedef struct {
  void *memory;
  u32 width;
  u32 height;
  u32 pitch;
} GameOffscreenBuffer;

#if HANDMADE_INTERNAL
typedef struct {
  void *contents;
  size_t contentsSize;
} debug_read_file_result;

// macro in FOO_BAR_BAZ format
#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name)                                  \
  debug_read_file_result name(const char *filename)
// function pointer type in foo_bar_baz format
// expand as: typedef debug_read_file_result (*name)(const char *filename);
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);
// function declaration in FooBarBaz format
DEBUG_PLATFORM_READ_ENTIRE_FILE(DebugPlatformReadEntireFile);
// function implementation in main.cpp

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name)                                 \
  bool32 name(const char *filename, size_t size, void *memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DebugPlatformWriteEntireFile);

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);
DEBUG_PLATFORM_FREE_FILE_MEMORY(DebugPlatformFreeFileMemory);
#endif

typedef struct {
  bool32 isInitialized;

  u8 *permanentStorage;
  size_t permanentStorageSize;

  u8 *transientStorage;
  size_t transientStorageSize;

  GameArena worldArena;

#if HANDMADE_INTERNAL
  debug_platform_read_entire_file *DebugPlatformReadEntireFile;
  debug_platform_write_entire_file *DebugPlatformWriteEntireFile;
  debug_platform_free_file_memory *DebugPlatformFreeFileMemory;
#endif

} GameMemory;

#define GAME_UPDATE_AND_RENDER(name)                                           \
  void name(GameMemory *gameMemory, GameInput *input,                          \
            GameOffscreenBuffer *imageBuffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
inline GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub) {}

#define GAME_GET_SOUND_SAMPLES(name)                                           \
  void name(GameMemory *gameMemory, GameSoundOutputBuffer *soundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);
inline GAME_GET_SOUND_SAMPLES(GameGetSoundSamplesStub) {}

typedef enum {
  EntityTypePlayer,
  EntityTypeZombie,
  EntityTypeWall,
} EntityType;

typedef struct {
  i32 x;
  i32 y;
  r32 relX;
  r32 relY;
} WorldPos;

// WorldPos is now defined in handmade_math.h
typedef struct {
  WorldPos pos;
  EntityType type;
} Entity;

struct EntityElement {
  Entity *entity;
  EntityElement *next;
};

typedef struct {
  i32 xOffset;
  i32 yOffset;
  u32 toneHz;
  Entity *player;
  Entity *entities[1000];
  u32 entityCount;
  WorldPos cameraPos;
} GameState;

typedef struct {

} Tile;

// better with mod operation?
inline WorldPos regulateWorldPosition(WorldPos pos) {
  while (pos.relX > 1) {
    pos.x += 1;
    pos.relX -= 1.0;
  }

  while (pos.relX < 0) {
    pos.x -= 1;
    pos.relX += 1.0;
  }

  while (pos.relY > 1) {
    pos.y += 1;
    pos.relY -= 1.0;
  }

  while (pos.relY < 0) {
    pos.y -= 1;
    pos.relY += 1.0;
  }

  return pos;
}

inline WorldPos operator+(WorldPos pos, v2 delta) {
  return regulateWorldPosition(
      (WorldPos){pos.x, pos.y, pos.relX + delta.x, pos.relY + delta.y});
}

inline WorldPos operator+(v2 delta, WorldPos pos) { return pos + delta; }
