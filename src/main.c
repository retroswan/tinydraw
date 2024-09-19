#define TINYDRAW_IMPLEMENTATION
#include "tinydraw.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC SDL_malloc
#define STBI_REALLOC SDL_realloc
#define STBI_FREE SDL_free
#include "vendor/stb_image.h"

#define FRAME(X, Y, WIDTH, HEIGHT, TWIDTH, THEIGHT) \
    (float2){ .x = X * (WIDTH / TWIDTH), .y = Y * (HEIGHT / THEIGHT) }, \
    (float2){ .x = (WIDTH / TWIDTH), .y = (HEIGHT / THEIGHT) }

int main(void)
{
    if (!TinyDraw_Init()) {
        return 1;
    }
    
    SDL_GPUShader* vertexShader = TinyDraw_Load_Shader("sprite.vert", 0, 1, 0, 0, SDL_GPU_SHADERSTAGE_VERTEX);
    if (vertexShader == NULL)
    {
        SDL_Log("Failed to create vertex shader!");
        return -1;
    }
    
    SDL_GPUShader* fragmentShader = TinyDraw_Load_Shader("sprite.frag", 1, 0, 0, 0, SDL_GPU_SHADERSTAGE_FRAGMENT);
    if (fragmentShader == NULL)
    {
        SDL_Log("Failed to create fragment shader!");
        return -1;
    }
    
    SDL_GPUGraphicsPipeline* pipeline = TinyDraw_Create_Pipeline(vertexShader, fragmentShader);
    
    SDL_GPUTexture* texture = TinyDraw_Load_Texture("paving 1.png", NULL, NULL);
    SDL_GPUTexture* texture2 = TinyDraw_Load_Texture("tiles_tiny_sample_2.png", NULL, NULL);
    
    SDL_GPUTexture* renderTarget = TinyDraw_Create_RenderTarget(160, 90);
    
    float X = 64, Y = 0;
    
    char quit = 0;
    SDL_Event event;
    while (!quit) {
        // Events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT: {
                    quit = 1;
                } break;
                
                case SDL_EVENT_KEY_DOWN: {
                    switch (event.key.key) {
                        case SDLK_D: {
                            static char fullscreen = 0;
                            TinyDraw_Resize(640, 360, (fullscreen = !fullscreen));
                        } break;
                        
                        case SDLK_RIGHT: {
                            X++;
                        } break;
                        
                        case SDLK_DOWN: {
                            Y++;
                        } break;
                    }
                } break;
            }
        }
        
        static float camX = 0, camY = 0;
        // camX += 0.1f;
        // camY += 0.1f;
        
        // Clear
        TinyDraw_Clear(renderTarget);
        
        float tilesize = 25;
        TinyDraw_Stage_Sprite(
            (float2){ .x = 0, .y = 48 },
            (float2){ .x = tilesize, .y = tilesize },
            FRAME(0, 1, tilesize, tilesize, 250.0f, 250.0f),
            (Color){ 1, 1, 1, 1 }
        );
        TinyDraw_Render(pipeline, texture2, (float3){ .x = camX, .y = camY, .z = 1.0f }, renderTarget, 0);
        
        TinyDraw_Stage_Sprite(
            (float2){ .x = X, .y = Y },
            (float2){ .x = 64, .y = 64 },
            (float2){ .x = 0, .y = 0 },
            (float2){ .x = 1, .y = 1 },
            (Color){ 1, 1, 1, 1 }
        );
        TinyDraw_Render(pipeline, texture, (float3){ .x = camX, .y = camY, .z = 1.0f }, renderTarget, 0);
        
        TinyDraw_Stage_Sprite(
            (float2){ .x = 0, .y = 0 },
            (float2){ .x = 160, .y = 90 },
            (float2){ .x = 0, .y = 0 },
            (float2){ .x = 1, .y = 1 },
            (Color){ 1, 1, 1, 1 }
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
