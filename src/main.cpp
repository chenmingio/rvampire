#include "debug.h"
#include "platform.h"
#include "raylib.h"
#include "resource_dir.h" // utility header for SearchAndSetResourceDir
#include "string.h"
#include "vampire.h"
#include <assert.h>
#include <cstddef>
#include <dlfcn.h>
#include <stdbool.h>
#include <stdlib.h> // Required for: malloc(), free()

#include <stdio.h>
#include <unistd.h>

global_variable RayLibSoundOutput ringOutput = {};

#if HANDMADE_INTERNAL
DEBUG_PLATFORM_READ_ENTIRE_FILE(DebugPlatformReadEntireFile) {
  int dataSize;
  void *data = LoadFileData(filename, &dataSize);
  debug_read_file_result result = {data, dataSize};
  return result;
};

bool32 DebugPlatformWriteEntireFile(const char *filename, size_t size,
                                    void *memory) {
  return SaveFileData(filename, memory, size);
};

void DebugPlatformFreeFileMemory(void *memory) {
  UnloadFileData((u8 *)memory);
};

#endif

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
    memcpy((u8 *)writeBuffer + region1Size, ringOutput.data, region2Size);
    ringOutput.readCursor = ringOutput.data + region2Size;
  }

  return;
}

internal bool32 isSoundBufferValid(RayLibSoundOutput *soundBuffer) {
  return (soundBuffer->writeCursor >= soundBuffer->data) &&
         (soundBuffer->writeCursor <=
          soundBuffer->data + soundBuffer->bufferSize) &&
         (soundBuffer->readCursor >= soundBuffer->data) &&
         (soundBuffer->readCursor <=
          soundBuffer->data + soundBuffer->bufferSize);
}

typedef struct {
  game_update_and_render *GameUpdateAndRender;
  game_get_sound_samples *GameGetSoundSamples;
  bool32 isValid;
  void *gameCodeDLL;
  long lastWriteTime;
} raylibGameCode;

internal void UnloadGameCode(raylibGameCode code) {
  if (code.gameCodeDLL) {
    dlclose(code.gameCodeDLL);
  }
}

internal raylibGameCode LoadGameCode() {

  raylibGameCode result = {};
  result.isValid = false;

  result.gameCodeDLL = dlopen("libgame.dylib", RTLD_NOW);
  if (!result.gameCodeDLL) {
    fprintf(stderr, "Error loading library: %s\n", dlerror());
  }
  if (result.gameCodeDLL) {
    result.GameUpdateAndRender = (game_update_and_render *)dlsym(
        result.gameCodeDLL, "GameUpdateAndRender");
    result.GameGetSoundSamples = (game_get_sound_samples *)dlsym(
        result.gameCodeDLL, "GameGetSoundSamples");
    if (result.GameUpdateAndRender && result.GameGetSoundSamples) {
      result.isValid = true;
      result.lastWriteTime = GetFileModTime("libgame.dylib");
    }
  }

  if (!result.isValid) {
    result.GameUpdateAndRender = GameUpdateAndRenderStub;
    result.GameGetSoundSamples = GameGetSoundSamplesStub;
    result.lastWriteTime = 0;
  }
  return result;
}

AudioStream setupAudio() {
  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);
  // sample size here is bit for each channel
  AudioStream stream = LoadAudioStream(SAMPLE_RATE, SAMPLE_SIZE * 8 / 2, 2);
  SetAudioStreamCallback(stream, AudioInputCallback);
  return stream;
}

void BeginRecordingInput(RayLibState *state) {
  state->writeInputStream = fopen("recording.vmi", "wb");
  fwrite(state->gameMemoryBlock, state->totalSize, 1, state->writeInputStream);
}

void BeginPlaybackInput(RayLibState *state) {
  state->readInputStream = fopen("recording.vmi", "rb");
  fread(state->gameMemoryBlock, state->totalSize, 1, state->readInputStream);
}

void EndRecordingInput(RayLibState *state) { fclose(state->writeInputStream); }

void EndPlaybackInput(RayLibState *state) { fclose(state->readInputStream); }

void RecordInput(RayLibState *state, GameInput *input) {
  size_t written = fwrite(input, sizeof(*input), 1, state->writeInputStream);
  if (written != 1) {
    fprintf(stderr, "Failed to write input to file\n");
  }
}

void LoopReadInput(RayLibState *state, GameInput *input) {
  size_t count = fread(input, sizeof(GameInput), 1, state->readInputStream);
  if (count == 0) {
    fseek(state->readInputStream, 0, SEEK_SET);
  }
}

int main() {

  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  SetTargetFPS(60);
  SetTraceLogLevel(LOG_WARNING);

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
  ringOutput.data = (u8 *)data;
  ringOutput.readCursor = (u8 *)data;
  ringOutput.writeCursor = (u8 *)data;
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

  GameInput input = {};
  GameControllerInput *keyboardController = &input.Controller[0];
  keyboardController->isConnected = true;

  RayLibState state = {};
  GameMemory memory = {};
  memory.permanentStorageSize = Megabytes(64);
  memory.transientStorageSize = Gigabytes(1);
#if HANDMADE_INTERNAL
  memory.DebugPlatformReadEntireFile = DebugPlatformReadEntireFile;
  memory.DebugPlatformWriteEntireFile = DebugPlatformWriteEntireFile;
  memory.DebugPlatformFreeFileMemory = DebugPlatformFreeFileMemory;
#endif
  state.totalSize = memory.permanentStorageSize + memory.transientStorageSize;
  memory.permanentStorage = (u8 *)calloc(1, state.totalSize);
  state.gameMemoryBlock = memory.permanentStorage;
  memory.transientStorage =
      memory.permanentStorage + memory.permanentStorageSize;

  if (samples && memory.permanentStorage) {

    raylibGameCode gameCode = LoadGameCode();

    // game loop
    while (!WindowShouldClose()) {
      BeginDrawing();
      ClearBackground(BLACK);

      // TODO put key press count into transient count
      keyboardController->moveUp.EndedDown = IsKeyDown(KEY_UP);
      keyboardController->moveDown.EndedDown = IsKeyDown(KEY_DOWN);
      keyboardController->moveRight.EndedDown = IsKeyDown(KEY_RIGHT);
      keyboardController->moveLeft.EndedDown = IsKeyDown(KEY_LEFT);

      if (IsKeyPressed(KEY_L)) {
        if (state.isRecording) {
          // stop recording and start replaying
          state.isRecording = false;
          state.isReplaying = true;
          EndRecordingInput(&state);
          BeginPlaybackInput(&state);
        } else if (state.isReplaying) {
          // stop replaying
          state.isReplaying = false;
          EndPlaybackInput(&state);
        } else {
          state.isRecording = true;
          BeginRecordingInput(&state);
        }
      }
      u32 maxControllerCount = 4;
      if (maxControllerCount > ArrayCount(input.Controller) - 1) {
        maxControllerCount = ArrayCount(input.Controller) - 1;
      }
      for (u32 controllerIndex = 0; controllerIndex < maxControllerCount;
           controllerIndex++) {
        // in gameController it's 1-5, 0 for keyboard
        GameControllerInput *gameController =
            &input.Controller[controllerIndex + 1];
        // in raylib it's 0-4
        if (IsGamepadAvailable(controllerIndex)) {
          gameController->isConnected = true;
          // SetGamepadVibration(controllerIndex, 0.5f, 0.5f); // not working
          // on macos

          gameController->stickAverageX =
              GetGamepadAxisMovement(controllerIndex, 0);
          gameController->stickAverageY =
              GetGamepadAxisMovement(controllerIndex, 1);

          if (gameController->stickAverageX != 0.0f ||
              gameController->stickAverageY != 0.0f) {
            gameController->isAnalog = true;
          }

          gameController->moveUp.EndedDown =
              IsGamepadButtonDown(controllerIndex, GAMEPAD_BUTTON_LEFT_FACE_UP);
          gameController->moveDown.EndedDown = IsGamepadButtonDown(
              controllerIndex, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
          gameController->moveLeft.EndedDown = IsGamepadButtonDown(
              controllerIndex, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
          gameController->moveRight.EndedDown = IsGamepadButtonDown(
              controllerIndex, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);

          if (gameController->moveUp.EndedDown ||
              gameController->moveDown.EndedDown ||
              gameController->moveLeft.EndedDown ||
              gameController->moveRight.EndedDown) {
            gameController->isAnalog = false;
          }

          gameController->actionUp.EndedDown = IsGamepadButtonDown(
              controllerIndex, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
          gameController->actionUp.EndedDown = IsGamepadButtonDown(
              controllerIndex, GAMEPAD_BUTTON_RIGHT_FACE_UP);
          gameController->actionDown.EndedDown = IsGamepadButtonDown(
              controllerIndex, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
          gameController->actionRight.EndedDown = IsGamepadButtonDown(
              controllerIndex, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
          gameController->start.EndedDown =
              IsGamepadButtonDown(controllerIndex, GAMEPAD_BUTTON_MIDDLE_RIGHT);
        } else {
          gameController->isConnected = false;
        }
      }

      if (state.isRecording) {
        RecordInput(&state, &input);
      } else if (state.isReplaying) {
        LoopReadInput(&state, &input);
      }

      if (state.isRecording) {
        DrawText("Recording", 10, 10, 20, RED);
      } else if (state.isReplaying) {
        DrawText("Replaying", 10, 10, 20, GREEN);
      }

      r32 timeSpan = GetFrameTime();
      if (timeSpan > 1.0f) {
        timeSpan = 1.0f;
      }

      // output sound on a normal buffer
      gameSound.sampleCount = timeSpan * SAMPLE_RATE;
      u32 bytesToWrite = gameSound.sampleCount * SAMPLE_SIZE;

      long currentDLLWriteTime = GetFileModTime("libgame.dylib");
      if (gameCode.lastWriteTime < currentDLLWriteTime) {
        UnloadGameCode(gameCode);
        gameCode = LoadGameCode();
        gameCode.lastWriteTime = currentDLLWriteTime;
      }
      gameCode.GameUpdateAndRender(&memory, &input, &imageBuffer);
      gameCode.GameGetSoundSamples(&memory, &gameSound);

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

      assert(isSoundBufferValid(&ringOutput));
      assert((region1Size % SAMPLE_SIZE) == 0);
      memcpy(ringOutput.writeCursor, (void *)gameSound.samples, region1Size);
      ringOutput.writeCursor = ringOutput.writeCursor + region1Size;
      assert(isSoundBufferValid(&ringOutput));

      assert((region2Size % SAMPLE_SIZE) == 0);
      if (region2Size > 0) {
        memcpy(ringOutput.data, (u8 *)gameSound.samples + region1Size,
               region2Size);
        ringOutput.writeCursor = ringOutput.data + region2Size;
        assert(isSoundBufferValid(&ringOutput));
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
  }
  CloseWindow();
  return 0;
}
