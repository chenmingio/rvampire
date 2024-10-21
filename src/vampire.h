#include "platform.h"
#include <assert.h>

#if DEBUG
#define Assert(Expression) assert(Expression);
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

typedef struct {
} World;

typedef struct {
  i32 xOffset;
  i32 yOffset;
  u32 toneHz;
} GameState;

typedef struct {
  bool32 HalfTransitionCount;
  bool32 EndedDown;
} GameButtonState;

typedef struct {
  bool32 isConnected;
  bool32 isAnalog;

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
  GameControllerInput Controller[4];
} GameInput;

// pixel size = 32 bits
typedef struct {
  void *memory;
  u32 width;
  u32 height;
  u32 pitch;

} GameOffscreenBuffer;

typedef struct {
  void *permanentStorage;
  size_t permanentStorageSize;
  void *transientStorage;
  size_t transientStorageSize;
  bool32 isInitialized;
} GameMemory;

void GameUpdateAndRender(GameMemory *gameMemory,
                         GameOffscreenBuffer *imageBuffer,
                         GameSoundOutputBuffer *soundBuffer, GameInput *input,
                         r32 timeSpan);

typedef struct {
  void *contents;
  size_t contentsSize;
} debug_read_file_result;

#if HANDMADE_INTERNAL

debug_read_file_result DebugPlatformReadEntireFile(char *filename);
bool32 DebugPlatformWriteEntireFile(char *filename, size_t size, void *memory);
void DebugPlatformFreeFileMemory(void *memory);

#endif