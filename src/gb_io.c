#include "gb_io.h"
#include "gb_timer.h"
#include "gb_cpu.h"
static char data_on_serial[2];
uint8_t gb_io_read(uint16_t address)
{
    switch(address)
    {
        case 0xFF01:
            return data_on_serial[0];
        case 0xFF02:
            return data_on_serial[1];
        case 0xFF04 ... 0xFF07:
            return gb_timer_read(address);
        case 0xFF0F:
            return gb_cpu_get_interrupt_flags();
        
    }
    return 0;
}
void gb_io_write(uint8_t value,uint16_t address)
{
    switch(address)
    {
        case 0xFF01:
            data_on_serial[0]=value;
            return;
        case 0xFF02:
            data_on_serial[1]=value;
            return;
        case 0xFF04 ... 0xFF07:
            gb_timer_write(value,address);
            return;
        case 0xFF0F:
            gb_cpu_set_interrupt_flags(value);
            return;
    }
    printf("UNSUPPORTED bus_write(%04X)\n", address);
}