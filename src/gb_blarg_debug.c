#include "gb_blarg_debug.h"
#include "gb_data_bus.h"
static char message[1024]={0};
static int message_size=0;
//This is all done purely for using the blarg debug tests, which send data through the serial port
void debug_update()
{
    if(gb_bus_read(0xFF02)==0x81) //If transfer is enabled and is using internal clock
    {
        char c = gb_bus_read(0xFF01);
        message[message_size++]=c;
        gb_bus_write(0, 0xFF02);//Reset the serial
    }
}
void debug_print()
{
    if(message[0])
    {
        printf("DBG: %s\n",message);
    }
}