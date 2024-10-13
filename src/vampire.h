#include "platform.h"

typedef struct {
} World;

typedef struct {
  bool32 HalfTransitionCount;
  bool32 EndedDown;
} GameButtonState;

typedef struct {
  bool32 connected;
  bool32 isAnalog;

  r32 gamepadX;
  r32 gamepadY;

  bool32 up;
  bool32 down;
  bool32 left;
  bool32 right;

  bool32 x;
  bool32 y;
  bool32 a;
  bool32 b;

  bool32 start;
} GameControllerInput;

typedef struct {
  i16 *samples;
  u32 sampleCount;
  u32 samplesPerSecond;
  size_t bufferSize;
} GameSoundOutputBuffer;

typedef struct {
  GameControllerInput Controller[4];
} GameInput;

typedef struct {
  void *memory;
  u32 width;
  u32 height;

} GameOffscreenBuffer;

void UpdateAndRenderWithSound(GameOffscreenBuffer *imageBuffer,
                              GameSoundOutputBuffer *soundBuffer,
                              GameInput *input, r32 timeSpan);