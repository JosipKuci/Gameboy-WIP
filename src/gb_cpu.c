#include "gb_cpu.h"
#include "gb_data_bus.h"
#include "gb_registers.h"
struct gb_cpu_info cpu_info = {{0}};

void gb_cpu_init()
{
    cpu_info.registers.program_counter=0x100;
}

static void gb_fetch_instruction()
{
    cpu_info.current_opcode=gb_bus_read(cpu_info.registers.program_counter++);
    cpu_info.current_instruction=instruction_by_opcode(cpu_info.current_opcode);
}
uint16_t gb_cpu_read_register(enum register_type regi)
{
    switch(regi)
    {
        case RT_A: return cpu_info.registers.a;
        case RT_F: return cpu_info.registers.f;
        case RT_B: return cpu_info.registers.b;
        case RT_C: return cpu_info.registers.c;
        case RT_D: return cpu_info.registers.d;
        case RT_E: return cpu_info.registers.e;
        case RT_H: return cpu_info.registers.h;
        case RT_L: return cpu_info.registers.l;

        case RT_AF:
        {
            uint16_t AF=cpu_info.registers.a;
            AF=AF<<8|cpu_info.registers.f;
            return AF;
        }
        case RT_BC:
        {
            uint16_t BC=cpu_info.registers.b;
            BC=BC<<8|cpu_info.registers.c;
            return BC;
        }
        case RT_DE:
        {
            uint16_t DE=cpu_info.registers.d;
            DE=DE<<8|cpu_info.registers.e;
            return DE;
        }
        case RT_HL: 
        {
            uint16_t HL=cpu_info.registers.h;
            HL=HL<<8|cpu_info.registers.l;
            return HL;
        }
        case RT_PC: return cpu_info.registers.program_counter;
        case RT_SP: return cpu_info.registers.stack_pointer;
    }
}
static void gb_fetch_data()
{
    cpu_info.memory_destination=0;
    cpu_info.is_destination_to_memory=false;
    switch(cpu_info.current_instruction->mode)
    {
        case(AM_IMP):
            break;
        case(AM_R):
            cpu_info.fetch_data=gb_cpu_read_register(cpu_info.current_instruction->register_1);
            break;
        case(AM_R_D8):
            cpu_info.fetch_data=gb_bus_read(cpu_info.registers.program_counter);
            gb_emulator_cycle(1);
            cpu_info.registers.program_counter++;
            break;
        case(AM_D16):
        {
            uint8_t low = gb_bus_read(cpu_info.registers.program_counter);
            gb_emulator_cycle(1);
            uint8_t high = gb_bus_read(cpu_info.registers.program_counter++);
            cpu_info.fetch_data=high;
            cpu_info.fetch_data=(cpu_info.fetch_data<<8)|low;
            cpu_info.registers.program_counter+=2;
            break;
        }

    }
}
static void gb_execute()
{
    //not implimented
}

bool gb_cpu_step()
{
    if(!cpu_info.is_halted)
    {
        gb_fetch_instruction();
        gb_fetch_data();
        gb_execute();
    }
    return gb_emulator_cycle-1;
}