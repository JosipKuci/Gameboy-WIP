#include "gb_ppu.h"
#include <string.h>
#include "gb_lcd.h"
#include "gb_ppu_pipeline.h"
#include "gb_cpu.h"
#include "gb_cartridge.h"
#include "SDL2/SDL.h"
static uint32_t target_frame_time=1000/60;
static long previous_frame_time=0;
static long start_timer = 0;
static long frame_count = 0;

struct gb_ppu_info ppu_info;
struct gb_ppu_info* gb_ppu_get_info()
{
    return &ppu_info;
}

void gb_ppu_initialize()
{
    ppu_info.line_ticks=0;
    ppu_info.current_frame=0;
    ppu_info.video_buffer=malloc(Y_RESOLUTION*X_RESOLUTION*sizeof(32));
    ppu_info.window_line=0;

    //FIFO
    ppu_info.FIFO_info.line_x_pos=0;
    ppu_info.FIFO_info.pushed_x=0;
    ppu_info.FIFO_info.fetched_x=0;
    ppu_info.FIFO_info.FIFO_queue.lenght=0;
    ppu_info.FIFO_info.FIFO_queue.first_element=NULL;
    ppu_info.FIFO_info.FIFO_queue.last_element=NULL;
    ppu_info.FIFO_info.fetch_state=GET_TILE;


    ppu_info.sprites_in_line=0;
    ppu_info.fetched_entry_count=0;
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
uint8_t gb_ppu_is_window_visible()
{
    return gb_lcd_is_window_enabled() && gb_get_lcd_info()->window_x>=0 && gb_get_lcd_info()->window_x <= 166 && gb_get_lcd_info()->window_y >= 0 && gb_get_lcd_info()->window_y < Y_RESOLUTION;
}

void gb_ppu_hblank()
{
    if(ppu_info.line_ticks>=DOTS_PER_LINE)
    {
        if(gb_ppu_is_window_visible() && gb_get_lcd_info()->lcd_y >= gb_get_lcd_info()->window_y && gb_get_lcd_info()->lcd_y < gb_get_lcd_info()->window_y+Y_RESOLUTION)
        {
            ppu_info.window_line++;
        }
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
            if(gb_lcd_is_interrupt_selected(MODE1_SELECT))
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

                if(gb_cartridge_does_cartridge_need_save())
                {
                    gb_cartridge_save_battery();
                }
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
        if(gb_ppu_is_window_visible() && gb_get_lcd_info()->lcd_y >= gb_get_lcd_info()->window_y && gb_get_lcd_info()->lcd_y < gb_get_lcd_info()->window_y+Y_RESOLUTION)
        {
            ppu_info.window_line++;
        }
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
            ppu_info.window_line=0;
        }
        ppu_info.line_ticks=0;

    }
}

void gb_ppu_load_sprites_in_line()
{
    int current_y_line=gb_get_lcd_info()->lcd_y;
    uint8_t sprite_height=gb_lcd_get_obj_size();
    memset(ppu_info.list_of_sprite_line_entries,0,sizeof(ppu_info.list_of_sprite_line_entries));
    //Going through all OAM entries
    for(int i=0;i<40;i++)
    {
        struct gb_oam_data oam_data=ppu_info.oam_ram[i];
        if(oam_data.x==0)//Means data is not visible
        {
            continue;
        }
        if(ppu_info.num_of_sprites>=10)
        {
            break;
        }
        if(oam_data.y <= current_y_line+16 && oam_data.y + sprite_height > current_y_line+16)
        {
            //Sprite is on current line
            struct gb_oam_in_line* current_line_object = &ppu_info.list_of_sprite_line_entries[ppu_info.num_of_sprites++]; 
            current_line_object->object=oam_data;
            current_line_object->next_object=NULL;
            if(!ppu_info.sprites_in_line || ppu_info.sprites_in_line->object.x > oam_data.x)
            {
                current_line_object->next_object=ppu_info.sprites_in_line;
                ppu_info.sprites_in_line=current_line_object;
                continue;
            }
            //Sorting of oam line objects
            struct gb_oam_in_line *temp = ppu_info.sprites_in_line;
            struct gb_oam_in_line *previous=temp;
            while(temp)
            {
                if(temp->object.x>oam_data.x)
                {
                    previous->next_object=current_line_object;
                    current_line_object->next_object=temp;
                    break;
                }

                if(!temp->next_object)
                {
                    temp->next_object=current_line_object;
                    break;
                }

                previous=temp;
                temp=temp->next_object;
            }
        }

    }
}

void gb_ppu_oam_scan()
{
    if(ppu_info.line_ticks>=80)
    {
        gb_lcd_set_ppu_mode(DRAWING);
        ppu_info.FIFO_info.fetch_state=GET_TILE;
        ppu_info.FIFO_info.line_x_pos=0;
        ppu_info.FIFO_info.fetched_x=0;
        ppu_info.FIFO_info.pushed_x=0;
        ppu_info.FIFO_info.fifo_x=0;
    }

    if(ppu_info.line_ticks==1)
    {
        ppu_info.sprites_in_line=0;
        ppu_info.num_of_sprites=0;

        gb_ppu_load_sprites_in_line();
    }
}

void gb_ppu_drawing()
{
    gb_ppu_pipeline_process();
    if(ppu_info.FIFO_info.pushed_x>=X_RESOLUTION)
    {
        gb_ppu_pipeline_FIFO_reset();
        gb_lcd_set_ppu_mode(HBLANK);
        if(gb_lcd_is_interrupt_selected(MODE0_SELECT))
        {
            gb_cpu_request_interrupt(IT_LCD_STAT);
        }
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