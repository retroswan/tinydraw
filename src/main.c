#include <SDL3/SDL.h>
#include "tinydraw/tinydraw.h"

int main(void)
{
    if (!TinyDraw_Init()) {
        return 1;
    }
    
    SDL_GPUShader* vertexShader = TinyDraw_Load_Shader("TexturedQuad.vert", 0, 1, 0, 0, SDL_GPU_SHADERSTAGE_VERTEX);
    if (vertexShader == NULL)
    {
        SDL_Log("Failed to create vertex shader!");
        return -1;
    }
    
    SDL_GPUShader* fragmentShader = TinyDraw_Load_Shader("TexturedQuad.frag", 1, 0, 0, 0, SDL_GPU_SHADERSTAGE_FRAGMENT);
    if (fragmentShader == NULL)
    {
        SDL_Log("Failed to create fragment shader!");
        return -1;
    }
    
    SDL_GPUGraphicsPipeline* pipeline = TinyDraw_Create_Pipeline(vertexShader, fragmentShader);
    
    SDL_GPUTexture* texture = TinyDraw_Load_Texture("ravioli.bmp", NULL, NULL);
    // SDL_GPUTexture* texture2 = TinyDraw_Load_Texture("ravioli_inverted.bmp", NULL, NULL);
    SDL_GPUTexture* texture2 = TinyDraw_Load_Texture("tiles_tiny_sample_2.png", NULL, NULL);
    
    SDL_GPUTexture* renderTarget = TinyDraw_Create_RenderTarget(160, 90);
    
    char quit = 0;
    SDL_Event event;
    while (!quit) {
        // Events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = 1;
            }
        }
        
        static float camX = 0, camY = 0;
        camX += 0.1f;
        camY += 0.1f;
        
        // FIXME: all of these rendering methods should use the same cmdbuf probably?
        
        // Clear
        TinyDraw_Clear(renderTarget);
        
        TinyDraw_Stage_Sprite(
            (float2){ .x = 32, .y = 48 },
            (float2){ .x = 16, .y = 16 },
            (int2){ .x = 0, .y = 0 },
            (int2){ .x = 32, .y = 32 }
        );
        TinyDraw_Render(pipeline, texture, (float3){ .x = camX, .y = camY, .z = 1.0f }, renderTarget, 0);
        
        TinyDraw_Stage_Sprite(
            (float2){ .x = 64, .y = 48 },
            (float2){ .x = 128, .y = 128 },
            (int2){ .x = 0, .y = 0 },
            (int2){ .x = 32, .y = 32 }
        );
        TinyDraw_Render(pipeline, texture2, (float3){ .x = camX, .y = camY, .z = 1.0f }, renderTarget, 0);
        
        TinyDraw_Stage_Sprite(
            (float2){ .x = 0, .y = 0 },
            (float2){ .x = 160, .y = 90 },
            (int2){ .x = 0, .y = 0 },
            (int2){ .x = 32, .y = 32 }
        );
        TinyDraw_Render(pipeline, renderTarget, (float3){ .x = 0, .y = 0, .z = 1.0f }, NULL, 1);
        
        // Sleep until next frame
        SDL_Delay(1000 / 60);
    }
    
    TinyDraw_Destroy_Pipeline(pipeline);
    
    TinyDraw_Unload_Texture(renderTarget);
    TinyDraw_Unload_Texture(texture);
    TinyDraw_Unload_Texture(texture2);
    TinyDraw_Unload_Shader(fragmentShader);
    TinyDraw_Unload_Shader(vertexShader);
    
    TinyDraw_Quit();
    
    return 0;
}
