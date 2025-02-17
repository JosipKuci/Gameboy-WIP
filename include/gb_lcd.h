#ifndef GB_LCD_H
#define GB_LCD_H
#include<stdint.h>
struct gb_lcd_info{
    /*
    LCD & PPU enable: 0 = Off; 1 = On
    Window tile map area: 0 = 9800–9BFF; 1 = 9C00–9FFF
    Window enable: 0 = Off; 1 = On
    BG & Window tile data area: 0 = 8800–97FF; 1 = 8000–8FFF
    BG tile map area: 0 = 9800–9BFF; 1 = 9C00–9FFF
    OBJ size: 0 = 8×8; 1 = 8×16
    OBJ enable: 0 = Off; 1 = On
    BG & Window enable / priority [Different meaning in CGB Mode]: 0 = Off; 1 = On
    */
    uint8_t lcd_control; //0xFF40
    /*
    LYC int select (Read/Write): If set, selects the LYC == LY condition for the STAT interrupt.
    Mode 2 int select (Read/Write): If set, selects the Mode 2 condition for the STAT interrupt.
    Mode 1 int select (Read/Write): If set, selects the Mode 1 condition for the STAT interrupt.
    Mode 0 int select (Read/Write): If set, selects the Mode 0 condition for the STAT interrupt.
    LYC == LY (Read-only): Set when LY contains the same value as LYC; it is constantly updated.
    PPU mode (Read-only): Indicates the PPU’s current status.
    */
    uint8_t lcd_status;//0xFF41
    uint8_t scroll_y;//0xFF42
    uint8_t scroll_x;//0xFF43
    uint8_t lcd_y;//0xFF44
    uint8_t ly_compare;//0xFF45
    uint8_t dma;//0xFF46;
    uint8_t bg_palette;//0xFF47;
    uint8_t obj_palette1;//0xFF48;
    uint8_t obj_palette2;//0xFF49;
    uint8_t window_y;
    uint8_t window_x;

    uint32_t bg_colors[4];
    uint32_t sprite1_colors[4];
    uint32_t sprite2_colors[4];
};
struct gb_lcd_info lcd_info;
enum ppu_modes{
    HBLANK,
    VBLANK,
    OAMSCAN,
    DRAWING,
};

enum stat_interrupt{
        MODE0_SELECT =   0b1000,
        MODE1_SELECT =  0b10000,
        MODE2_SELECT = 0b100000,
        LYC_SELECT =  0b1000000
};


struct gb_lcd_info* gb_get_lcd_info();

void gb_lcd_initialize();

uint8_t gb_lcd_read(uint16_t address);

void gb_lcd_write(uint8_t value, uint16_t address);

//Functions to fetch specific bits from the lcd_control register
uint8_t gb_lcd_is_bg_enabled();
uint8_t gb_lcd_is_obj_enabled();
uint8_t gb_lcd_get_obj_size();
uint16_t gb_lcd_get_tile_map_area();
uint16_t gb_lcd_get_window_data_area();
uint16_t gb_lcd_get_window_map_area();
uint8_t gb_lcd_is_window_enabled();
uint8_t gb_lcd_is_lcd_enabled();

//Functions to fetch specific bits from the lcd_status register
enum ppu_modes gb_lcd_get_ppu_mode();
void gb_lcd_set_ppu_mode(enum ppu_modes mode);
uint8_t gb_lcd_get_lyc_ly_comparison();
void gb_lcd_set_lyc_ly_comparison(uint8_t bit);
uint8_t gb_lcd_is_interrupt_selected(enum stat_interrupt interrupt);
#endif