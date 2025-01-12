#include "gb_timer.h"
#include "gb_interrupts.h"
static struct gb_timer_info timer_info={0};
void gb_timer_init()
{
    timer_info.DIV=0xAC00;

}

struct gb_timer_info* gb_timer_get_info()
{
    return &timer_info;
}
void gb_timer_tick()
{
    uint16_t previous_timer_DIV=timer_info.DIV;
    timer_info.DIV++;
    bool does_timer_update=false;

    switch(timer_info.TAC&(0b11)) //Get lowest 2 bits to see how TIMA is incremented 
    {
        case 0b00:
        {
            does_timer_update=(previous_timer_DIV&(1<<9))&&(!(timer_info.DIV&(1<<9))); //divided by 1024
            break;
        }
        case 0b01:
        {
            does_timer_update=(previous_timer_DIV&(1<<3))&&(!(timer_info.DIV&(1<<3))); //divided by 16
            break;
        }
        case 0b10:
        {
            does_timer_update=(previous_timer_DIV&(1<<5))&&(!(timer_info.DIV&(1<<5))); //divided by 64
            break;
        }
        case 0b11:
        {
            does_timer_update=(previous_timer_DIV&(1<<7))&&(!(timer_info.DIV&(1<<7))); //divided by 256
            break;
        }
    }
    if(does_timer_update && timer_info.TAC&(1<<2))//See if timer updates and if 3rd bit in TAC is set (Timer enable)
    {
        timer_info.TIMA++;
        if(timer_info.TIMA==0xFF)
        {
            timer_info.TIMA=timer_info.TMA;
            gb_cpu_request_interrupt(IT_TIMER);
        }
    }
}
uint8_t gb_timer_read(uint16_t address)
{
    switch(address)
    {
        case 0xFF04:
        {
            return timer_info.DIV>>8;//Returns only top byte
        }
        case 0xFF05:
        {
            return timer_info.TIMA;

        }
        case 0xFF06:
        {
            return timer_info.TMA;

        }
        case 0xFF07:
        {
            return timer_info.TAC;
        }
    }
}
void gb_timer_write(uint8_t value, uint16_t address)
{
    switch(address)
    {
        case 0xFF04:
        {
            timer_info.DIV=0; //Writing naything to this register sets it to 0
            break;
        }
        case 0xFF05:
        {
            timer_info.TIMA=value;
            break;
        }
        case 0xFF06:
        {
            timer_info.TMA=value;
            break;
        }
        case 0xFF07:
        {
            timer_info.TAC=value;
            break;
        }
    }
}
