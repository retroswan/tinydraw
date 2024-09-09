#include "tinydraw.h"
#define SDL_GPU_SHADERCROSS_IMPLEMENTATION
#include <SDL_gpu_shadercross.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC SDL_malloc
#define STBI_REALLOC SDL_realloc
#define STBI_FREE SDL_free
#define STBI_ONLY_HDR
#include "../vendor/stb_image.h"

// File System
static const char* basePath;
// TODO: should this be larger?
static char fullPath[256];

// Game metadata
const char* title = "TinyDraw Test (" TINYDRAW_VERSION ")";
const int2 sizeGame = {
    .x = 160,
    .y = 90,
};
const int2 sizeWindow = {
    .x = 1280,
    .y = 720,
};

// SDL_GPU
static SDL_GPUDevice* device;
static SDL_Window* window;

int TinyDraw_Init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 0;
    }
    
    // TODO: error handling
    device = SDL_CreateGPUDevice(SDL_ShaderCross_GetShaderFormats(), SDL_TRUE, NULL);
    if (device == NULL) {
        SDL_Log("Failed to create GPU device");
        return 0;
    }
    
    // TODO: error handling
    window = SDL_CreateWindow(title, sizeWindow.x, sizeWindow.y, 0);
    if (window == NULL) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return 0;
    }
    
    // TODO: error handling
    if (!SDL_ClaimGPUWindow(device, window)) {
        SDL_Log("Failed to claim window");
        return 0;
    }
    
    basePath = SDL_GetBasePath();
    
    return 1;
}

SDL_GPUShader* TinyDraw_Load_Shader(
    const char* fileName,
    Uint32 samplerCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount,
    SDL_GPUShaderStage stage
)
{
    SDL_snprintf(
        fullPath,
        sizeof(fullPath),
        "%sContent/Shaders/Compiled/%s.spv",
        basePath,
        fileName
    );
    
    size_t codeSize;
    void* code = SDL_LoadFile(fullPath, &codeSize);
    if (code == NULL) {
        SDL_Log("Failed to load shader from disk! %s", fullPath);
        return NULL;
    }
    
    SDL_GPUShader* shader;
    SDL_GPUShaderCreateInfo shaderInfo = {
        .code = code,
        .codeSize = codeSize,
        .entryPointName = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage,
        .samplerCount = samplerCount,
        .uniformBufferCount = uniformBufferCount,
        .storageBufferCount = storageBufferCount,
        .storageTextureCount = storageTextureCount
    };
    
    if (SDL_GetGPUDriver(device) == SDL_GPU_DRIVER_VULKAN) {
        shader = SDL_CreateGPUShader(device, &shaderInfo);
    } else {
        shader = SDL_ShaderCross_CompileFromSPIRV(device, &shaderInfo, SDL_FALSE);
    }
    
    if (shader == NULL) {
        SDL_Log("Failed to create shader!");
        SDL_free(code);
        return NULL;
    }
    
    SDL_free(code);
    
    return shader;
}

void TinyDraw_Quit(void)
{
    SDL_UnclaimGPUWindow(device, window);
    SDL_DestroyWindow(window);
    SDL_DestroyGPUDevice(device);
}
