#include "gb_interface.h"
#include "gb_emulator.h"
#include "gb_ppu.h"
#include "gb_data_bus.h"
#include "gb_joypad.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

//Rendering the main screen
SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
SDL_Surface *screen;

//Render the debug window
SDL_Window *sdlDebugWindow;
SDL_Renderer *sdlDebugRenderer;
SDL_Texture *sdlDebugTexture;
SDL_Surface *DebugScreen;

static int scale=4;
static unsigned long color_pallette[4]={0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};
void gb_initialize_interface()
{
    SDL_Init(SDL_INIT_VIDEO);
    printf("SDL INIT\n");
    TTF_Init();
    printf("TTF INIT\n");

    //Window for main screen
    SDL_CreateWindowAndRenderer(1024, 768, 0, &sdlWindow, &sdlRenderer);

    screen = SDL_CreateRGBSurface(0, 1024, 768, 32,
        0x00FF0000,
        0x0000FF00,
        0x000000FF,
        0xFF000000);
    sdlTexture = SDL_CreateTexture(sdlRenderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            1024, 768);


    //Window for debug screen (tilemap)
    SDL_CreateWindowAndRenderer(16*8*scale, 768, 0, &sdlDebugWindow, &sdlDebugRenderer);


    DebugScreen = SDL_CreateRGBSurface(0, (16 * 8 * scale) + (16 * scale), 
                                            (32 * 8 * scale) + (64 * scale), 32,
                                            0x00FF0000,
                                            0x0000FF00,
                                            0x000000FF,
                                            0xFF000000);

    sdlDebugTexture=SDL_CreateTexture(sdlDebugRenderer,SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,(16*8*scale)+(16*scale),(32*8*scale)+(64*scale));

    int x,y;
    SDL_GetWindowPosition(sdlWindow,&x,&y);
    SDL_SetWindowPosition(sdlDebugWindow,x+1034,y);//Putting the debig screen next to the main screen
}
void gb_display_tile(SDL_Surface *DebugScreen, uint16_t vram_address, uint16_t tile_num, int x, int y)
{
    SDL_Rect rectangle;
    for(int tile_y=0;tile_y<16;tile_y+=2)
    {
        uint8_t byte1 = gb_bus_read(vram_address+(tile_num*16)+tile_y);
        uint8_t byte2 = gb_bus_read(vram_address+(tile_num*16)+tile_y+1);

        for(int bit=7;bit>=0;bit--)
        {
            uint8_t hi=!!(byte1&(1<<bit))<<1;
            uint8_t lo=!!(byte2&(1<<bit));

            uint8_t color=hi|lo;
            rectangle.x=x+((7-bit)*scale);
            rectangle.y=y+(tile_y/2*scale);
            rectangle.w=scale;
            rectangle.h=scale;

            SDL_FillRect(DebugScreen,&rectangle,color_pallette[color]);
        }
    }
}

void gb_ui_update_debug_window()
{
    int x_pos=0;
    int y_pos=0;
    int tile_num=0;

    SDL_Rect rectangle;
    rectangle.x=0;
    rectangle.y=0;
    rectangle.w=DebugScreen->w;
    rectangle.h=DebugScreen->h;
    SDL_FillRect(DebugScreen,&rectangle,0xFF111111);

    uint16_t vram_address=0x8000;

    //384 tiles, 24x16

    for(int y=0;y<24;y++)
    {
        for(int x=0;x<16;x++)
        {
            gb_display_tile(DebugScreen,vram_address,tile_num,x_pos+(x*scale),y_pos+(y*scale));
            x_pos+=(8*scale);
            tile_num++;
        }
        y_pos+=(8*scale);
        x_pos=0;
    }
    SDL_UpdateTexture(sdlDebugTexture, NULL, DebugScreen->pixels, DebugScreen->pitch);
	SDL_RenderClear(sdlDebugRenderer);
	SDL_RenderCopy(sdlDebugRenderer, sdlDebugTexture, NULL, NULL);
	SDL_RenderPresent(sdlDebugRenderer);
}

void gb_ui_update_window()
{
    SDL_Rect rc;
    rc.x = rc.y = 0;
    rc.w = rc.h = 2048;

    uint32_t *video_buffer = gb_ppu_get_info()->video_buffer;
    for (int line_num = 0; line_num < Y_RESOLUTION; line_num++) {
        for (int x = 0; x < X_RESOLUTION; x++) {
            rc.x = x * scale;
            rc.y = line_num * scale;
            rc.w = scale;
            rc.h = scale;

            SDL_FillRect(screen, &rc, video_buffer[x + (line_num * X_RESOLUTION)]);
        }
    }

    SDL_UpdateTexture(sdlTexture, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
    gb_ui_update_debug_window();
}

void delay(uint32_t ms) {
    SDL_Delay(ms);
}
gb_interface_press_key(uint32_t key_pressed)
{
    //Up
    if(key_pressed==SDLK_w)
    {
        *gb_joypad_get_joypad_dpad_values()&=~(0b00000100);
    }
    //Down
    else if(key_pressed==SDLK_s)
    {
        *gb_joypad_get_joypad_dpad_values()&=~(0b00001000);
    }
    //Left
    else if(key_pressed==SDLK_a)
    {
        *gb_joypad_get_joypad_dpad_values()&=~(0b10);
    }
    //Right
    else if(key_pressed==SDLK_d)
    {
        *gb_joypad_get_joypad_dpad_values()&=~(0b1);
    }
    //Start
    else if(key_pressed==SDLK_RETURN)
    {
        *gb_joypad_get_joypad_button_values()&=~(0b00001000);
    }
    //Select
    else if(key_pressed==SDLK_TAB)
    {
        *gb_joypad_get_joypad_button_values()&=~(0b00000100);
    }
    //B
    else if(key_pressed==SDLK_o)
    {
        *gb_joypad_get_joypad_button_values()&=~(0b10);
        
    }
    //A
    else if(key_pressed==SDLK_p)
    {
        *gb_joypad_get_joypad_button_values()&=~(0b1);
        
    }

}

gb_interface_depress_key(uint32_t key_pressed)
{
    //Up
    if(key_pressed==SDLK_w)
    {
        *gb_joypad_get_joypad_dpad_values()|=(0b00000100);
    }
    //Down
    else if(key_pressed==SDLK_s)
    {
        *gb_joypad_get_joypad_dpad_values()|=(0b00001000);
        
    }
    //Left
    else if(key_pressed==SDLK_a)
    {
        *gb_joypad_get_joypad_dpad_values()|=(0b10);
    }
    //Right
    else if(key_pressed==SDLK_d)
    {
        *gb_joypad_get_joypad_dpad_values()|=(0b1);
        
    }
    //Start
    else if(key_pressed==SDLK_RETURN)
    {
        *gb_joypad_get_joypad_button_values()|=(0b00001000);
        
    }
    //Select
    else if(key_pressed==SDLK_TAB)
    {
        *gb_joypad_get_joypad_button_values()|=(0b00000100);
        
    }
    //B
    else if(key_pressed==SDLK_o)
    {
        *gb_joypad_get_joypad_button_values()|=(0b10);
    }
    //A
    else if(key_pressed==SDLK_p)
    {
        *gb_joypad_get_joypad_button_values()|=(0b1);
    }

}

void gb_interface_handle_events()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) > 0)
    {
        if(e.type==SDL_KEYDOWN)
        {
            gb_interface_press_key(e.key.keysym.sym);
        }

        if(e.type == SDL_KEYUP)
        {
            gb_interface_depress_key(e.key.keysym.sym);
        }

        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
            gb_emulator_get_info()->is_killed=true;
        }
    }
}