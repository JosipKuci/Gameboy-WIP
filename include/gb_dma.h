#ifndef GB_DMA_H
#define GB_DMA_H
#include<stdint.h>
#include<stdbool.h>
struct gb_dma_info{
    bool is_active;
    uint8_t current_byte;
    uint8_t delay;
    uint8_t value;
};

void gb_dma_initialize(uint8_t start_byte);
void gb_dma_tick();
bool gb_dma_is_transferring();
#endif