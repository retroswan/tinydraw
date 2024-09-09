#pragma once

#include <SDL3/SDL.h>

#include "types.h"

#define TINYDRAW_VERSION "v0.0.1"

int TinyDraw_Init(void);

SDL_GPUShader* TinyDraw_Load_Shader(
    const char* fileName,
    Uint32 samplerCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount,
    SDL_GPUShaderStage stage
);

void TinyDraw_Load_Texture(void);

void TinyDraw_Draw_Sprite(void);

void TinyDraw_Present(void);

void TinyDraw_Unload_Shader(void);

void TinyDraw_Unload_Texture(void);

void TinyDraw_Quit(void);
