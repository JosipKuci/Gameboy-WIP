#ifndef GB_INTERRUPTS_H
#define GB_INTERRUPTS_H
#include "gb_cpu.h"
#include "gb_stack.h"
enum gb_interrupt_type{
    IT_VBLANK = 1,
    IT_LCD_STAT = 2,
    IT_TIMER = 4,
    IT_SERIAL = 8,
    IT_JOYPAD = 16
};
void gb_cpu_handle_interrupts(struct gb_cpu_info* cpu_info);
void gb_cpu_request_interrupt(enum gb_interrupt_type interrupt);
#endif