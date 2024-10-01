#include "platform.h"
#include "raylib.h"

typedef struct {
} World;

typedef struct {
  bool32 connected;

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
} GameController;

typedef struct {
  i16 *samples;
  u32 sampleCount;
  u32 samplesPerSecond;
} GameSoundBuffer;

void UpdateAndRenderWithSound(Image *imageBuffer, GameSoundBuffer *soundBuffer,
                              GameController input[4], r32 timeSpan);