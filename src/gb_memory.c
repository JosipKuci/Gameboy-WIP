#include "gb_memory.h"
static struct gb_memory_info memory_info;
//Converts the address so that it corresponds to the wram range (0xC000 - 0xE000)
static uint16_t gb_convert_address_to_wram_address(uint16_t address)
{
    return address-0xC000;
}

//Converts the address so that it corresponds to the hram range (0xFF80 - 0xFFFF)
static uint16_t gb_convert_address_to_hram_address(uint16_t address)
{
    return address-0xFF80;
}

uint8_t gb_wram_read(uint16_t address)
{
    uint16_t wram_address=gb_convert_address_to_wram_address(address);
    return memory_info.wram[wram_address];
}
void gb_wram_write(uint8_t value, uint16_t address)
{
    uint16_t wram_address=gb_convert_address_to_wram_address(address);
    memory_info.wram[wram_address]=value;
}

uint8_t gb_hram_read(uint16_t address)
{
    uint16_t hram_address=gb_convert_address_to_hram_address(address);
    return memory_info.hram[hram_address];
}
void gb_hram_write(uint8_t value, uint16_t address)
{
    uint16_t hram_address=gb_convert_address_to_hram_address(address);
    memory_info.hram[hram_address]=value;
}