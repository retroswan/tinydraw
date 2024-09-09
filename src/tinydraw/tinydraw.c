#include <SDL3/SDL.h>
#include "tinydraw.h"

static SDL_Window* window;

void TinyDraw_Init(void)
{
    // TODO: error handling
    SDL_Init(SDL_INIT_VIDEO);
    
    window = SDL_CreateWindow("TinyDraw Test", 1280, 720, 0);
}

void TinyDraw_Quit(void)
{
    SDL_DestroyWindow(window);
}
