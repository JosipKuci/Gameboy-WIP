#ifndef GB_IO_H
#define GB_IO_H
#include <stdint.h>
uint8_t gb_io_read(uint16_t address);
void gb_io_write(uint8_t value,uint16_t address);
#endif
