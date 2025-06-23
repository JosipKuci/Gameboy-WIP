#include "gb_ppu.h"
#include "gb_ppu_pipeline.h"
#include "gb_lcd.h"
#include "gb_data_bus.h"
#include <stdlib.h>

enum sprite_load_type{
    DATA_LOW,
    DATA_HIGH,
};
void gb_FIFO_push(uint32_t value)
{
    struct gb_FIFO_element* element = malloc(sizeof(struct gb_FIFO_element));
    element->color_value=value;
    element->next_element=NULL;
    if(gb_ppu_get_info()->FIFO_info.FIFO_queue.first_element==NULL)
    {
        gb_ppu_get_info()->FIFO_info.FIFO_queue.first_element=element;
        gb_ppu_get_info()->FIFO_info.FIFO_queue.last_element=element;
    }
    else
    {
        gb_ppu_get_info()->FIFO_info.FIFO_queue.last_element->next_element=element;
        gb_ppu_get_info()->FIFO_info.FIFO_queue.last_element=element;
    }
    gb_ppu_get_info()->FIFO_info.FIFO_queue.lenght++;
}

uint32_t gb_FIFO_pop()
{
    struct gb_FIFO_element* popped_element=gb_ppu_get_info()->FIFO_info.FIFO_queue.first_element;
    gb_ppu_get_info()->FIFO_info.FIFO_queue.first_element=gb_ppu_get_info()->FIFO_info.FIFO_queue.first_element->next_element;
    uint32_t value=popped_element->color_value;
    gb_ppu_get_info()->FIFO_info.FIFO_queue.lenght--;
    free(popped_element);
    return value;
}

uint32_t gb_ppu_fetch_sprite_pixels(int current_bit, uint32_t color, uint8_t background_color)
{
    for(int i=0;i<gb_ppu_get_info()->fetched_entry_count;i++)
    {
        int sprite_x_pos=(gb_ppu_get_info()->fetched_entries[i].x-8)+((gb_get_lcd_info()->scroll_x%8));
        if(sprite_x_pos+8<gb_ppu_get_info()->FIFO_info.fifo_x)
        {
            //We have already passed the current pixel point
            continue;
        }
        int offset = gb_ppu_get_info()->FIFO_info.fifo_x-sprite_x_pos;
        //If offset is out of bounds or larger than 8 bit
        if(offset<0 || offset>7)
        {
            continue;
        }
        current_bit=7-offset;
        //Check for x-flip
        if(gb_ppu_get_info()->fetched_entries[i].attributes&0b00100000)//X-flip check
        {
            current_bit=offset;
        }

        uint8_t lo=!!(gb_ppu_get_info()->FIFO_info.oam_data[i*2]&(1<<current_bit));
        uint8_t hi=!!(gb_ppu_get_info()->FIFO_info.oam_data[(i*2)+1]&(1<<current_bit))<<1;

        uint8_t sprite_bg_priority = !!(gb_ppu_get_info()->fetched_entries[i].attributes&0b10000000);

        if(!(hi|lo))
        {
            //color of sprite is transparent
            continue;
        }
        if(!sprite_bg_priority || background_color==0)
        {
            if(gb_ppu_get_info()->fetched_entries[i].attributes & 0b00010000) //Check dmg pallette of OAM
            {
                color=gb_get_lcd_info()->sprite2_colors[hi|lo];
            }
            else
            {
                color=gb_get_lcd_info()->sprite1_colors[hi|lo];
            }
            if(hi|lo)
            {
                //We know this is a pixel value we want to display
                break;
            }
        }
    }
    return color;
}

void  gb_ppu_pipline_load_sprite_tile()
{
    struct gb_oam_in_line *temp = gb_ppu_get_info()->sprites_in_line;
    while(temp)
    {
        int sprite_x_pos= (temp->object.x-8)+(gb_get_lcd_info()->scroll_x%8);
        if((sprite_x_pos>=gb_ppu_get_info()->FIFO_info.fetched_x && sprite_x_pos < (gb_ppu_get_info()->FIFO_info.fetched_x+8)) || ((sprite_x_pos+8)>=gb_ppu_get_info()->FIFO_info.fetched_x && (sprite_x_pos+8)<gb_ppu_get_info()->FIFO_info.fetched_x+8))
        {
            //Entry needs to be added
            gb_ppu_get_info()->fetched_entries[gb_ppu_get_info()->fetched_entry_count++]=temp->object;
        }
        temp=temp->next_object;
        if( !temp || gb_ppu_get_info()->fetched_entry_count>=3)
        {
            //The most number of sprites we can check on an 8bit section
            break;
        }
    }
}

void gb_ppu_pipeline_load_sprite_data(enum sprite_load_type load_type)
{
    int current_y_pos = gb_get_lcd_info()->lcd_y;
    uint8_t sprite_height=gb_lcd_get_obj_size();
    for(int i=0;i<gb_ppu_get_info()->fetched_entry_count;i++)
    {
        uint8_t tile_y_pos=((current_y_pos+16)-gb_ppu_get_info()->fetched_entries[i].y)*2;
        if (gb_ppu_get_info()->fetched_entries[i].attributes&0b01000000) //Checking Y-flip OAM attribute
        {
            tile_y_pos=(sprite_height*2-2)-tile_y_pos;
        }
        uint8_t tile_index = gb_ppu_get_info()->fetched_entries[i].tile_index;
        if(sprite_height==16)
        {
            tile_index&=~(1); //Remove last bit as it is not needed
        }
        gb_ppu_get_info()->FIFO_info.oam_data[(i*2)+load_type]=gb_bus_read(0x8000 + (tile_index*16)+tile_y_pos+load_type);
    }
}
//Picks the tile for the window if window is enabled
void gb_ppu_pipeline_load_window_tile()
{
    if(gb_ppu_is_window_visible())
    {
    if(gb_ppu_get_info()->FIFO_info.fetched_x+7 >= gb_get_lcd_info()->window_x && gb_ppu_get_info()->FIFO_info.fetched_x+7<gb_get_lcd_info()->window_x+Y_RESOLUTION+14)
    {
        if(gb_get_lcd_info()->lcd_y >= gb_get_lcd_info()->window_y && gb_get_lcd_info()->lcd_y < gb_get_lcd_info()->window_y + X_RESOLUTION)
        {
            uint8_t window_tile_y_pos = gb_ppu_get_info()->window_line/8;
            gb_ppu_get_info()->FIFO_info.bg_data[0]=gb_bus_read(gb_lcd_get_window_map_area()+
            ((gb_ppu_get_info()->FIFO_info.fetched_x+7-gb_get_lcd_info()->window_x)/8)+
            (window_tile_y_pos*32));
            if(gb_lcd_get_window_data_area()==0x8800) //If it is in this area, increment by 128
            {
                gb_ppu_get_info()->FIFO_info.bg_data[0]+=128;
            }
        }
    }
    }
}


void gb_ppu_pipeline_fetch()
{
    switch(gb_ppu_get_info()->FIFO_info.fetch_state)
    {
        case GET_TILE:
        {
            gb_ppu_get_info()->fetched_entry_count=0;

            if(gb_lcd_is_bg_enabled())
            {
                gb_ppu_get_info()->FIFO_info.bg_data[0]=gb_bus_read(gb_lcd_get_tile_map_area()+(gb_ppu_get_info()->FIFO_info.map_x/8)+((gb_ppu_get_info()->FIFO_info.map_y/8)*32)); 
                if(gb_lcd_get_window_data_area()==0x8800)
                {
                gb_ppu_get_info()->FIFO_info.bg_data[0]+=128;
                
                }
                gb_ppu_pipeline_load_window_tile();
            }
            if(gb_lcd_is_obj_enabled() && gb_ppu_get_info()->sprites_in_line)
            {
                gb_ppu_pipline_load_sprite_tile();
            }

            gb_ppu_get_info()->FIFO_info.fetch_state=GET_TILE_DATA_LOW;
            gb_ppu_get_info()->FIFO_info.fetched_x+=8;

            
        }break;
        case GET_TILE_DATA_LOW:
        {
            gb_ppu_get_info()->FIFO_info.bg_data[1]=gb_bus_read(gb_lcd_get_window_data_area()+(gb_ppu_get_info()->FIFO_info.bg_data[0]*16)+gb_ppu_get_info()->FIFO_info.tile_y);
            gb_ppu_pipeline_load_sprite_data(DATA_LOW);
            gb_ppu_get_info()->FIFO_info.fetch_state=GET_TILE_DATA_HIGH;

            
        }break;
        case GET_TILE_DATA_HIGH:
        {
            gb_ppu_get_info()->FIFO_info.bg_data[2]=gb_bus_read(gb_lcd_get_window_data_area()+(gb_ppu_get_info()->FIFO_info.bg_data[0]*16)+gb_ppu_get_info()->FIFO_info.tile_y+1);
            gb_ppu_pipeline_load_sprite_data(DATA_HIGH);
            gb_ppu_get_info()->FIFO_info.fetch_state=SLEEP;
            
        }break;
        case SLEEP:
        {
            gb_ppu_get_info()->FIFO_info.fetch_state=PUSH;
            
        }break;
        case PUSH:
        {
            if(gb_ppu_get_info()->FIFO_info.FIFO_queue.lenght>8)
            {
                //Cant push to queue
            }
            else
            {
                int x=gb_ppu_get_info()->FIFO_info.fetched_x-(8-(gb_get_lcd_info()->scroll_x%8));
                for(int i=0;i<8;i++)
                {
                    uint8_t lower_byte=gb_ppu_get_info()->FIFO_info.bg_data[1];
                    uint8_t upper_byte=gb_ppu_get_info()->FIFO_info.bg_data[2];
                    int current_bit=7-i;
                    uint8_t hi=!!(upper_byte&(1<<current_bit))<<1;
                    uint8_t lo=!!(lower_byte&(1<<current_bit));
                    uint32_t color = gb_get_lcd_info()->bg_colors[hi|lo];

                    if(!gb_lcd_is_bg_enabled())
                    {
                        color=gb_get_lcd_info()->bg_colors[0];
                    }

                    if(gb_lcd_is_obj_enabled())
                    {
                        color=gb_ppu_fetch_sprite_pixels(current_bit,color, hi|lo);
                    }

                    if(x>=0)
                    {
                        gb_FIFO_push(color);
                        gb_ppu_get_info()->FIFO_info.fifo_x++;
                    }
                }
                gb_ppu_get_info()->FIFO_info.fetch_state=GET_TILE;
            }
            
        }
    }
}

void gb_pipeline_push_pixel()
{
    //We can only pop the queue when atleast 8 pixels are in it
    if(gb_ppu_get_info()->FIFO_info.FIFO_queue.lenght>8)
    {
        uint32_t color_data = gb_FIFO_pop();
        if(gb_ppu_get_info()->FIFO_info.line_x_pos>=(gb_get_lcd_info()->scroll_x%8))
        {
            int video_buffer_address=gb_ppu_get_info()->FIFO_info.pushed_x+(gb_get_lcd_info()->lcd_y*X_RESOLUTION);
            gb_ppu_get_info()->video_buffer[video_buffer_address]=color_data;
            gb_ppu_get_info()->FIFO_info.pushed_x++;
        }
        gb_ppu_get_info()->FIFO_info.line_x_pos++;
    }
}

void gb_ppu_pipeline_process()
{
    gb_ppu_get_info()->FIFO_info.map_y=(gb_get_lcd_info()->lcd_y+gb_get_lcd_info()->scroll_y);
    gb_ppu_get_info()->FIFO_info.map_x=(gb_ppu_get_info()->FIFO_info.fetched_x+gb_get_lcd_info()->scroll_x);
    gb_ppu_get_info()->FIFO_info.tile_y=((gb_get_lcd_info()->lcd_y+gb_get_lcd_info()->scroll_y)%8)*2;
    if(gb_ppu_get_info()->line_ticks%2==0)
    {
        gb_ppu_pipeline_fetch();
    }
    gb_pipeline_push_pixel();
}

void gb_ppu_pipeline_FIFO_reset()
{
    while(gb_ppu_get_info()->FIFO_info.FIFO_queue.lenght>0)
    {
        gb_FIFO_pop();
    }
    gb_ppu_get_info()->FIFO_info.FIFO_queue.first_element=0;
}