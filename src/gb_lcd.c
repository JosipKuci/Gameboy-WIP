#include "gb_lcd.h"
#include "gb_dma.h"

static unsigned long color_palette[4]={0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

struct gb_lcd_info* gb_get_lcd_info()
{
    return &lcd_info;
}

uint8_t gb_lcd_is_bg_enabled()
{
    if(gb_get_lcd_info()->lcd_control&0x1)
    {
        return 1;
    }
    return 0;
}

uint8_t gb_lcd_is_obj_enabled()
{
    if(gb_get_lcd_info()->lcd_control&0x2)
    {
        return 1;
    }
    return 0;
}

uint8_t gb_lcd_get_obj_size()
{
    if(gb_get_lcd_info()->lcd_control&0x4)
    {
        return 16;
    }
    return 8;
}

uint16_t gb_lcd_get_tile_map_area()
{
    if(gb_get_lcd_info()->lcd_control&0x8)
    {
        return 0x9C00;
    }
    return 0x9800;
}

uint16_t gb_lcd_get_window_data_area()
{
    if(gb_get_lcd_info()->lcd_control&0x10)
    {
        return 0x8000;
    }
    return 0x8800;
}

uint16_t gb_lcd_get_window_map_area()
{
    if(gb_get_lcd_info()->lcd_control&0x40)
    {
        return 0x9C00;
    }
    return 0x9800;
}

uint8_t gb_lcd_is_window_enabled()
{
    if(gb_get_lcd_info()->lcd_control&0x20)
    {
        return 1;
    }
    return 0;
}

uint8_t gb_lcd_is_lcd_enabled()
{
    if(gb_get_lcd_info()->lcd_control&0x80)
    {
        return 1;
    }
    return 0;
}


enum ppu_modes gb_lcd_get_ppu_mode()
{
    uint8_t current_mode=gb_get_lcd_info()->lcd_status&0b11;
    switch(current_mode){
        case 0:
            return HBLANK;
        case 1:
            return VBLANK;
        case 2:
            return OAMSCAN;
        case 3:
            return DRAWING;
    }
}

void gb_lcd_set_ppu_mode(enum ppu_modes mode)
{
    gb_get_lcd_info()->lcd_status&=~(0b11);// resetting the ppu mode
    gb_get_lcd_info()->lcd_status|=mode;
}

uint8_t gb_lcd_get_lyc_ly_comparison()
{
    if(gb_get_lcd_info()->lcd_status&0x4)
    {
        return 1;
    }
    return 0;
}
void gb_lcd_set_lyc_ly_comparison(uint8_t bit)
{
    gb_get_lcd_info()->lcd_status&=~(0b100);
    if(bit)
    {
        gb_get_lcd_info()->lcd_status|=0b100;
    }
    return;
}


uint8_t gb_lcd_is_interrupt_selected(enum stat_interrupt interrupt)
{
    return gb_get_lcd_info()->lcd_status&interrupt;
}




void gb_lcd_initialize()
{
    lcd_info.lcd_control=0x91;
    lcd_info.scroll_x=0;
    lcd_info.scroll_y=0;
    lcd_info.window_x=0;
    lcd_info.window_y=0;
    lcd_info.lcd_y=0;
    lcd_info.ly_compare=0;
    lcd_info.bg_palette=0xFC;
    lcd_info.obj_palette1=0xFF;
    lcd_info.obj_palette2=0xFF;
    for(int i=0;i<4;i++)
    {
        lcd_info.bg_colors[i]=color_palette[i];
        lcd_info.sprite1_colors[i]=color_palette[i];
        lcd_info.sprite2_colors[i]=color_palette[i];
    }
}

uint8_t gb_lcd_read(uint16_t address)
{
    address-=0xFF40;//Remove the offset
    uint8_t* pointer=(uint8_t *)&lcd_info;
    return pointer[address];
}

void gb_lcd_update_palette(uint8_t value, uint8_t palette)
{
    uint32_t *colors=lcd_info.bg_colors;
    switch(palette)
    {
        case 1:
            colors=lcd_info.sprite1_colors;
            value&=0b11111100;
            break;
        case 2:
            colors=lcd_info.sprite2_colors;
            value&=0b11111100;
            break;
    }

    colors[0]=color_palette[value&0b11];
    colors[1]=color_palette[(value>>2)&0b11];
    colors[2]=color_palette[(value>>4)&0b11];
    colors[3]=color_palette[(value>>6)&0b11];
}

void gb_lcd_write(uint8_t value, uint16_t address)
{
    uint8_t offset=address-0xFF40;//Remove the offset
    uint8_t* pointer=(uint8_t *)&lcd_info;
    pointer[offset]=value;
    if( offset == 6) //0xFF46
    {
        gb_dma_initialize(value);
    }
    else if(address == 0xFF47)
    {
        gb_lcd_update_palette(value,0);
    }
    else if(address == 0xFF48)
    {
        gb_lcd_update_palette(value,1);
    }
    else if(address == 0xFF49)
    {
        gb_lcd_update_palette(value,2);
    }
    return;
}