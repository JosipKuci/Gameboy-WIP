#ifndef GB_REGISTERS_H
#define GB_REGISTERS_H
#include<stdint.h>
struct gb_registers
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t f;
    uint8_t h;
    uint8_t l;

    uint16_t program_counter;
    uint16_t stack_pointer;
};
#endif