#include <SDL3/SDL.h>
#include "tinydraw/tinydraw.h"

int main(void)
{
    TinyDraw_Init();
    
    char quit = 0;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = 1;
            }
        }
        
        SDL_Delay(1000 / 60);
    }
    
    TinyDraw_Quit();
    
    return 0;
}
