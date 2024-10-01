#include "debug.h"
#include "platform.h"
#include "raylib.h"
#include "resource_dir.h" // utility header for SearchAndSetResourceDir
#include "vampire.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h> // Required for: malloc(), free()

global_variable RayLibSoundOutput soundOutput = {0};

// frames = samples
void AudioInputCallback(void *writeBuffer, unsigned int frames) {
  u32 bytesToRead = frames * SAMPLE_SIZE;

  size_t region1Size;
  size_t region2Size = 0;

  if (soundOutput.readCursorP + bytesToRead >
      soundOutput.data + soundOutput.size) {
    region1Size = soundOutput.data + soundOutput.size - soundOutput.readCursorP;
    region2Size = bytesToRead - region1Size;
  } else {
    region1Size = bytesToRead;
  }

  i16 *d = (i16 *)writeBuffer;
  u32 region1Samples = region1Size / SAMPLE_SIZE;
  for (u32 i = 0; i < region1Samples; i++) {
    *d++ = *soundOutput.readCursorP++;
    if (soundOutput.readCursorP == soundOutput.writeCursorP) {
      soundOutput.readCursorP = soundOutput.writeCursorP;
    }
  }

  u32 region2Samples = region2Size / SAMPLE_SIZE;
  for (u32 i = 0; i < region2Samples; i++) {
    soundOutput.readCursorP = soundOutput.data;
    *d++ = *soundOutput.readCursorP++;
    if (soundOutput.readCursorP == soundOutput.writeCursorP) {
      soundOutput.readCursorP = soundOutput.writeCursorP;
    }
  }

  return;
}

internal bool32 isValidSoundBuffer(RayLibSoundOutput *soundBuffer) {
  return (soundBuffer->writeCursorP >= soundBuffer->data) &&
         (soundBuffer->writeCursorP <= soundBuffer->data + soundBuffer->size) &&
         (soundBuffer->readCursorP >= soundBuffer->data) &&
         (soundBuffer->readCursorP <= soundBuffer->data + soundBuffer->size);
}

AudioStream setupAudio() {
  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);
  AudioStream stream = LoadAudioStream(SAMPLE_RATE, SAMPLE_SIZE * 8, 1);
  SetAudioStreamCallback(stream, AudioInputCallback);
  return stream;
}

int main() {

  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  SetTargetFPS(60);

  int screenWidth = 1280;
  int screenHeight = 720;
  // Create the window and OpenGL context
  InitWindow(screenWidth, screenHeight, "Vampire Game");

  // store 3 seconds of audio data
  size_t dataSize = SAMPLE_SIZE * MAX_SAMPLES_SECONDS * SAMPLE_RATE;
  i16 *data = (i16 *)calloc(SAMPLE_RATE * MAX_SAMPLES_SECONDS, SAMPLE_SIZE);
  assert(CheckClean(data, dataSize));
  soundOutput.data = data;
  soundOutput.readCursorP = data;
  soundOutput.writeCursorP = data;
  soundOutput.size = dataSize;

  AudioStream stream = setupAudio();
  bool32 playingSound = false;

  SearchAndSetResourceDir("resources");
  SetGamepadMappings(LoadFileText("gamecontrollerdb.txt"));

  Image buffer = GenImageColor(screenWidth, screenHeight, BLANK);

  // inputs
  GameController inputs[5];
  // keyboard control is first input
  GameController *keyboardController = &inputs[0];
  *keyboardController = (GameController){0};
  keyboardController->connected = true;

  // game loop
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    // input assign
    for (i32 controllerIndex = 0; controllerIndex < 4; controllerIndex++) {
      GameController *gameController = &inputs[controllerIndex + 1];
      if (IsGamepadAvailable(controllerIndex)) {
        *gameController = (GameController){0};
        gameController->connected = true;

        gameController->gamepadX = GetGamepadAxisMovement(controllerIndex, 0);
        gameController->gamepadY = GetGamepadAxisMovement(controllerIndex, 1);

        gameController->up =
            IsGamepadButtonDown(controllerIndex, GAMEPAD_BUTTON_LEFT_FACE_UP);
        gameController->down =
            IsGamepadButtonDown(controllerIndex, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
        gameController->left =
            IsGamepadButtonDown(controllerIndex, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
        gameController->right = IsGamepadButtonDown(
            controllerIndex, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
        gameController->x = IsGamepadButtonDown(controllerIndex,
                                                GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
        gameController->y =
            IsGamepadButtonDown(controllerIndex, GAMEPAD_BUTTON_RIGHT_FACE_UP);
        gameController->a = IsGamepadButtonDown(controllerIndex,
                                                GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
        gameController->b = IsGamepadButtonDown(
            controllerIndex, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
        gameController->start =
            IsGamepadButtonDown(controllerIndex, GAMEPAD_BUTTON_MIDDLE_RIGHT);
      } else {
        gameController->connected = false;
      }
    }

    keyboardController->right = IsKeyDown(KEY_RIGHT);
    keyboardController->left = IsKeyDown(KEY_LEFT);
    keyboardController->up = IsKeyDown(KEY_UP);
    keyboardController->down = IsKeyDown(KEY_DOWN);

    r32 timeSpan = GetFrameTime();

    GameSoundBuffer soundBuffer = {0};
    i16 *soundData = (i16 *)calloc(SAMPLE_RATE * 3, SAMPLE_SIZE);
    soundBuffer.samples = soundData;
    soundBuffer.sampleCount = timeSpan * SAMPLE_RATE;
    soundBuffer.samplesPerSecond = SAMPLE_RATE;
    u32 bytesToWrite = soundBuffer.sampleCount * SAMPLE_SIZE;

    UpdateAndRenderWithSound(&buffer, &soundBuffer, inputs, timeSpan);

    i16 *s = soundBuffer.samples;
    if (bytesToWrite > soundOutput.size) {
      bytesToWrite = soundOutput.size;
    }
    assert(bytesToWrite <= soundOutput.size);
    u32 region1Size;
    u32 region2Size = 0;

    if (soundOutput.writeCursorP + bytesToWrite >
        soundOutput.data + soundOutput.size) {
      region1Size =
          soundOutput.data + soundOutput.size - soundOutput.writeCursorP;
      region2Size = bytesToWrite - region1Size;
    } else {
      region1Size = bytesToWrite;
    }

    u32 region1SizeSamples = region1Size / SAMPLE_SIZE;
    for (u32 i = 0; i < region1SizeSamples; i++) {
      assert(isValidSoundBuffer(&soundOutput));
      *soundOutput.writeCursorP++ = *s++;
    }

    if (region2Size > 0) {
      u32 region2SizeSamples = region2Size / SAMPLE_SIZE;
      soundOutput.writeCursorP = soundOutput.data;
      for (u32 i = 0; i < region2SizeSamples; i++) {
        assert(isValidSoundBuffer(&soundOutput));
        *soundOutput.writeCursorP++ = *s++;
      }
    }

    DrawSoundWave(soundOutput.data, soundOutput.size, screenWidth,
                  screenHeight);

    Texture bufferTexture = LoadTextureFromImage(buffer);
    DrawTexture(bufferTexture, 0, 0, WHITE);

    if (!playingSound) {
      PlayAudioStream(stream);
      playingSound = true;
    }

    EndDrawing();
    UnloadTexture(bufferTexture);
  }

  CloseWindow();
  return 0;
}
