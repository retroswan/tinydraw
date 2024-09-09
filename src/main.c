#include <SDL3/SDL.h>
#include "tinydraw/tinydraw.h"

int main(void)
{
    if (!TinyDraw_Init()) {
        return 1;
    }
    
    SDL_GPUShader* shaderVertex = TinyDraw_Load_Shader(
        "TexturedQuad.vert",
        0,
        1,
        0,
        0,
        SDL_GPU_SHADERSTAGE_VERTEX
    );
    SDL_GPUShader* shaderFragment = TinyDraw_Load_Shader(
        "TexturedQuad.frag",
        1,
        0,
        0,
        0,
        SDL_GPU_SHADERSTAGE_FRAGMENT
    );
    if (shaderVertex == NULL || shaderFragment == NULL) {
        return 1;
    }
    
    char quit = 0;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = 1;
            }
        }
        
        // Do all the stuff here
        
        SDL_Delay(1000 / 60);
    }
    
    TinyDraw_Quit();
    
    return 0;
}
