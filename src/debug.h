#pragma once

#include "platform.h"
#include "raylib.h"
#include <stddef.h>

void DrawSoundWave(void *data, u32 size, u32 width, u32 height);

bool32 CheckClean(void *data, size_t size);