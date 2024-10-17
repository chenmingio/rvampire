#include "debug.h"
#include "platform.h"
#include "raylib.h"
#include "resource_dir.h" // utility header for SearchAndSetResourceDir
#include "string.h"
#include "vampire.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h> // Required for: malloc(), free()

global_variable RayLibSoundOutput ringOutput = {};

// frames = samples
void AudioInputCallback(void *writeBuffer, unsigned int frames) {
  u32 bytesToRead = frames * SAMPLE_SIZE;

  size_t region1Size;
  size_t region2Size = 0;

  if (ringOutput.readCursor + bytesToRead >
      ringOutput.data + ringOutput.bufferSize) {
    region1Size =
        ringOutput.data + ringOutput.bufferSize - ringOutput.readCursor;
    region2Size = bytesToRead - region1Size;
  } else {
    region1Size = bytesToRead;
  }

  memcpy(writeBuffer, ringOutput.readCursor, region1Size);
  ringOutput.readCursor += region1Size;

  if (region2Size > 0) {
    memcpy(writeBuffer + region1Size, ringOutput.data, region2Size);
    ringOutput.readCursor = ringOutput.data + region2Size;
  }

  return;
}

internal bool32 isValidSoundBuffer(RayLibSoundOutput *soundBuffer) {
  return (soundBuffer->writeCursor >= soundBuffer->data) &&
         (soundBuffer->writeCursor <=
          soundBuffer->data + soundBuffer->bufferSize) &&
         (soundBuffer->readCursor >= soundBuffer->data) &&
         (soundBuffer->readCursor <=
          soundBuffer->data + soundBuffer->bufferSize);
}

AudioStream setupAudio() {
  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);
  // sample size here is bit for each channel
  AudioStream stream = LoadAudioStream(SAMPLE_RATE, SAMPLE_SIZE * 8 / 2, 2);
  SetAudioStreamCallback(stream, AudioInputCallback);
  return stream;
}

int main() {

  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  SetTargetFPS(60);

  int screenWidth = SCREEN_WIDTH;
  int screenHeight = SCREEN_HEIGHT;
  // Create the window and OpenGL context
  InitWindow(screenWidth, screenHeight, "Vampire Game");

  // store 3 seconds of audio data
  // sample size = 2 channels * 2 bytes per sample
  ringOutput.duration = 3;
  ringOutput.sampleRate = SAMPLE_RATE;
  ringOutput.sampleSize = SAMPLE_SIZE;
  size_t outputBufferSize =
      ringOutput.sampleSize * ringOutput.sampleRate * ringOutput.duration;
  i16 *data = (i16 *)calloc(SAMPLE_RATE * 3, SAMPLE_SIZE);
  assert(CheckClean(data, outputBufferSize));
  ringOutput.data = data;
  ringOutput.readCursor = data;
  ringOutput.writeCursor = data;
  // reserve maximum size = ring buffer size
  ringOutput.bufferSize = outputBufferSize;

  AudioStream stream = setupAudio();
  bool32 playingSound = false;

  GameSoundOutputBuffer gameSound = {};
  i16 *samples = (i16 *)calloc(SAMPLE_RATE * 1, SAMPLE_SIZE);
  gameSound.samples = samples;
  gameSound.samplesPerSecond = ringOutput.sampleRate;

  SearchAndSetResourceDir("resources");
  SetGamepadMappings(LoadFileText("gamecontrollerdb.txt"));

  Image offscreenImage = GenImageColor(screenWidth, screenHeight, BLANK);
  GameOffscreenBuffer imageBuffer = {offscreenImage.data, screenWidth,
                                     screenHeight, screenWidth};

  // inputs slot = 4, 0-2 for gamepad, 3 for keyboard
  GameInput input = {};
  GameControllerInput *keyboardController = &input.Controller[3];
  keyboardController->connected = true;

  GameMemory gameMemory = {};
  gameMemory.permanentStorageSize = Megabytes(64);
  gameMemory.transientStorageSize = Gigabytes(4);
  size_t totalSize =
      gameMemory.permanentStorageSize + gameMemory.transientStorageSize;
  gameMemory.permanentStorage = calloc(1, totalSize);
  gameMemory.transientStorage =
      gameMemory.permanentStorage + gameMemory.permanentStorageSize;

  if (samples && gameMemory.permanentStorage) {
    // continue
  }

  // game loop
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    for (i32 controllerIndex = 0; controllerIndex < 3; controllerIndex++) {
      // and put them in 1-4 in game input controllers
      GameControllerInput *gameController = &input.Controller[controllerIndex];
      if (IsGamepadAvailable(controllerIndex)) {
        gameController->connected = true;

        gameController->gamepadX = GetGamepadAxisMovement(controllerIndex, 0);
        gameController->gamepadY = GetGamepadAxisMovement(controllerIndex, 1);

        gameController->up.EndedDown =
            IsGamepadButtonDown(controllerIndex, GAMEPAD_BUTTON_LEFT_FACE_UP);
        gameController->down.EndedDown =
            IsGamepadButtonDown(controllerIndex, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
        gameController->left.EndedDown =
            IsGamepadButtonDown(controllerIndex, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
        gameController->right.EndedDown = IsGamepadButtonDown(
            controllerIndex, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
        gameController->x.EndedDown = IsGamepadButtonDown(
            controllerIndex, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
        gameController->y.EndedDown =
            IsGamepadButtonDown(controllerIndex, GAMEPAD_BUTTON_RIGHT_FACE_UP);
        gameController->a.EndedDown = IsGamepadButtonDown(
            controllerIndex, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
        gameController->b.EndedDown = IsGamepadButtonDown(
            controllerIndex, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
        gameController->start.EndedDown =
            IsGamepadButtonDown(controllerIndex, GAMEPAD_BUTTON_MIDDLE_RIGHT);
      } else {
        gameController->connected = false;
      }
    }

    keyboardController->right.EndedDown = IsKeyDown(KEY_RIGHT);
    keyboardController->left.EndedDown = IsKeyDown(KEY_LEFT);
    keyboardController->up.EndedDown = IsKeyDown(KEY_UP);
    keyboardController->down.EndedDown = IsKeyDown(KEY_DOWN);

    r32 timeSpan = GetFrameTime();
    if (timeSpan > 1.0f) {
      timeSpan = 1.0f;
    }

    // output sound on a normal buffer
    gameSound.sampleCount = timeSpan * SAMPLE_RATE;
    u32 bytesToWrite = gameSound.sampleCount * SAMPLE_SIZE;

    GameUpdateAndRender(&gameMemory, &imageBuffer, &gameSound, &input,
                        timeSpan);

    size_t region1Size;
    size_t region2Size = 0;

    if (ringOutput.writeCursor + bytesToWrite >
        ringOutput.data + ringOutput.bufferSize) {
      region1Size =
          ringOutput.data + ringOutput.bufferSize - ringOutput.writeCursor;
      region2Size = bytesToWrite - region1Size;
    } else {
      region1Size = bytesToWrite;
    }

    assert(isValidSoundBuffer(&ringOutput));
    assert((region1Size % SAMPLE_SIZE) == 0);
    memcpy(ringOutput.writeCursor, (void *)gameSound.samples, region1Size);
    ringOutput.writeCursor = ringOutput.writeCursor + region1Size;
    assert(isValidSoundBuffer(&ringOutput));

    assert((region2Size % SAMPLE_SIZE) == 0);
    if (region2Size > 0) {
      memcpy(ringOutput.data, (void *)gameSound.samples + region1Size,
             region2Size);
      ringOutput.writeCursor = ringOutput.data + region2Size;
      assert(isValidSoundBuffer(&ringOutput));
    }

#if 0
    DrawSoundWave(ringOutput.data, ringOutput.bufferSize, 1);
    DrawCursor(ringOutput.data, ringOutput.writeCursor, ringOutput.bufferSize,
               1, RED);
    DrawCursor(ringOutput.data, ringOutput.readCursor, ringOutput.bufferSize, 1,
               GREEN);
    DrawCursor(ringOutput.data, ringOutput.data + bytesToWrite,
               ringOutput.bufferSize, 1, BLUE);
#endif

    Texture bufferTexture = LoadTextureFromImage(offscreenImage);
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
