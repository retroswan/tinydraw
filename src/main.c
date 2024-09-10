#include <SDL3/SDL.h>
#include "tinydraw/tinydraw.h"

int main(void)
{
    if (!TinyDraw_Init()) {
        return 1;
    }
    
    TinyDraw_Begin_Load();
    TinyDraw_Load_IndexBuffer();
    SDL_GPUTexture* textureRavioli = TinyDraw_Load_Texture("ravioli.bmp");
    TinyDraw_End_Load();
    
    char quit = 0;
    SDL_Event event;
    while (!quit) {
        // Events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = 1;
            }
        }
        
        TinyDraw_Stage_Sprite(
            (float2){ .x = 32, .y = 48 },
            (float2){ .x = 16, .y = 16 },
            (int2){ .x = 0, .y = 0 },
            (int2){ .x = 0, .y = 0 }
        );
        
        TinyDraw_Begin_Load();
        TinyDraw_Render(
            textureRavioli,
            (float3){ .x = 0, .y = 0, .z = 1 },
            NULL
        );
        TinyDraw_EndFrame();
        TinyDraw_End_Load();
        
        // Sleep until next frame
        SDL_Delay(1000 / 60);
    }
    
    TinyDraw_Unload_Texture(textureRavioli);
    
    TinyDraw_Quit();
    
    return 0;
}
