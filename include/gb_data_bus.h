#ifndef GB_DATA_BUS_H
#define GB_DATA_BUS_H
 
#include<stdint.h>
uint8_t gb_bus_read(uint16_t address);
uint16_t gb_bus_read_16(uint16_t address);
void gb_bus_write(uint8_t value, uint16_t address);
void gb_bus_write_16(uint16_t value, uint16_t address);
#endif