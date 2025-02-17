#include "gb_ppu.h"
#include <string.h>
#include "gb_lcd.h"
#include "gb_cpu.h"
#include "SDL2/SDL.h"
static uint32_t target_frame_time=1000/60;
static long previous_frame_time=0;
static long start_timer = 0;
static long frame_count = 0;

static struct gb_ppu_info ppu_info;
struct gb_ppu_info* gb_ppu_get_info()
{
    return &ppu_info;
}

void gb_ppu_initialize()
{
    ppu_info.line_ticks=0;
    ppu_info.current_frame=0;
    ppu_info.video_buffer=malloc(Y_RESOLUTION*X_RESOLUTION*sizeof(uint32_t));
    gb_lcd_initialize();
    gb_lcd_set_ppu_mode(OAMSCAN);
    memset(ppu_info.oam_ram,0,sizeof(ppu_info.oam_ram));
    memset(ppu_info.video_buffer,0,Y_RESOLUTION*X_RESOLUTION*sizeof(uint32_t));
}


void gb_ppu_oam_write(uint8_t value, uint16_t address)
{
    if(address>=0xFE00) //Have to check because DMA may call it relatively
        address-=0xFE00;
    uint8_t *p=(uint8_t*)ppu_info.oam_ram;
    p[address]=value;
    return;
}
uint8_t gb_ppu_oam_read(uint16_t address)
{
    if(address>=0xFE00)
        address-=0xFE00;
    uint8_t *p=(uint8_t*)ppu_info.oam_ram;
    return p[address];
}

void gb_ppu_vram_write(uint8_t value, uint16_t address)
{
    address-=0x8000;
    ppu_info.vram[address]=value;
    return;
}
uint8_t gb_ppu_vram_read(uint16_t address)
{
    address-=0x8000;
    return ppu_info.vram[address];
}


void gb_ppu_hblank()
{
    if(ppu_info.line_ticks>=DOTS_PER_LINE)
    {
        gb_get_lcd_info()->lcd_y++;
        if(gb_get_lcd_info()->lcd_y==gb_get_lcd_info()->ly_compare)
        {
            gb_lcd_set_lyc_ly_comparison(1);
            if(gb_lcd_is_interrupt_selected(LYC_SELECT))
            {
                gb_cpu_request_interrupt(IT_LCD_STAT);
            }
        }
        else{
            gb_lcd_set_lyc_ly_comparison(0);
        }
        if(gb_get_lcd_info()->lcd_y>=Y_RESOLUTION)
        {
            gb_lcd_set_ppu_mode(VBLANK);
            gb_cpu_request_interrupt(IT_VBLANK);
            if(gb_lcd_is_interrupt_selected(VBLANK))
            {
                gb_cpu_request_interrupt(IT_LCD_STAT);
            }
            ppu_info.current_frame++;

            //Current fps
            uint32_t current_tick=SDL_GetTicks();
            uint32_t frame_time=current_tick-previous_frame_time;

            if (frame_time < target_frame_time) {
                SDL_Delay((target_frame_time - frame_time));
            }

            if (current_tick - start_timer >= 1000) {
                uint32_t fps = frame_count;
                start_timer = current_tick;
                frame_count = 0;

                printf("FPS: %d\n", fps);
            }

            frame_count++;
            previous_frame_time = SDL_GetTicks();
        }
        else{
            gb_lcd_set_ppu_mode(OAMSCAN);
        }
        ppu_info.line_ticks=0;
    }
}

void gb_ppu_vblank()
{
    if(ppu_info.line_ticks>=DOTS_PER_LINE)
    {
        gb_get_lcd_info()->lcd_y++;
        if(gb_get_lcd_info()->lcd_y==gb_get_lcd_info()->ly_compare)
        {
            gb_lcd_set_lyc_ly_comparison(1);
            if(gb_lcd_is_interrupt_selected(LYC_SELECT))
            {
                gb_cpu_request_interrupt(IT_LCD_STAT);
            }
        }
        else{
            gb_lcd_set_lyc_ly_comparison(0);
        }
        if(gb_get_lcd_info()->lcd_y>=LINES_PER_FRAME)
        {
            gb_lcd_set_ppu_mode(OAMSCAN);
            gb_get_lcd_info()->lcd_y=0;
        }
        ppu_info.line_ticks=0;

    }
}

void gb_ppu_oam_scan()
{
    if(ppu_info.line_ticks>=80)
    {
        gb_lcd_set_ppu_mode(DRAWING);
    }
}

void gb_ppu_drawing()
{
    if(ppu_info.line_ticks>=80+172)
    {
        gb_lcd_set_ppu_mode(HBLANK);
    }
}

void gb_ppu_tick()
{
    ppu_info.line_ticks++;
    switch(gb_lcd_get_ppu_mode())
    {
        case HBLANK:
            gb_ppu_hblank();
            break;
        case VBLANK:
            gb_ppu_vblank();
            break;
        case OAMSCAN:
            gb_ppu_oam_scan();
            break;
        case DRAWING:
            gb_ppu_drawing();
            break;
    }
}