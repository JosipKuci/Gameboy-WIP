#include "gb_interrupts.h"
void push_current_routine(struct gb_cpu_info* cpu_info, uint16_t address)
{
    gb_stack_push_16(cpu_info->registers.program_counter);
    cpu_info->registers.program_counter=address;
}
bool check_interrupt(struct gb_cpu_info* cpu_info, uint16_t address, enum gb_interrupt_type interrupt)
{
    if(cpu_info->interrupt_flags & interrupt && cpu_info->registers.IE & interrupt)
    {
        push_current_routine(cpu_info,address); //Video interrupt, executes 59.7 times a second
        cpu_info->interrupt_flags&=~interrupt; //Disable vblank bit
        cpu_info->is_halted=false;
        cpu_info->is_master_interrupt_enabled=false;
        return true;
    }
    return false;
}
void gb_cpu_handle_interrupts(struct gb_cpu_info* cpu_info)
{
    if(check_interrupt(cpu_info,0x40,IT_VBLANK))
        return;
    if(check_interrupt(cpu_info,0x48,IT_LCD_STAT))
        return;
    if(check_interrupt(cpu_info,0x50,IT_TIMER))
        return;
    if(check_interrupt(cpu_info,0x58,IT_SERIAL))
        return;
    if(check_interrupt(cpu_info,0x60,IT_JOYPAD))
        return;
}
