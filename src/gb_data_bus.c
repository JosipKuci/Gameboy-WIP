#include "gb_data_bus.h"
#include "gb_cartridge.h"
uint8_t gb_bus_read(uint16_t address)
{
    if(address < 0x8000)
    {
        return gb_cartridge_read(address);
    }
}
void gb_bus_write(uint8_t value, uint16_t address)
{
    if(address < 0x8000)
    {
        return gb_cartridge_write(value, address);
    }
}