#include "gb_cpu.h"
#include <stdio.h>
#include "gb_data_bus.h"
#include "gb_registers.h"
#include "gb_timer.h"
#include "gb_interrupts.h"
#include "gb_blarg_debug.h"
#include "SDL2/SDL.h"
struct gb_cpu_info cpu_info = {0};

void gb_cpu_init()
{
    cpu_info.registers.program_counter=0x100;
    cpu_info.registers.stack_pointer=0xFFFE;
    cpu_info.registers.a=0x01;
    cpu_info.registers.f=0xB0;
    cpu_info.registers.b=0x00;
    cpu_info.registers.c=0x13;
    cpu_info.registers.d=0x00;
    cpu_info.registers.e=0xD8;
    cpu_info.registers.h=0x01;
    cpu_info.registers.l=0x4D;
    cpu_info.registers.IE=0;
    cpu_info.interrupt_flags=0;
    cpu_info.is_master_interrupt_enabled=false;
    cpu_info.is_enabling_interrupt=false;

    gb_timer_get_info()->DIV=0xABCC;
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
        case RT_IE: return cpu_info.registers.IE;

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
        default: return NULL;
    }
}
void gb_cpu_set_register(enum register_type register_type, uint16_t value)
{
    switch(register_type)
    {
        case RT_A:
        {
            cpu_info.registers.a=(uint8_t)(value & 0xff);
            return;
        }
        case RT_F:
        {
            cpu_info.registers.f=(uint8_t)(value & 0xff);
            return;
        }
        case RT_B:
        {
            cpu_info.registers.b=(uint8_t)(value & 0xff);
            return;
        }
        case RT_C:
        {
            cpu_info.registers.c=(uint8_t)(value & 0xff);
            return;
        }
        case RT_D:
        {
            cpu_info.registers.d=(uint8_t)(value & 0xff);
            return;
        }
        case RT_E:
        {
            cpu_info.registers.e=(uint8_t)(value & 0xff);
            return;
        } 
        case RT_H:
        {
            cpu_info.registers.h=(uint8_t)(value & 0xff);
            return;
        }
        case RT_L:
        {
            cpu_info.registers.l=(uint8_t)(value & 0xff);
            return;
        }
        case RT_IE:
        {
            cpu_info.registers.IE=(uint8_t)(value & 0xff);
            return;
        }
        case RT_AF:
        {
            cpu_info.registers.a=(uint8_t)((value & 0xff00)>>8);
            cpu_info.registers.f=(uint8_t)(value & 0xff);
            return;
        }
        case RT_BC:
        {
            cpu_info.registers.b=(uint8_t)((value & 0xff00)>>8);
            cpu_info.registers.c=(uint8_t)(value & 0xff);
            return;
        }
        case RT_DE:
        {
            cpu_info.registers.d=(uint8_t)((value & 0xff00)>>8);
            cpu_info.registers.e=(uint8_t)(value & 0xff);
            return;
        }
        case RT_HL:
        {
            cpu_info.registers.h=(uint8_t)((value & 0xff00)>>8);
            cpu_info.registers.l=(uint8_t)(value & 0xff);
            return;
        }
        case RT_SP:
        {
            cpu_info.registers.stack_pointer=value;
        }
    }
}
static void gb_fetch_data()
{
    cpu_info.memory_destination=0;
    cpu_info.is_destination_to_memory=false;
    if (cpu_info.current_instruction == NULL) {
        return;
    }
    switch(cpu_info.current_instruction->mode)
    {
        case(AM_IMP):
            return;
        case(AM_R):
        {
            cpu_info.fetch_data=gb_cpu_read_register(cpu_info.current_instruction->register_1);
            return;
        }
        case(AM_R_R):
        {
            cpu_info.fetch_data=gb_cpu_read_register(cpu_info.current_instruction->register_2);
            return;
        }

        case(AM_MR_R):
        {
            cpu_info.fetch_data=gb_cpu_read_register(cpu_info.current_instruction->register_2);
            cpu_info.memory_destination=gb_cpu_read_register(cpu_info.current_instruction->register_1);
            cpu_info.is_destination_to_memory=true;
            /*special case for the C register type when it holds an absolute address, defined 
            on page 14 of "Gameboy: Complete Technical Reference" written by Joonas Javanainen*/
            if(cpu_info.current_instruction->register_1==RT_C) 
            {
                cpu_info.memory_destination |= 0xFF00;
            }
            return;
        }
        case(AM_R_MR):
        {
            uint16_t address=gb_cpu_read_register(cpu_info.current_instruction->register_2);
            /*special case for the C register type */
            if(cpu_info.current_instruction->register_2==RT_C) 
            {
                address |= 0xFF00;
            }
            cpu_info.fetch_data=gb_bus_read(address);
            gb_emulator_cycle(1);
            return;
        }
        case(AM_R_HLI):
        {
            cpu_info.fetch_data=gb_bus_read(gb_cpu_read_register(cpu_info.current_instruction->register_2));
            gb_emulator_cycle(1);
            gb_cpu_set_register(RT_HL,gb_cpu_read_register(RT_HL)+1);
            return;
        }

        case(AM_R_HLD):
        {
            cpu_info.fetch_data=gb_bus_read(gb_cpu_read_register(cpu_info.current_instruction->register_2));
            gb_emulator_cycle(1);
            gb_cpu_set_register(RT_HL,gb_cpu_read_register(RT_HL)-1);
            return;
        }

        case(AM_HLI_R):
        {
            cpu_info.fetch_data=gb_cpu_read_register(cpu_info.current_instruction->register_2);
            cpu_info.memory_destination=gb_cpu_read_register(cpu_info.current_instruction->register_1);
            cpu_info.is_destination_to_memory=true;
            gb_cpu_set_register(RT_HL,gb_cpu_read_register(RT_HL)+1);
            return;
        }
        case(AM_HLD_R):
        {
            cpu_info.fetch_data=gb_cpu_read_register(cpu_info.current_instruction->register_2);
            cpu_info.memory_destination=gb_cpu_read_register(cpu_info.current_instruction->register_1);
            cpu_info.is_destination_to_memory=true;
            gb_cpu_set_register(RT_HL,gb_cpu_read_register(RT_HL)-1);
            return;
        }
        case(AM_R_A8):
        {
            cpu_info.fetch_data=gb_bus_read(cpu_info.registers.program_counter);
            gb_emulator_cycle(1);
            cpu_info.registers.program_counter++;
            return;
        }
        case(AM_A8_R):
        {
            cpu_info.memory_destination=gb_bus_read(cpu_info.registers.program_counter) | 0xFF00;
            cpu_info.is_destination_to_memory=true;
            gb_emulator_cycle(1);
            cpu_info.registers.program_counter++;
            return;
        }
        case(AM_R_D8):
        {
            cpu_info.fetch_data=gb_bus_read(cpu_info.registers.program_counter);
            gb_emulator_cycle(1);
            cpu_info.registers.program_counter++;
            return;
        }
        case(AM_MR_D8):
        {
            cpu_info.fetch_data=gb_bus_read(cpu_info.registers.program_counter);
            gb_emulator_cycle(1);
            cpu_info.registers.program_counter++;
            cpu_info.memory_destination=gb_cpu_read_register(cpu_info.current_instruction->register_1);
            cpu_info.is_destination_to_memory=true;
            return;
        }

        case(AM_MR):
        {
            cpu_info.memory_destination=gb_cpu_read_register(cpu_info.current_instruction->register_1);
            cpu_info.is_destination_to_memory=true;
            cpu_info.fetch_data=gb_bus_read(gb_cpu_read_register(cpu_info.current_instruction->register_1));
            gb_emulator_cycle(1);
            return;
        }
        case(AM_D16):
        {
            uint16_t low = gb_bus_read(cpu_info.registers.program_counter);
            gb_emulator_cycle(1);
            uint16_t high = gb_bus_read(cpu_info.registers.program_counter+1);
            gb_emulator_cycle(1);
            cpu_info.fetch_data=(high<<8)|low;
            cpu_info.registers.program_counter+=2;
            return;
        }
        case(AM_R_D16):
        {
            uint16_t low = gb_bus_read(cpu_info.registers.program_counter);
            gb_emulator_cycle(1);
            uint16_t high = gb_bus_read(cpu_info.registers.program_counter+1);
            gb_emulator_cycle(1);
            cpu_info.fetch_data=(high<<8)|low;
            cpu_info.registers.program_counter+=2;
            return;
        }
        case(AM_D16_R):
        {
            uint16_t low = gb_bus_read(cpu_info.registers.program_counter);
            gb_emulator_cycle(1);
            uint16_t high = gb_bus_read(cpu_info.registers.program_counter+1);
            gb_emulator_cycle(1);
            cpu_info.memory_destination=(high<<8)|low;
            cpu_info.is_destination_to_memory=true;
            cpu_info.registers.program_counter+=2;

            cpu_info.fetch_data=gb_cpu_read_register(cpu_info.current_instruction->register_2);
            return;
        }
        case(AM_A16_R):
        {
            uint16_t low = gb_bus_read(cpu_info.registers.program_counter);
            gb_emulator_cycle(1);
            uint16_t high = gb_bus_read(cpu_info.registers.program_counter+1);
            gb_emulator_cycle(1);
            cpu_info.memory_destination=(high<<8)|low;
            cpu_info.is_destination_to_memory=true;
            cpu_info.registers.program_counter+=2;

            cpu_info.fetch_data=gb_cpu_read_register(cpu_info.current_instruction->register_2);
            return;
        }
        case(AM_R_A16):
        {
            uint16_t low = gb_bus_read(cpu_info.registers.program_counter);
            gb_emulator_cycle(1);
            uint16_t high = gb_bus_read(cpu_info.registers.program_counter+1);
            gb_emulator_cycle(1);
            uint16_t address=(high<<8)|low;
            cpu_info.registers.program_counter+=2;
            cpu_info.fetch_data=gb_bus_read(address);
            gb_emulator_cycle(1);
            return;
        }
        case(AM_HL_SPR):
        {
            cpu_info.fetch_data=gb_bus_read(cpu_info.registers.program_counter);
            gb_emulator_cycle(1);
            cpu_info.registers.program_counter++;
            return;
        }
        case(AM_D8):
        {
            cpu_info.fetch_data=gb_bus_read(cpu_info.registers.program_counter);
            gb_emulator_cycle(1);
            cpu_info.registers.program_counter++;
            return;
        }
        default:
            printf("Unknown Addressing Mode! %d (%02X)\n", cpu_info.current_instruction->mode, cpu_info.current_opcode);
            exit(-7);
            return;

    }
}
struct gb_registers* gb_get_all_registers()
{
    return &cpu_info.registers;
}
static void gb_execute()
{
    IN_PROC process = instruction_get_processor(cpu_info.current_instruction->type);
    if(!process)
    {
        //Not implemented
    }
    process(&cpu_info);
}
uint8_t gb_cpu_read_register_cb(enum register_type regi)
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
        case RT_HL:
        {
            return gb_bus_read(gb_cpu_read_register(RT_HL));
        }
    }
}

void gb_cpu_set_register_cb(enum register_type regi, uint8_t value)
{
    switch(regi)
    {
        case RT_A:
        {
            cpu_info.registers.a=(value & 0xff);
            return;
        }
        case RT_F:
        {
            cpu_info.registers.f=(value & 0xff);
            return;
        }
        case RT_B:
        {
            cpu_info.registers.b=(value & 0xff);
            return;
        }
        case RT_C:
        {
            cpu_info.registers.c=(value & 0xff);
            return;
        }
        case RT_D:
        {
            cpu_info.registers.d=(value & 0xff);
            return;
        }
        case RT_E:
        {
            cpu_info.registers.e=(value & 0xff);
            return;
        } 
        case RT_H:
        {
            cpu_info.registers.h=(value & 0xff);
            return;
        }
        case RT_L:
        {
            cpu_info.registers.l=(value & 0xff);
            return;
        }
        case RT_HL:
        {
            gb_bus_write(value,gb_cpu_read_register(RT_HL));
            return;
        }
    }
           
}
uint8_t gb_cpu_get_interrupt_flags()
{
    return cpu_info.interrupt_flags;
}
void gb_cpu_set_interrupt_flags(uint8_t value)
{
    cpu_info.interrupt_flags=value;
}
void gb_cpu_request_interrupt(enum gb_interrupt_type interrupt)
{
    cpu_info.interrupt_flags |= interrupt;
}
bool gb_cpu_step()
{
    if(!cpu_info.is_halted)
    {

        uint16_t PC=cpu_info.registers.program_counter;
        gb_fetch_instruction();
        gb_emulator_cycle(1);
        gb_fetch_data(); 
        printf("%08llX - %04X: %-7s (%02X %02X %02X) A: %02X BC: %02X%02X DE: %02X%02X HL: %02X%02X FLAGS:%02X \n", gb_emulator_get_info()->timer_ticks, PC,  instruction_name(cpu_info.current_instruction->type), cpu_info.current_opcode,
        gb_bus_read(PC + 1), gb_bus_read(PC + 2), cpu_info.registers.a, cpu_info.registers.b, cpu_info.registers.c, cpu_info.registers.d, cpu_info.registers.e, cpu_info.registers.h, cpu_info.registers.l, cpu_info.registers.f);
        debug_print();
        
        if (cpu_info.current_instruction == NULL) {
            printf("Unknown Instruction! %02X\n", cpu_info.current_opcode);
            exit(-7);
        }
        debug_update();
        
        gb_execute();
        
        
    }
    else
    {
        gb_emulator_cycle(1);
        if(cpu_info.interrupt_flags)
        {
            cpu_info.is_halted=false;
        }
    }
    if(cpu_info.is_master_interrupt_enabled)
    {
        gb_cpu_handle_interrupts(&cpu_info);
        cpu_info.is_enabling_interrupt=false;
    }
    if(cpu_info.is_enabling_interrupt)
    {
        cpu_info.is_master_interrupt_enabled=true;
    }
    return true;
}