#ifndef GB_MEMORY_H
#define GB_MEMORY_H
#include<stdint.h>
struct gb_memory_info{
    uint8_t wram[0x2000];
    uint8_t hram[0x80];
};
uint8_t gb_wram_read(uint16_t address);
void gb_wram_write(uint8_t value, uint16_t address);

uint8_t gb_hram_read(uint16_t address);
void gb_hram_write(uint8_t value, uint16_t address);
#endif