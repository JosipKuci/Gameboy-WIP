#include "gb_stack.h"
#include "gb_cpu.h"
#include "gb_emulator.h"
#include "gb_data_bus.h"
#include<stdio.h>
//Stack pointer at start set to the end of wram (0xDFFF)
void gb_stack_push(uint8_t data)
{
    gb_get_all_registers()->stack_pointer--;
    gb_bus_write(data, gb_get_all_registers()->stack_pointer);
}
uint8_t gb_stack_pop()
{
    
    return gb_bus_read(gb_get_all_registers()->stack_pointer++);

}

void gb_stack_push_16(uint16_t data)
{
    gb_stack_push((data >> 8) & 0xFF);
    gb_stack_push(data & 0xFF);
}
uint16_t gb_stack_pop_16()
{
    uint16_t low = gb_stack_pop();
    uint16_t high = gb_stack_pop();
    return (high<<8)|low;
}