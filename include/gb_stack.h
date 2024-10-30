#ifndef GB_STACK_H
#define GB_STACK_H
#include <stdint.h>
void gb_stack_push(uint8_t data);
uint8_t gb_stack_pop();

void gb_stack_push_16(uint16_t data);
uint16_t gb_stack_pop_16();
#endif