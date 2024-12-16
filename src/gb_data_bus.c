#include "gb_data_bus.h"
#include "gb_memory.h"
#include "gb_cpu.h"
#include "gb_cartridge.h"
#include <stdio.h>
uint8_t gb_bus_read(uint16_t address)
{
    if(address < 0x8000) //Cartridge memory
    {
        return gb_cartridge_read(address);
    }
    else if(address < 0xA000) //VRAM
    {
        //Not implemented
        fprintf(stderr, "NOT YET IMPLEMENTED\n"); exit(-5);
    }
    else if(address < 0xC000) //Cartridge RAM
    {
        return gb_cartridge_read(address);
    }
    else if(address < 0xE000) //Working RAM
    {
        return gb_wram_read(address);
    }
    else if(address < 0xFE00) //Echo RAM
    {
        //Reserved as it is prohibited to access
        return 0;
    }
    else if(address < 0xFEA0) //Object attribute memory (OAM)
    {
        //Not implemented
        fprintf(stderr, "NOT YET IMPLEMENTED\n"); exit(-5);
    }
    else if(address < 0xFF00) //Not usable
    {
        //Reserved as it is prohibited to access
        return 0;
    }
    else if(address < 0xFF80) //IO registers
    {
        //Not implemented
        fprintf(stderr, "NOT YET IMPLEMENTED\n"); exit(-5);
    }
    else if(address < 0xFFFF)//HRAM
    {
        
        return gb_hram_read(address);
    }
    else if(address == 0xFFFF)//interrupt enable register
    {
        return gb_cpu_read_register(RT_IE);
    }
    return;
}
uint16_t gb_bus_read_16(uint16_t address)
{
    uint16_t low=gb_bus_read(address);
    uint16_t high=gb_bus_read(address+1);
    return (high<<8)|low;
}
void gb_bus_write(uint8_t value, uint16_t address)
{
    if(address < 0x8000) //Cartridge memory
    {
        gb_cartridge_write(value, address);
    }
    else if(address < 0xA000) //VRAM
    {
        //Not implemented
    }
    else if(address < 0xC000) //Cartridge RAM
    {
        gb_cartridge_write(value, address);
    }
    else if(address < 0xE000) //Working RAM
    {   
        gb_wram_write(value, address);
    }
    else if(address < 0xFE00) //Echo RAM
    {
        //Reserved as it is prohibited to access
    }
    else if(address < 0xFEA0) //Object attribute memory (OAM)
    {
        //Not implemented
    }
    else if(address < 0xFF00) //Not usable
    {
        //Reserved as it is prohibited to access
    }
    else if(address < 0xFF80) //IO registers
    {
        //Not implemented
    }
    else if(address < 0xFFFF)//HRAM
    {
        gb_hram_write(value,address);
    }
    else if(address == 0xFFFF)//interrupt enable register
    {
        gb_cpu_set_register(RT_IE, value);
    }
    return;
}
void gb_bus_write_16(uint16_t value, uint16_t address)
{
    gb_bus_write(address + 1, (value >> 8) & 0xFF);
    gb_bus_write(address, value & 0xFF);
    return;
}