#pragma once

/*
MIT License

Copyright (c) 2024 retroswan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <SDL3/SDL.h>

#define TINYDRAW_VERSION "v0.0.2"

// Types

typedef struct int2
{
    int x;
    int y;
} int2;

typedef struct float2
{
    float x;
    float y;
} float2;

typedef struct float3
{
    float x;
    float y;
    float z;
} float3;

typedef struct matrix4x4
{
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
} matrix4x4;

typedef struct Color
{
    float r, g, b, a;
} Color;

typedef struct Vertex
{
    float x, y, z;
    float u, v;
    float r, g, b, a;
} Vertex;

// Function Declarations

/**
 * Initializes SDL3 & SDL_GPU.
 *
 * @return  int truthy for success, falsy for failure. Logs to console on
 *              failure as well.
 */
int TinyDraw_Init(void);

/**
 * Resize window & go in or out of fullscreen.
 *
 * @param   int   width
 * @param   int   height
 * @param   char  fullscreen
 */
void TinyDraw_Resize(int width, int height, char fullscreen);

/**
 * Create a Render Target with the given size.
 *
 * Returns an `SDL_GPUTexture*`. Destroy with `TinyDraw_Unload_Texture`.
 *
 * @param   int             width
 * @param   int             height
 *
 * @return  SDL_GPUTexture*
 */
SDL_GPUTexture* TinyDraw_Create_RenderTarget(int width, int height);

/**
 * Create a Pipeline. They are associated with a vertex & fragment shader.
 *
 * @param   SDL_GPUShader*  vertexShader
 * @param   SDL_GPUShader*  fragmentShader
 *
 * @return  SDL_GPUGraphicsPipeline*
 */
SDL_GPUGraphicsPipeline* TinyDraw_Create_Pipeline(
    SDL_GPUShader* vertexShader,
    SDL_GPUShader* fragmentShader
);

/**
 * Load a shader file with the given parameters.
 *
 * @param   char*               filename            under `./Content/shaders/`
 * @param   Uint32              samplerCount
 * @param   Uint32              uniformBufferCount
 * @param   Uint32              storageBufferCount
 * @param   Uint32              storageTextureCount
 * @param   SDL_GPUShaderStage  stage               whether vertex or fragment
 *
 * @return  SDL_GPUShader[return description]
 */
SDL_GPUShader* TinyDraw_Load_Shader(
    const char* fileName,
    Uint32 samplerCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount,
    SDL_GPUShaderStage stage
);

/**
 * Load a texture file.
 *
 * Optionally output the width & height of the texture.
 *
 * @param   char*   filename    under `./Content/sprites/`
 * @param   int*    width       pointer to write to, or `NULL`
 * @param   int*    height      pointer to write to, or `NULL`
 *
 * @return  SDL_GPUTexture*
 */
SDL_GPUTexture* TinyDraw_Load_Texture(
    const char* fileName,
    int* width,
    int* height
);

/**
 * Prepare a sprite to be drawn.
 *
 * @param   float2  destPos
 * @param   float2  destSize
 * @param   float2  sourcePos   takes values between 0 and 1
 * @param   float2  sourceSize  takes values between 0 and 1
 * @param   Color   color
 */
void TinyDraw_Stage_Sprite(
    float2 destPos,
    float2 destSize,
    float2 sourcePos,
    float2 sourceSize,
    Color color
);

/**
 * Render staged sprites to the screen, or to a render target.
 *
 * @param   SDL_GPUGraphicsPipeline*    pipeline
 * @param   SDL_GPUTexture*             texture
 * @param   float3                      camera
 * @param   SDL_GPUTexture*             renderTarget
 * @param   char                        clear
 */
void TinyDraw_Render(
    SDL_GPUGraphicsPipeline* pipeline,
    SDL_GPUTexture* texture,
    float3 camera,
    SDL_GPUTexture* renderTarget,
    char clear
);

/**
 * Clear the screen or a render target.
 *
 * @param   SDL_GPUTexture* renderTarget
 */
void TinyDraw_Clear(SDL_GPUTexture* renderTarget);

/**
 * Destroy a pipeline after you're done with it.
 *
 * @param   SDL_GPUGraphicsPipeline*    pipeline
 */
void TinyDraw_Destroy_Pipeline(SDL_GPUGraphicsPipeline* pipeline);

/**
 * Unload a shader after you're done with it.
 *
 * @param   SDL_GPUShader* shader
 */
void TinyDraw_Unload_Shader(SDL_GPUShader* shader);

/**
 * Unload a texture after you're done with it.
 *
 * @param   SDL_GPUTexture* texture
 */
void TinyDraw_Unload_Texture(SDL_GPUTexture* texture);

/**
 * Quit TinyDraw.
 */
void TinyDraw_Quit(void);

#ifdef TINYDRAW_IMPLEMENTATION

#define SDL_GPU_SHADERCROSS_IMPLEMENTATION
#include <SDL_gpu_shadercross.h>

extern unsigned char* stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);
extern void stbi_image_free(void *retval_from_stbi_load);

#define SPRITE_COUNT 1024

// File System
static const char* basePath = NULL;
// TODO: should this be larger?
static char fullPath[256] = "";

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

// SDL_GPU spritebatch
static SDL_GPUTransferBuffer* vertexBufferTransferBuffer = NULL;
static SDL_GPUBuffer* indexBuffer = NULL;
static SDL_GPUBuffer* vertexBuffer = NULL;
static int spriteBatchCount = 0;

// SDL_GPU transfer
static SDL_GPUCommandBuffer* stageCmdbuf = NULL;
static SDL_GPUCopyPass* stageCopyPass = NULL;
static SDL_GPUTransferBuffer* stageTransferBuffer = NULL;
static Vertex* stageTransferData = NULL;

// SDL_GPU misc
static SDL_GPUDevice* device = NULL;
static SDL_GPUSampler* sampler = NULL;
static SDL_Window* window = NULL;

// SDL_GPU assets
static SDL_GPUShader* vertexShader = NULL;
static SDL_GPUShader* fragmentShader = NULL;

// Static methods

static matrix4x4 Matrix4x4_CreateOrthographicOffCenter(
    float left,
    float right,
    float bottom,
    float top,
    float zNearPlane,
    float zFarPlane
) {
    return (matrix4x4){
        2.0f / (right - left), 0, 0, 0,
        0, 2.0f / (top - bottom), 0, 0,
        0, 0, 1.0f / (zNearPlane - zFarPlane), 0,
        (left + right) / (left - right), (top + bottom) / (bottom - top), zNearPlane / (zNearPlane - zFarPlane), 1
    };
}

// Public Methods

int TinyDraw_Init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 0;
    }
    
    device = SDL_CreateGPUDevice(SDL_ShaderCross_GetShaderFormats(), SDL_TRUE, NULL);
    if (device == NULL) {
        SDL_Log("Failed to create GPU device");
        return 0;
    }
    
    window = SDL_CreateWindow(title, sizeWindow.x, sizeWindow.y, 0);
    if (window == NULL) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return 0;
    }
    
    if (!SDL_ClaimGPUWindow(device, window)) {
        SDL_Log("Failed to claim window");
        return 0;
    }
    
    basePath = SDL_GetBasePath();
    
    sampler = SDL_CreateGPUSampler(device, &(SDL_GPUSamplerCreateInfo){
        .minFilter = SDL_GPU_FILTER_NEAREST,
        .magFilter = SDL_GPU_FILTER_NEAREST,
        .mipmapMode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .addressModeU = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .addressModeV = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .addressModeW = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    });
    
    vertexBuffer = SDL_CreateGPUBuffer(
        device,
        &(SDL_GPUBufferCreateInfo) {
            .usageFlags = SDL_GPU_BUFFERUSAGE_VERTEX_BIT,
            .sizeInBytes = sizeof(Vertex) * 4 * SPRITE_COUNT
        }
    );
    SDL_SetGPUBufferName(
        device,
        vertexBuffer,
        "TinyDraw Vertex Buffer"
    );
    
    indexBuffer = SDL_CreateGPUBuffer(
        device,
        &(SDL_GPUBufferCreateInfo) {
            .usageFlags = SDL_GPU_BUFFERUSAGE_INDEX_BIT,
            .sizeInBytes = sizeof(Uint16) * 6 * SPRITE_COUNT
        }
    );
    SDL_SetGPUBufferName(
        device,
        vertexBuffer,
        "TinyDraw Index Buffer"
    );
    SDL_GPUTransferBuffer* bufferTransferBuffer = SDL_CreateGPUTransferBuffer(
        device,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .sizeInBytes = (sizeof(Vertex) * 4 * SPRITE_COUNT) + (sizeof(Uint16) * 6 * SPRITE_COUNT)
        }
    );
    Vertex* transferData = SDL_MapGPUTransferBuffer(
        device,
        bufferTransferBuffer,
        SDL_FALSE
    );
    Uint16* indexData = (Uint16*) &transferData[4 * SPRITE_COUNT];
    for (int i = 0; i < SPRITE_COUNT; i++) {
        const int u = i * 6;
        const int v = i * 4;
        indexData[u + 0] = v + 0;
        indexData[u + 1] = v + 1;
        indexData[u + 2] = v + 2;
        indexData[u + 3] = v + 0;
        indexData[u + 4] = v + 2;
        indexData[u + 5] = v + 3;
    }
    SDL_UnmapGPUTransferBuffer(device, bufferTransferBuffer);
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
    SDL_UploadToGPUBuffer(
        copyPass,
        &(SDL_GPUTransferBufferLocation) {
            .transferBuffer = bufferTransferBuffer,
            .offset = sizeof(Vertex) * 4 * SPRITE_COUNT
        },
        &(SDL_GPUBufferRegion) {
            .buffer = indexBuffer,
            .offset = 0,
            .size = sizeof(Uint16) * 6 * SPRITE_COUNT
        },
        SDL_FALSE
    );
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPU(uploadCmdBuf);
    SDL_ReleaseGPUTransferBuffer(device, bufferTransferBuffer);
    
    return 1;
}

void TinyDraw_Resize(int width, int height, char fullscreen)
{
    SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    
    if (!fullscreen) {
        SDL_SetWindowSize(window, width, height);
    }
}

SDL_GPUTexture* TinyDraw_Create_RenderTarget(int width, int height)
{
    SDL_GPUTexture* renderTarget = SDL_CreateGPUTexture(device,
        &(SDL_GPUTextureCreateInfo){
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GetGPUSwapchainTextureFormat(device, window),
            .width = width,
            .height = height,
            .layerCountOrDepth = 1,
            .levelCount = 1,
            .usageFlags = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET_BIT | SDL_GPU_TEXTUREUSAGE_SAMPLER_BIT,
        }
    );
    
    return renderTarget;
}

SDL_GPUGraphicsPipeline* TinyDraw_Create_Pipeline(
    SDL_GPUShader* vertexShader,
    SDL_GPUShader* fragmentShader
)
{
    SDL_GPUGraphicsPipelineCreateInfo info = {
        .attachmentInfo = {
            .colorAttachmentCount = 1,
            .colorAttachmentDescriptions = (SDL_GPUColorAttachmentDescription[]){{
                .format = SDL_GetGPUSwapchainTextureFormat(device, window),
                .blendState = {
                    .blendEnable = SDL_TRUE,
                    .alphaBlendOp = SDL_GPU_BLENDOP_ADD,
                    .colorBlendOp = SDL_GPU_BLENDOP_ADD,
                    .colorWriteMask = 0xF,
                    .srcColorBlendFactor = SDL_GPU_BLENDFACTOR_ONE,
                    .srcAlphaBlendFactor = SDL_GPU_BLENDFACTOR_ONE,
                    .dstColorBlendFactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                    .dstAlphaBlendFactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                }
            }},
        },
        .vertexInputState = (SDL_GPUVertexInputState){
            .vertexBindingCount = 1,
            .vertexBindings = (SDL_GPUVertexBinding[]){{
                .binding = 0,
                .inputRate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                .instanceStepRate = 0,
                .stride = sizeof(Vertex)
            }},
            .vertexAttributeCount = 3,
            .vertexAttributes = (SDL_GPUVertexAttribute[]){
                {
                    .binding = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                    .location = 0,
                    .offset = 0,
                },
                {
                    .binding = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                    .location = 1,
                    .offset = sizeof(float) * 3,
                },
                {
                    .binding = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                    .location = 2,
                    .offset = sizeof(float) * 5,
                },
            },
        },
        .multisampleState.sampleMask = 0xFFFF,
        .primitiveType = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertexShader = vertexShader,
        .fragmentShader = fragmentShader,
        // TODO: depth stencil state here
    };
    
    return SDL_CreateGPUGraphicsPipeline(
        device,
        &info
    );
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
        "%sContent/shaders/%s.spv",
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

SDL_GPUTexture* TinyDraw_Load_Texture(
    const char* fileName,
    int* width,
    int* height
)
{
    SDL_snprintf(fullPath, sizeof(fullPath), "%sContent/sprites/%s", basePath, fileName);
    int w, h, comp;
    unsigned char* pixels = stbi_load(fullPath, &w, &h, &comp, 0);
    if (pixels == NULL) {
        SDL_Log("Failed to load image `%s`\n", fullPath);
        return NULL;
    }
    
    if (width != NULL) {
        *width = w;
    }
    
    if (height != NULL) {
        *height = h;
    }
    
    SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &(SDL_GPUTextureCreateInfo){
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .width = w,
        .height = h,
        .layerCountOrDepth = 1,
        .levelCount = 1,
        .usageFlags = SDL_GPU_TEXTUREUSAGE_SAMPLER_BIT
    });
    
    SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(
        device,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .sizeInBytes = w * h * 4
        }
    );
    Uint8* textureTransferPtr = SDL_MapGPUTransferBuffer(
        device,
        textureTransferBuffer,
        SDL_FALSE
    );
    SDL_memcpy(textureTransferPtr, pixels, w * h * 4);
    SDL_UnmapGPUTransferBuffer(device, textureTransferBuffer);
    
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
    SDL_UploadToGPUTexture(
        copyPass,
        &(SDL_GPUTextureTransferInfo) {
            .transferBuffer = textureTransferBuffer,
            .offset = 0,
        },
        &(SDL_GPUTextureRegion){
            .texture = texture,
            .w = w,
            .h = h,
            .d = 1
        },
        SDL_FALSE
    );
    stbi_image_free(pixels);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPU(uploadCmdBuf);
    SDL_ReleaseGPUTransferBuffer(device, textureTransferBuffer);
    
    return texture;
}

void TinyDraw_Stage_Sprite(
    float2 destPos,
    float2 destSize,
    float2 sourcePos,
    float2 sourceSize,
    Color color
)
{
    if (stageCmdbuf == NULL) {
        stageCmdbuf = SDL_AcquireGPUCommandBuffer(device);
        if (stageCmdbuf == NULL) {
            SDL_Log("GPUAcquireCommandBuffer failed");
            return;
        }
        stageCopyPass = SDL_BeginGPUCopyPass(stageCmdbuf);
        stageTransferBuffer = SDL_CreateGPUTransferBuffer(
            device,
            &(SDL_GPUTransferBufferCreateInfo) {
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .sizeInBytes = (sizeof(Vertex) * 4 * SPRITE_COUNT) + (sizeof(Uint16) * 6 * SPRITE_COUNT)
            }
        );
        stageTransferData = SDL_MapGPUTransferBuffer(
            device,
            stageTransferBuffer,
            SDL_FALSE
        );
    }
    
    const int offset = spriteBatchCount * 4;
    stageTransferData[offset + 0] = (Vertex) {
        .x = destPos.x,
        .y = destPos.y,
        .z = 0,
        .u = sourcePos.x,
        .v = sourcePos.y,
        .r = color.r, .g = color.g, .b = color.b, .a = color.a,
    };
    stageTransferData[offset + 1] = (Vertex) {
        .x = destPos.x + destSize.x,
        .y = destPos.y,
        .z = 0,
        .u = sourcePos.x + sourceSize.x,
        .v = sourcePos.y,
        .r = color.r, .g = color.g, .b = color.b, .a = color.a,
    };
    stageTransferData[offset + 2] = (Vertex) {
        .x = destPos.x + destSize.x,
        .y = destPos.y + destSize.y,
        .z = 0,
        .u = sourcePos.x + sourceSize.x,
        .v = sourcePos.y + sourceSize.y,
        .r = color.r, .g = color.g, .b = color.b, .a = color.a,
    };
    stageTransferData[offset + 3] = (Vertex) {
        .x = destPos.x,
        .y = destPos.y + destSize.y,
        .z = 0,
        .u = sourcePos.x,
        .v = sourcePos.y + sourceSize.y,
        .r = color.r, .g = color.g, .b = color.b, .a = color.a,
    };
    
    spriteBatchCount++;
}

void TinyDraw_Render(
    SDL_GPUGraphicsPipeline* pipeline,
    SDL_GPUTexture* texture,
    float3 camera,
    SDL_GPUTexture* renderTarget,
    char clear
)
{
    // First, copy the vertices if there are any
    if (spriteBatchCount) {
        SDL_UploadToGPUBuffer(
            stageCopyPass,
            &(SDL_GPUTransferBufferLocation) {
                .transferBuffer = stageTransferBuffer,
                .offset = 0,
            },
            &(SDL_GPUBufferRegion) {
                .buffer = vertexBuffer,
                .offset = 0,
                .size = sizeof(Vertex) * 4 * spriteBatchCount,
            },
            SDL_FALSE
        );
        SDL_UnmapGPUTransferBuffer(device, stageTransferBuffer);
        SDL_EndGPUCopyPass(stageCopyPass);
        SDL_SubmitGPU(stageCmdbuf);
        
        stageCmdbuf = NULL;
        stageCopyPass = NULL;
        stageTransferBuffer = NULL;
        stageTransferData = NULL;
    }
    
    matrix4x4 cameraMatrix = Matrix4x4_CreateOrthographicOffCenter(
        camera.x,
        camera.x + 160,
        camera.y + 90,
        camera.y,
        0,
        -1
    );
    
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(device);
    if (cmdbuf == NULL) {
        SDL_Log("GPUAcquireCommandBuffer failed");
        return;
    }
    Uint32 w, h;
    SDL_GPUTexture* swapchainTexture = renderTarget
        ? renderTarget
        : SDL_AcquireGPUSwapchainTexture(cmdbuf, window, &w, &h);
    if (swapchainTexture != NULL)
    {
        SDL_GPUColorAttachmentInfo colorAttachmentInfo = { 0 };
        colorAttachmentInfo.texture = swapchainTexture;
        colorAttachmentInfo.clearColor = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };
        colorAttachmentInfo.loadOp = clear
            ? SDL_GPU_LOADOP_CLEAR
            : SDL_GPU_LOADOP_LOAD;
        colorAttachmentInfo.storeOp = SDL_GPU_STOREOP_STORE;
        
        // TODO: depth stencil (goes where `NULL` is here)
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorAttachmentInfo, 1, NULL);
        
        if (spriteBatchCount) {
            SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
            SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){ .buffer = vertexBuffer, .offset = 0 }, 1);
            SDL_BindGPUIndexBuffer(renderPass, &(SDL_GPUBufferBinding){ .buffer = indexBuffer, .offset = 0 }, SDL_GPU_INDEXELEMENTSIZE_16BIT);
            SDL_BindGPUFragmentSamplers(renderPass, 0, &(SDL_GPUTextureSamplerBinding){ .texture = texture, .sampler = sampler }, 1);
            SDL_PushGPUVertexUniformData(
                cmdbuf,
                0,
                &cameraMatrix,
                sizeof(matrix4x4)
            );
            SDL_DrawGPUIndexedPrimitives(renderPass, spriteBatchCount * 6, 1, 0, 0, 0);
        }

        SDL_EndGPURenderPass(renderPass);
    }
    
    SDL_SubmitGPU(cmdbuf);
    
    spriteBatchCount = 0;
}

void TinyDraw_Clear(SDL_GPUTexture* renderTarget)
{
    TinyDraw_Render(NULL, NULL, (float3){}, renderTarget, 1);
}

void TinyDraw_Destroy_Pipeline(SDL_GPUGraphicsPipeline* pipeline)
{
    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
}

void TinyDraw_Unload_Shader(SDL_GPUShader* shader)
{
    SDL_ReleaseGPUShader(device, shader);
}

void TinyDraw_Unload_Texture(SDL_GPUTexture* texture)
{
    SDL_ReleaseGPUTexture(device, texture);
}

void TinyDraw_Quit(void)
{
    TinyDraw_Unload_Shader(vertexShader);
    TinyDraw_Unload_Shader(fragmentShader);
    SDL_ReleaseGPUBuffer(device, vertexBuffer);
    SDL_ReleaseGPUBuffer(device, indexBuffer);
    SDL_ReleaseGPUSampler(device, sampler);
    SDL_UnclaimGPUWindow(device, window);
    SDL_DestroyWindow(window);
    SDL_DestroyGPUDevice(device);
}

#endif
