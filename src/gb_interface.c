#include "gb_interface.h"
#include "gb_emulator.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
SDL_Surface *screen;
void gb_initialize_interface()
{
    SDL_Init(SDL_INIT_VIDEO);
    printf("SDL INIT\n");
    TTF_Init();
    printf("TTF INIT\n");

    SDL_CreateWindowAndRenderer(1024, 768, 0, &sdlWindow, &sdlRenderer);
}
void delay(uint32_t ms) {
    SDL_Delay(ms);
}
void gb_interface_handle_events()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) > 0)
    {
        //TODO SDL_UpdateWindowSurface(sdlWindow);
        //TODO SDL_UpdateWindowSurface(sdlTraceWindow);
        //TODO SDL_UpdateWindowSurface(sdlDebugWindow);

        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
            gb_emulator_get_info()->is_killed=true;
        }
    }
}