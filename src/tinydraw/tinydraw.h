#pragma once

#include <SDL3/SDL.h>

#include "types.h"

#define TINYDRAW_VERSION "v0.0.1"

// Takes basic info about the game; e.g. resolution
int TinyDraw_Init(void);

// Takes a width & height
SDL_GPUTexture* TinyDraw_Create_RenderTarget(int width, int height);

SDL_GPUShader* TinyDraw_Load_Shader(
    const char* fileName,
    Uint32 samplerCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount,
    SDL_GPUShaderStage stage
);

SDL_GPUTexture* TinyDraw_Load_Texture(const char* fileName);

SDL_GPUGraphicsPipeline* TinyDraw_Create_Pipeline(
    SDL_GPUShader* vertexShader,
    SDL_GPUShader* fragmentShader
);

// Takes float2 dest pos, float2 dest size, int2 source pos, int2 source size
void TinyDraw_Stage_Sprite(float2 destPos, float2 destSize, int2 sourcePos, int2 sourceSize);

// Takes:
// - texture
// - rendertarget (or null to send to window)
// - float3 camera (x & y pos, z zoom)
// TODO: should eventually take a pipeline, which allows you to use a different fragment shader
// Uploads vertices to GPU & draws to render target
void TinyDraw_Render(
    SDL_GPUGraphicsPipeline* pipeline,
    SDL_GPUTexture* texture,
    float3 camera,
    SDL_GPUTexture* renderTarget,
    char clear
);

void TinyDraw_Clear(SDL_GPUTexture* renderTarget);

void TinyDraw_Destroy_Pipeline(SDL_GPUGraphicsPipeline* pipeline);

void TinyDraw_Unload_Shader(SDL_GPUShader* shader);

void TinyDraw_Unload_Texture(SDL_GPUTexture* texture);

void TinyDraw_Quit(void);
