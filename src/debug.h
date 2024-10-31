#pragma once

#include "vampire.h"
#include <raylib.h>
#include <stddef.h>

void DrawSoundWave(void *data, size_t size, u32 line);
void DrawCursor(void *start, void *cursor, size_t fullX, u32 lines,
                Color color);

bool32 CheckClean(void *data, size_t size);