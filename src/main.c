#include "debug.h"
#include "platform.h"
#include "raylib.h"
#include "resource_dir.h" // utility header for SearchAndSetResourceDir
#include "string.h"
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

  if (soundOutput.readCursor + bytesToRead >
      soundOutput.data + soundOutput.bufferSize) {
    region1Size =
        soundOutput.data + soundOutput.bufferSize - soundOutput.readCursor;
    region2Size = bytesToRead - region1Size;
  } else {
    region1Size = bytesToRead;
  }

  memcpy(writeBuffer, soundOutput.readCursor, region1Size);
  soundOutput.readCursor += region1Size;

  if (region2Size > 0) {
    memcpy(writeBuffer + region1Size, soundOutput.data, region2Size);
    soundOutput.readCursor = soundOutput.data + region2Size;
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
  size_t outputBufferSize = SAMPLE_SIZE * SAMPLE_RATE * 3;
  i16 *data = (i16 *)calloc(SAMPLE_RATE * 3, SAMPLE_SIZE);
  assert(CheckClean(data, outputBufferSize));
  soundOutput.data = data;
  soundOutput.readCursor = data;
  soundOutput.writeCursor = data;
  soundOutput.bufferSize = outputBufferSize;

  AudioStream stream = setupAudio();
  bool32 playingSound = false;

  GameSoundBuffer soundBuffer = {0};
  // 1 second of audio data
  i16 *soundData = (i16 *)calloc(SAMPLE_RATE * 1, SAMPLE_SIZE);
  soundBuffer.samples = soundData;
  soundBuffer.samplesPerSecond = SAMPLE_RATE;
  soundBuffer.bufferSize = SAMPLE_RATE * 1 * SAMPLE_SIZE;

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
    if (timeSpan > 1.0f) {
      timeSpan = 1.0f;
    }

    // output sound on a normal buffer
    soundBuffer.sampleCount = timeSpan * SAMPLE_RATE;
    u32 bytesToWrite = soundBuffer.sampleCount * SAMPLE_SIZE;
    assert(bytesToWrite <= soundBuffer.bufferSize);

    UpdateAndRenderWithSound(&buffer, &soundBuffer, inputs, timeSpan);
    DrawSoundWave(soundBuffer.samples, soundOutput.bufferSize, 2);

    size_t region1Size;
    size_t region2Size = 0;

    if (soundOutput.writeCursor + bytesToWrite >
        soundOutput.data + soundOutput.bufferSize) {
      region1Size =
          soundOutput.data + soundOutput.bufferSize - soundOutput.writeCursor;
      region2Size = bytesToWrite - region1Size;
    } else {
      region1Size = bytesToWrite;
    }

    assert(isValidSoundBuffer(&soundOutput));
    assert((region1Size % SAMPLE_SIZE) == 0);
    memcpy(soundOutput.writeCursor, (void *)soundBuffer.samples, region1Size);
    soundOutput.writeCursor = soundOutput.writeCursor + region1Size;
    assert(isValidSoundBuffer(&soundOutput));

    assert((region2Size % SAMPLE_SIZE) == 0);
    if (region2Size > 0) {
      memcpy(soundOutput.data, (void *)soundBuffer.samples + region1Size,
             region2Size);
      soundOutput.writeCursor = soundOutput.data + region2Size;
      assert(isValidSoundBuffer(&soundOutput));
    }

    DrawSoundWave(soundOutput.data, soundOutput.bufferSize, 1);
    DrawCursor(soundOutput.data, soundOutput.writeCursor,
               soundOutput.bufferSize, 1, RED);
    DrawCursor(soundOutput.data, soundOutput.readCursor, soundOutput.bufferSize,
               1, GREEN);
    DrawCursor(soundOutput.data, soundOutput.data + bytesToWrite,
               soundOutput.bufferSize, 1, BLUE);

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
