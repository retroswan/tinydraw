#include "tinydraw.h"
#define SDL_GPU_SHADERCROSS_IMPLEMENTATION
#include <SDL_gpu_shadercross.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC SDL_malloc
#define STBI_REALLOC SDL_realloc
#define STBI_FREE SDL_free
#define STBI_ONLY_HDR
#include "../vendor/stb_image.h"

#define SPRITE_COUNT 1024

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

// SDL_GPU spritebatch
static SDL_GPUTransferBuffer* vertexBufferTransferBuffer = NULL;
static SDL_GPUBuffer* indexBuffer;
static SDL_GPUBuffer* vertexBuffer;
static int spriteBatchCount = 0;

// SDL_GPU misc
static SDL_GPUDevice* device;
static SDL_GPUSampler* sampler;
static SDL_Window* window;

// SDL_GPU assets
static SDL_GPUShader* vertexShader;
static SDL_GPUShader* fragmentShader;

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
    
    SDL_SetGPUTextureName(
        device,
        renderTarget,
        "Render Target"
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
                    .dstColorBlendFactor = SDL_GPU_BLENDFACTOR_ZERO,
                    .dstAlphaBlendFactor = SDL_GPU_BLENDFACTOR_ZERO
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

SDL_GPUTexture* TinyDraw_Load_Texture(
    const char* fileName,
    int* width,
    int* height
)
{
    SDL_Surface* imageData;
    SDL_PixelFormat format = SDL_PIXELFORMAT_ABGR8888;
    SDL_snprintf(fullPath, sizeof(fullPath), "%sContent/Images/%s", basePath, fileName);
    imageData = SDL_LoadBMP(fullPath);
    if (imageData == NULL) {
        SDL_Log("Failed to load BMP: %s", SDL_GetError());
        return NULL;
    }
    
    if (imageData->format != format) {
        SDL_Surface *next = SDL_ConvertSurface(imageData, format);
        SDL_DestroySurface(imageData);
        imageData = next;
    }
    
    if (width != NULL) {
        *width = imageData->w;
    }
    
    if (height != NULL) {
        *height = imageData->h;
    }
    
    SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &(SDL_GPUTextureCreateInfo){
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .width = imageData->w,
        .height = imageData->h,
        .layerCountOrDepth = 1,
        .levelCount = 1,
        .usageFlags = SDL_GPU_TEXTUREUSAGE_SAMPLER_BIT
    });
    SDL_SetGPUTextureName(
        device,
        texture,
        "Ravioli Texture 🖼️"
    );
    
    SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(
        device,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .sizeInBytes = imageData->w * imageData->h * 4
        }
    );
    Uint8* textureTransferPtr = SDL_MapGPUTransferBuffer(
        device,
        textureTransferBuffer,
        SDL_FALSE
    );
    SDL_memcpy(textureTransferPtr, imageData->pixels, imageData->w * imageData->h * 4);
    SDL_UnmapGPUTransferBuffer(device, textureTransferBuffer);
    
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
    SDL_UploadToGPUTexture(
        copyPass,
        &(SDL_GPUTextureTransferInfo) {
            .transferBuffer = textureTransferBuffer,
            .offset = 0, /* Zeros out the rest */
        },
        &(SDL_GPUTextureRegion){
            .texture = texture,
            .w = imageData->w,
            .h = imageData->h,
            .d = 1
        },
        SDL_FALSE
    );
    SDL_DestroySurface(imageData);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPU(uploadCmdBuf);
    SDL_ReleaseGPUTransferBuffer(device, textureTransferBuffer);
    
    return texture;
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

void TinyDraw_Stage_Sprite(float2 destPos, float2 destSize, int2 sourcePos, int2 sourceSize)
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(device);
    if (cmdbuf == NULL) {
        SDL_Log("GPUAcquireCommandBuffer failed");
        return;
    }
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdbuf);
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
    
    // TODO: texture placement; we're just drawing the whole thing here
    // TODO: render more than one sprite
    transferData[0] = (Vertex) {
        .x = destPos.x,
        .y = destPos.y,
        .z = 0,
        .u = 0,
        .v = 0,
        .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f,
    };
    transferData[1] = (Vertex) {
        .x = destPos.x + destSize.x,
        .y = destPos.y,
        .z = 0,
        .u = 1,
        .v = 0,
        .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f,
    };
    transferData[2] = (Vertex) {
        .x = destPos.x + destSize.x,
        .y = destPos.y + destSize.y,
        .z = 0,
        .u = 1,
        .v = 1,
        .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f,
    };
    transferData[3] = (Vertex) {
        .x = destPos.x,
        .y = destPos.y + destSize.y,
        .z = 0,
        .u = 0,
        .v = 1,
        .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f,
    };
    
    spriteBatchCount++;
    
    SDL_UploadToGPUBuffer(
        copyPass,
        &(SDL_GPUTransferBufferLocation) {
            .transferBuffer = bufferTransferBuffer,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = vertexBuffer,
            .offset = 0,
            .size = sizeof(Vertex) * 4 * spriteBatchCount
        },
        SDL_FALSE
    );
    SDL_UnmapGPUTransferBuffer(device, bufferTransferBuffer);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPU(cmdbuf);
}

void TinyDraw_Render(
    SDL_GPUGraphicsPipeline* pipeline,
    SDL_GPUTexture* texture,
    float3 camera,
    SDL_GPUTexture* renderTarget,
    char clear
)
{
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
        // FIXME: should only clear sometimes, not sure when??
        colorAttachmentInfo.loadOp = clear
            ? SDL_GPU_LOADOP_CLEAR
            : SDL_GPU_LOADOP_LOAD;
        colorAttachmentInfo.storeOp = SDL_GPU_STOREOP_STORE;
        
        // TODO: depth stencil stuff here, where `NULL` is
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorAttachmentInfo, 1, NULL);
        
        if (texture && spriteBatchCount) {
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
