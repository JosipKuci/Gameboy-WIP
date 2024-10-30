#include "gb_cpu.h"
#include "gb_data_bus.h"
#include "gb_stack.h"
#include <stdio.h>
static void set_bit(struct gb_cpu_info* cpu_info, uint8_t value, uint8_t location)
{
    if(value==1)
    {
        cpu_info->registers.f|=(1U<<location);
    }
    else
    {
        cpu_info->registers.f&=~(1U<<location);
    }
}
//Checks if the 7th bit of the flags register (the Zero flag) is set
int get_cpu_z_flag(struct gb_cpu_info* cpu_info)
{
    if((cpu_info->registers.f)&(1U<<7)==0b01000000)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
//Checks if the 4th bit of the flags register (the Carry flag) is set
int get_cpu_c_flag(struct gb_cpu_info* cpu_info)
{
    if(((cpu_info->registers.f)&(1U<<4))==0b00001000)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void gb_cpu_set_flags(struct gb_cpu_info* cpu_info, uint8_t z, uint8_t n, uint8_t h, uint8_t c)
{
    set_bit(cpu_info,z,7);
    set_bit(cpu_info,n,6);
    set_bit(cpu_info,h,5);
    set_bit(cpu_info,c,4);

}
static int check_condition(struct gb_cpu_info *cpu_info)
{
    int zero = get_cpu_z_flag(cpu_info);
    int carry = get_cpu_c_flag(cpu_info);
    switch(cpu_info->current_instruction->condition)
    {
        case CT_NONE:
            return 1;
        case CT_C:
            return carry;
        case CT_NC:
            return !carry;
        case CT_Z:
            return zero;
        case CT_NZ:
            return !zero;
    }
    return 0;
}
static void jump_to_address(struct gb_cpu_info *cpu_info, uint16_t address, bool does_push_sp)
{
    if(check_condition(cpu_info))
    {
        if(does_push_sp)
        {
            gb_emulator_cycle(2);
            gb_stack_push_16(cpu_info->registers.program_counter);
        }
        cpu_info->registers.program_counter=address;
        gb_emulator_cycle(1);
    }
}


static void proc_none(struct gb_cpu_info *cpu_info)
{
    printf("INVALID INSTRUCTION\n");
    exit(-1);
}

static void proc_ld(struct gb_cpu_info *cpu_info)
{
    if(cpu_info->is_destination_to_memory==true)
    {
        if(cpu_info->current_instruction->register_2>=RT_AF)
        {
            gb_emulator_cycle(1);
            gb_bus_write_16(cpu_info->fetch_data,cpu_info->memory_destination);
        }
        else
        {
            gb_emulator_cycle(1);
            gb_bus_write(cpu_info->fetch_data,cpu_info->memory_destination);
        }
    }
    if (cpu_info->current_instruction->mode == AM_HL_SPR) {
        uint8_t hflag = (gb_cpu_read_register(cpu_info->current_instruction->register_2) & 0xF) + 
            (cpu_info->fetch_data & 0xF) >= 0x10;

        uint8_t cflag = (gb_cpu_read_register(cpu_info->current_instruction->register_2) & 0xFF) + 
            (cpu_info->fetch_data & 0xFF) >= 0x100;

        gb_cpu_set_flags(cpu_info, 0, 0, hflag, cflag);
        gb_cpu_set_register(cpu_info->current_instruction->register_1, 
        gb_cpu_read_register(cpu_info->current_instruction->register_2) + (char)cpu_info->fetch_data);

        return;
    }
    gb_cpu_set_register(cpu_info->current_instruction->register_1,cpu_info->fetch_data);
}
static void proc_ldh(struct gb_cpu_info *cpu_info)
{
    if(cpu_info->current_instruction->register_1==RT_A)
    {
        gb_cpu_set_register(RT_A,gb_bus_read(cpu_info->fetch_data|0xFF00));
    }
    else
    {
        gb_bus_write(RT_A, cpu_info->fetch_data|0xFF00);
    }
    gb_emulator_cycle(1);
}
static void proc_jp(struct gb_cpu_info *cpu_info)
{
    if(check_condition(cpu_info))
    {
        cpu_info->registers.program_counter=cpu_info->fetch_data;
        gb_emulator_cycle(1);
    }
}
static void proc_nop(struct gb_cpu_info *cpu_info)
{
    //Nothing has to be done in this instruction
}
static void proc_di(struct gb_cpu_info *cpu_info)
{
    cpu_info->is_master_interrupt_enabled=false;
}
static void proc_xor(struct gb_cpu_info *cpu_info)
{
    cpu_info->registers.a = cpu_info->registers.a^(cpu_info->fetch_data&0xff);
    gb_cpu_set_flags(cpu_info,0,0,0,0);
}
static void proc_pop(struct gb_cpu_info *cpu_info)
{
    //Since the AF register contains the flags between the 8th and 4th least significant bit, 
    //All of them are changed when doing a stack pop, the 4 least significant bits should be set as 0.
    if(cpu_info->current_instruction->register_1==RT_AF)
    {
        gb_cpu_set_register(cpu_info->current_instruction->register_1,gb_stack_pop_16()&0xfff0);
    }
    else
    {
        gb_cpu_set_register(cpu_info->current_instruction->register_1,gb_stack_pop_16());
    }
}

static void proc_push(struct gb_cpu_info *cpu_info)
{
    uint16_t high =(gb_cpu_read_register(cpu_info->current_instruction->register_1)>>8)&0xFF;
    uint16_t low =gb_cpu_read_register(cpu_info->current_instruction->register_1)&0xFF;
    gb_stack_push(high);
    gb_emulator_cycle(1);
    gb_stack_push(low);
    gb_emulator_cycle(1);
}

static void proc_call(struct gb_cpu_info *cpu_info)
{
    jump_to_address(cpu_info, cpu_info->fetch_data, true);
}
//Unconditional jump to the relative address specified by the signed 8-bit operand e.
static void proc_jr(struct gb_cpu_info *cpu_info)
{
    int16_t offset = cpu_info->fetch_data & 0xff;
    offset+=cpu_info->registers.program_counter;
    jump_to_address(cpu_info,offset,false);

}
static void proc_ret(struct gb_cpu_info *cpu_info)
{
    if(cpu_info->current_instruction->condition != CT_NONE)
    {
        gb_emulator_cycle(1);
    }
    if(check_condition(cpu_info))
    {
        uint16_t low = gb_stack_pop();
        uint16_t high = gb_stack_pop();
        uint16_t n=(high<<8)|low;
        cpu_info->registers.program_counter=n;
        gb_emulator_cycle(1);
    }
}
static void proc_reti(struct gb_cpu_info *cpu_info)
{
    cpu_info->is_master_interrupt_enabled=true;
    proc_ret(cpu_info);
}

static void proc_rst(struct gb_cpu_info *cpu_info)
{
    jump_to_address(cpu_info, cpu_info->current_instruction->param, true);
}

static void proc_inc(struct gb_cpu_info *cpu_info)
{
    uint16_t value=gb_cpu_read_register(cpu_info->current_instruction->register_1)+1;
    if(cpu_info->current_instruction->register_1>=RT_AF)//Do an extra m-cycle if register is 16-bit
    {
        gb_emulator_cycle(1);
        if(cpu_info->current_instruction->register_1==RT_HL && cpu_info->is_destination_to_memory==true)
        {
            value &= 0xFF;
            gb_bus_write(value,gb_cpu_read_register(RT_HL));
            return;
        }
    }
    gb_cpu_set_register(cpu_info->current_instruction->register_1, value);
}
static IN_PROC processors[]={
    [IN_NONE]=proc_none,
    [IN_NOP]=proc_nop,
    [IN_LD]=proc_ld,
    [IN_LDH]=proc_ldh,
    [IN_JP]=proc_jp,
    [IN_DI]=proc_di, //Disables master interrupts
    [IN_XOR]=proc_xor,
    [IN_POP]=proc_pop,
    [IN_PUSH]=proc_push,
    [IN_CALL]=proc_call,
    [IN_JR]=proc_jr,
    [IN_RET]=proc_ret,
    [IN_RETI]=proc_reti, //returns and enables master interrupt
    [IN_RST]=proc_rst,

};

IN_PROC instruction_get_processor(enum instruction_type type)
{
    return processors[type];
}