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

struct gb_ppu_info{
    struct gb_oam_data oam_ram[40];
    uint8_t vram[8192];

    uint32_t current_frame;
    uint32_t line_ticks;
    uint32_t* video_buffer;
};

struct gb_ppu_info* gb_ppu_get_info();

void gb_ppu_oam_write(uint8_t value, uint16_t address);
uint8_t gb_ppu_oam_read(uint16_t address);

void gb_ppu_vram_write(uint8_t value, uint16_t address);
uint8_t gb_ppu_vram_read(uint16_t address);

void gb_ppu_initialize();
void gb_ppu_tick();
#endif