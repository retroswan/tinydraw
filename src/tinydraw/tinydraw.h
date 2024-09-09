#pragma once

#include <SDL3/SDL.h>

#include "types.h"

#define TINYDRAW_VERSION "v0.0.1"

int TinyDraw_Init(void);

// Takes a width & height
void TinyDraw_Create_RenderTarget(void);

SDL_GPUShader* TinyDraw_Load_Shader(
    const char* fileName,
    Uint32 samplerCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount,
    SDL_GPUShaderStage stage
);

// Takes a string
void TinyDraw_Load_Texture(void);

// Takes shader
// Frag or vertex & will do the right thing either way
void TinyDraw_Bind_Shader(void);

// Takes texture
// Run for each `TinyDraw_Render`
void TinyDraw_Bind_Texture(void);

// Takes float2 dest pos, float2 dest size, int2 source pos, int2 source size
void TinyDraw_Stage_Sprite(void);

// Takes rendertarget (or null to send to window)
// Uploads vertices to GPU & draws to render target
void TinyDraw_Render(void);

// Sends to window & ends the frame
void TinyDraw_EndFrame(void);

// Takes texture
void TinyDraw_Unload_Texture(void);

void TinyDraw_Quit(void);
