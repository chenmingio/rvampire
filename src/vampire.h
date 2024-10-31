#pragma once
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

// private to the file (function)
#define internal static
// non updated varible in the function scope
#define local_persist static
// private to the file (global variable)
#define global_variable static

#define PI 3.14159265359

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float r32;
typedef double r64;
typedef int32_t bool32;

#if DEBUG
#define Assert(Expression) assert(Expression);
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

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
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);
// function declaration in FooBarBaz format
DEBUG_PLATFORM_READ_ENTIRE_FILE(DebugPlatformReadEntireFile);

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
  void name(GameMemory *Memory, GameSoundOutputBuffer *SoundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);
inline GAME_GET_SOUND_SAMPLES(GameGetSoundSamplesStub) {}

typedef struct {
  i32 xOffset;
  i32 yOffset;
  u32 toneHz;
} GameState;