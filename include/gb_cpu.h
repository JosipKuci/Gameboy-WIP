#ifndef GB_CPU_H
#define GB_CPU_H
#endif
#include <stdbool.h>
#include "gb_registers.h"
#include "gb_instructions.h"
#include "gb_emulator.h"
struct gb_cpu_info
{
    struct gb_registers registers;
    uint16_t fetch_data;
    uint16_t memory_destination;
    bool is_destination_to_memory;
    uint8_t current_opcode;    
    
    bool is_halted;
    bool is_stepping;

    struct gb_instruction *current_instruction;
};

void gb_cpu_init();
bool gb_cpu_step();
uint16_t gb_cpu_read_register(enum register_type register);
