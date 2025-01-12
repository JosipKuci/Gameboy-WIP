#ifndef GB_TIMER_H
#define GB_TIMER_H
#include <stdint.h>
struct gb_timer_info{
    uint16_t DIV; //Divider register
    uint8_t TIMA; //Timer counter
    uint8_t TMA; //Timer modulo
    uint8_t TAC; //Timer control
};
void gb_timer_init();
void gb_timer_tick();

uint8_t gb_timer_read(uint16_t address);
void gb_timer_write(uint8_t value, uint16_t address);

struct gb_timer_info* gb_timer_get_info();
#endif