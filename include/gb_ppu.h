#ifndef GB_PPU_H
#define GB_PPU_H
#include<stdint.h>
#define LINES_PER_FRAME 154
#define DOTS_PER_LINE 456
#define Y_RESOLUTION 144
#define X_RESOLUTION 160


//Stores the data of  the sprite in memory
//Explained in detail at https://gbdev.io/pandocs/OAM.html
struct gb_oam_data{ //"Object attribute memory"
    uint8_t y;
    uint8_t x;
    uint8_t tile_index;
    uint8_t attributes; //Priority-Y_flip-X_flip-DMG_Palette-Bank
};

struct gb_oam_in_line{
    struct gb_oam_data object;
    struct gb_oam_in_line* next_object;
};


enum FIFO_pixel_fetch{
    GET_TILE,
    GET_TILE_DATA_LOW,
    GET_TILE_DATA_HIGH,
    SLEEP,
    PUSH,
};

struct gb_FIFO_element{
    struct gb_FIFO_element* next_element;
    uint32_t color_value;
};

struct gb_FIFO_queue{
    struct gb_FIFO_element* first_element;
    struct gb_FIFO_element* last_element;
    uint32_t lenght;
};

struct gb_FIFO_queue_info{
    enum FIFO_pixel_fetch fetch_state;
    struct gb_FIFO_queue FIFO_queue;
    uint8_t line_x_pos;
    uint8_t pushed_x;
    uint8_t fetched_x;
    uint8_t bg_data[3];
    uint8_t oam_data[6];
    uint8_t map_y;
    uint8_t map_x;
    uint8_t tile_y;
    uint8_t fifo_x;
};




struct gb_ppu_info{
    struct gb_oam_data oam_ram[40];
    uint8_t vram[8192];
    struct gb_FIFO_queue_info FIFO_info;

    uint32_t current_frame;
    uint32_t line_ticks;
    uint32_t* video_buffer;
    uint8_t window_line;

    struct gb_oam_in_line* sprites_in_line;
    uint8_t num_of_sprites; // There is at most 10 sprites per line (DMG-acid2)
    struct gb_oam_in_line list_of_sprite_line_entries[10]; // Used to store memory spaces of specific sprites

    //For FIFO sprite fetching
    uint8_t fetched_entry_count; //Max of 3 per pixel fetch
    struct gb_oam_data fetched_entries[3];
};




struct gb_ppu_info* gb_ppu_get_info();

void gb_ppu_oam_write(uint8_t value, uint16_t address);
uint8_t gb_ppu_oam_read(uint16_t address);

void gb_ppu_vram_write(uint8_t value, uint16_t address);
uint8_t gb_ppu_vram_read(uint16_t address);

void gb_ppu_initialize();
void gb_ppu_tick();

uint8_t gb_ppu_is_window_visible();
#endif