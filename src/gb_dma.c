#include "gb_dma.h"
#include "gb_data_bus.h"
#include "gb_ppu.h"
static struct gb_dma_info dma_info;
void gb_dma_initialize(uint8_t start_byte)
{
    dma_info.is_active=true;
    dma_info.current_byte=0;
    dma_info.delay=2;//2 cycles before the dma transfer starts
    dma_info.value=start_byte;
}
void gb_dma_tick()
{
    if(!dma_info.is_active)
    {   
        return;
    }
    else
    {
        if(dma_info.delay > 0)
        {
            dma_info.delay--;
            return;
        }
        else
        {
            gb_ppu_oam_write(gb_bus_read((dma_info.value*0x100)+dma_info.current_byte),dma_info.current_byte);
            dma_info.current_byte++;
            dma_info.is_active=dma_info.current_byte<0xA0;
        }
    }
}
bool gb_dma_is_transferring()
{
    return dma_info.is_active;
}