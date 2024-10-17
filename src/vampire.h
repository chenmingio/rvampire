#include "platform.h"
#include <assert.h>

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#if DEBUG
#define Assert(Expression) assert(Expression);
#else
#define Assert(Expression)
#endif

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
  bool32 connected;
  bool32 isAnalog;

  r32 gamepadX;
  r32 gamepadY;

  union {
    GameButtonState buttons[9];
    struct {
      GameButtonState up;
      GameButtonState down;
      GameButtonState left;
      GameButtonState right;

      GameButtonState x;
      GameButtonState y;
      GameButtonState a;
      GameButtonState b;

      GameButtonState start;
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
