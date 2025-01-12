#ifndef GB_CPU_H
#define GB_CPU_H
#include <stdbool.h>
#include <stdlib.h>
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
    bool is_master_interrupt_enabled;
    bool is_enabling_interrupt;
    uint8_t interrupt_flags;

    struct gb_instruction *current_instruction;
};
//Checks if the 7th bit of the flags register (the Zero flag) is set
int get_cpu_z_flag(struct gb_cpu_info* cpu_info);
//Checks if the 4th bit of the flags register (the Carry flag) is set
int get_cpu_c_flag(struct gb_cpu_info* cpu_info);

void gb_cpu_set_flags(struct gb_cpu_info* cpu_info, uint8_t z, uint8_t n, uint8_t h, uint8_t c);
void gb_cpu_set_register(enum register_type, uint16_t value);

struct gb_registers* gb_get_all_registers();

void gb_cpu_init();
bool gb_cpu_step();
uint16_t gb_cpu_read_register(enum register_type register);

typedef void (*IN_PROC)(struct gb_cpu_info*);

IN_PROC instruction_get_processor(enum instruction_type type);

void gb_cpu_set_register_cb(enum register_type regi, uint8_t value);
uint8_t gb_cpu_read_register_cb(enum register_type regi);

uint8_t gb_cpu_get_interrupt_flags();
void gb_cpu_set_interrupt_flags(uint8_t value);
#endif