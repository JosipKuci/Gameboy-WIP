#ifndef GB_PPU_PIPELINE_H
#define GB_PPU_PIPELINE_H
#include <stdint.h>
#include <string.h> //for memset
void gb_FIFO_push(uint32_t value);
uint32_t gb_FIFO_pop();
void gb_ppu_pipeline_fetch();
void gb_pipeline_push_pixel();
void gb_ppu_pipeline_process();
void gb_ppu_pipeline_FIFO_reset();
#endif