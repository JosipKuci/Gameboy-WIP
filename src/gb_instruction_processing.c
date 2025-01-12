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
        else if(value==0)
        {
            cpu_info->registers.f&=~(1U<<location);
        }
    
}
//Checks if the 7th bit of the flags register (the Zero flag) is set
int get_cpu_z_flag(struct gb_cpu_info* cpu_info)
{
    if(((cpu_info->registers.f)&(1<<7))==0b10000000)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int get_cpu_n_flag(struct gb_cpu_info* cpu_info)
{
    if(((cpu_info->registers.f)&(1<<6))==0b01000000)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
int get_cpu_h_flag(struct gb_cpu_info* cpu_info)
{
    if(((cpu_info->registers.f)&(1<<5))==0b00100000)
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
    if(((cpu_info->registers.f)&(1<<4))==0b00010000)
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
    bool zero = get_cpu_z_flag(cpu_info);
    bool carry = get_cpu_c_flag(cpu_info);
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
static void proc_stop(struct gb_cpu_info *cpu_info)
{
    //Weird instruction, from what i see none of the games use it
    printf("STOP INSTRUCTION (not implemented yet)\n");
    //exit(-1);
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
            gb_bus_write(cpu_info->fetch_data,cpu_info->memory_destination);
        }
        gb_emulator_cycle(1);
        return;
    }
    if (cpu_info->current_instruction->mode == AM_HL_SPR) {
        uint8_t hflag = (gb_cpu_read_register(cpu_info->current_instruction->register_2) & 0xF) + (cpu_info->fetch_data & 0xF) >= 0x10;

        uint8_t cflag = (gb_cpu_read_register(cpu_info->current_instruction->register_2) & 0xFF) + (cpu_info->fetch_data & 0xFF) >= 0x100;

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
        gb_cpu_set_register(cpu_info->current_instruction->register_1,gb_bus_read(cpu_info->fetch_data|0xFF00));
    }
    else
    {
        gb_bus_write(cpu_info->registers.a, cpu_info->memory_destination);
    }
    gb_emulator_cycle(1);
}
static void proc_jp(struct gb_cpu_info *cpu_info)
{
    jump_to_address(cpu_info,cpu_info->fetch_data,false);
}
static void proc_nop(struct gb_cpu_info *cpu_info)
{
    //Nothing has to be done in this instruction
}
static void proc_di(struct gb_cpu_info *cpu_info)
{
    cpu_info->is_master_interrupt_enabled=false;
}
static void proc_ei(struct gb_cpu_info *cpu_info)
{
    cpu_info->is_enabling_interrupt=true;
}
static void proc_xor(struct gb_cpu_info *cpu_info)
{
    cpu_info->registers.a = cpu_info->registers.a^(cpu_info->fetch_data&0xff);
    bool z=(cpu_info->registers.a==0);
    gb_cpu_set_flags(cpu_info,z,0,0,0);
}
static void proc_or(struct gb_cpu_info *cpu_info)
{
    cpu_info->registers.a = cpu_info->registers.a|(cpu_info->fetch_data&0xff);
    int z=(cpu_info->registers.a==0)?1:0;//myb
    gb_cpu_set_flags(cpu_info,z,0,0,0);
}
static void proc_and(struct gb_cpu_info *cpu_info)
{
    cpu_info->registers.a&=cpu_info->fetch_data;
    int z=(cpu_info->registers.a==0)?1:0; //Check if accumulator is equal to 0, reset n and c, set h
    gb_cpu_set_flags(cpu_info,z,0,1,0);
}

static void proc_cp(struct gb_cpu_info *cpu_info) //Comparing A with n, where we just subtract n from A and se if it is zero
{
    int check = (int)cpu_info->registers.a-(int)(cpu_info->fetch_data);
    bool z, n, h, c;
    z=(check==0);
    n=1;
    h=((int)(cpu_info->registers.a&0xF)-(int)(cpu_info->fetch_data&0xF)<0);
    c=(check<0);
    gb_cpu_set_flags(cpu_info,z,n,h,c);
}
static void proc_pop(struct gb_cpu_info *cpu_info)
{
    //Since the AF register contains the flags between the 8th and 4th least significant bit, 
    //All of them are changed when doing a stack pop, the 4 least significant bits should be set as 0.
    uint16_t low = gb_stack_pop();
    gb_emulator_cycle(1);
    uint16_t high = gb_stack_pop();
    gb_emulator_cycle(1);
    uint16_t value = (high<<8)|low;
    if(cpu_info->current_instruction->register_1==RT_AF)
    {
        gb_cpu_set_register(cpu_info->current_instruction->register_1,value&0xfff0);
    }
    else
    {
        gb_cpu_set_register(cpu_info->current_instruction->register_1,value);
    }
}

static void proc_push(struct gb_cpu_info *cpu_info)
{
    uint16_t high =(gb_cpu_read_register(cpu_info->current_instruction->register_1)>>8)&0xFF;
    gb_emulator_cycle(1);
    gb_stack_push(high);
    uint16_t low =gb_cpu_read_register(cpu_info->current_instruction->register_1)&0xFF;
    gb_emulator_cycle(1);
    gb_stack_push(low);
    gb_emulator_cycle(1);
}

static void proc_call(struct gb_cpu_info *cpu_info)
{
    jump_to_address(cpu_info, cpu_info->fetch_data, true);
}
//conditional jump to the relative address specified by the signed 8-bit operand offset.
static void proc_jr(struct gb_cpu_info *cpu_info)
{
            int8_t offset = (int8_t)(cpu_info->fetch_data);
            uint16_t address=cpu_info->registers.program_counter+offset;
            jump_to_address(cpu_info,address,false);
            return;
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
        gb_emulator_cycle(1);
        uint16_t high = gb_stack_pop();
        gb_emulator_cycle(1);
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

static void proc_inc(struct gb_cpu_info *cpu_info)//OVDJE SE VRATITI ZBOG HL-A
{
    uint16_t value=gb_cpu_read_register(cpu_info->current_instruction->register_1)+1;
    if(cpu_info->current_instruction->register_1>=RT_AF)//Do an extra m-cycle if register is 16-bit
    {
        gb_emulator_cycle(1);
    }
    if(cpu_info->current_instruction->register_1==RT_HL && cpu_info->current_instruction->mode==AM_MR)
    {
        value=gb_bus_read(gb_cpu_read_register(RT_HL))+1;
        value &= 0xFF;
        gb_bus_write(value,gb_cpu_read_register(RT_HL));
    }
    else
    {
        gb_cpu_set_register(cpu_info->current_instruction->register_1,value);
        value=gb_cpu_read_register(cpu_info->current_instruction->register_1);
    }
    if((cpu_info->current_opcode&0x03)==0x03)//Checks if opcode ends with 0x03, then we dont have to change CPU flags
    {
        return;
    }
    else
    {
        bool z,n,h,c;
        z=(value==0x0000);
        n=0;//reset
        h=((value&0x0F)==0x0000); //set if carry from bit 3
        //c=-1; //do not change value of c
        gb_cpu_set_flags(cpu_info,z,n,h,-1);
    }
    return;
}
    
static void proc_dec(struct gb_cpu_info *cpu_info)//OVDJE SE VRATITI ZBOG HL-A
{
    uint16_t value=gb_cpu_read_register(cpu_info->current_instruction->register_1)-1;
    if(cpu_info->current_instruction->register_1>=RT_AF)//Do an extra m-cycle if register is 16-bit
    {
        gb_emulator_cycle(1);
    }
        if(cpu_info->current_instruction->register_1==RT_HL && cpu_info->current_instruction->mode==AM_MR)
        {
            value = gb_bus_read(gb_cpu_read_register(RT_HL))-1;
            gb_bus_write(value,gb_cpu_read_register(RT_HL));
        }
        else
        {
            gb_cpu_set_register(cpu_info->current_instruction->register_1,value);
            value=gb_cpu_read_register(cpu_info->current_instruction->register_1);
        }
        if((cpu_info->current_opcode&0x0B)==0x0B)
            return;
        else
        {
            bool z,n,h,c;
                z=(value==0x0000);
                n=1;//set
                h=((value&0x0F)==0x0F); //set if no borrow from bit 4
                //c=-1;
                gb_cpu_set_flags(cpu_info,z,n,h,-1);
        }
}


static void proc_add(struct gb_cpu_info* cpu_info)
{
    int z,n,h,c;
    uint32_t value=gb_cpu_read_register(cpu_info->current_instruction->register_1)+cpu_info->fetch_data;
    if(cpu_info->current_instruction->register_1>=RT_AF)
    {
        gb_emulator_cycle(1);
    }
    if(cpu_info->current_instruction->register_1==RT_SP)
    {
        value=gb_cpu_read_register(cpu_info->current_instruction->register_1)+(char)cpu_info->fetch_data;
    }
    z=(value&0xFF)==0;
    h=(gb_cpu_read_register(cpu_info->current_instruction->register_1)&0xF)+(cpu_info->fetch_data&0xF)>=0x10;
    c=(int)(gb_cpu_read_register(cpu_info->current_instruction->register_1)&0xFF)+(int)(cpu_info->fetch_data&0xFF)>=0x100;
    if(cpu_info->current_instruction->register_1>=RT_AF)
    {
        z=-1;
        h=(gb_cpu_read_register(cpu_info->current_instruction->register_1)&0xFFF)+(cpu_info->fetch_data&0xFFF)>=0x1000;
        uint32_t carry_check=((uint32_t)gb_cpu_read_register(cpu_info->current_instruction->register_1))+((uint32_t)cpu_info->fetch_data);
        c=carry_check>=0x10000;
    }
    if(cpu_info->current_instruction->register_1==RT_SP)
    {
        z=0;
        h=(gb_cpu_read_register(cpu_info->current_instruction->register_1)&0xF)+(cpu_info->fetch_data&0xF)>=0x10;
        c=(int)(gb_cpu_read_register(cpu_info->current_instruction->register_1)&0xFF)+(int)(cpu_info->fetch_data&0xFF)>=0x100;
    }
    gb_cpu_set_register(cpu_info->current_instruction->register_1,value&0xFFFF);
    gb_cpu_set_flags(cpu_info,z,0,h,c);
}
static void proc_adc(struct gb_cpu_info* cpu_info)//add to accumulator n and carry
{
    uint16_t value=cpu_info->fetch_data;
    uint16_t carry = get_cpu_c_flag(cpu_info);
    uint16_t accumulator = cpu_info->registers.a;
    cpu_info->registers.a=(accumulator+carry+value)&0xFF;
    bool z, n, h, c;
    z=(cpu_info->registers.a==0);
    n=0;
    h=(value&0xF)+(accumulator&0xF)+carry>0xF;
    c=((value+carry+accumulator)>0xFF);
    gb_cpu_set_flags(cpu_info,z,n,h,c);
}

static void proc_sub(struct gb_cpu_info* cpu_info)
{
    uint16_t value=gb_cpu_read_register(cpu_info->current_instruction->register_1)-cpu_info->fetch_data;
    bool z, n, h, c;
    z=(value==0);
    n=1;
    h=((int)gb_cpu_read_register(cpu_info->current_instruction->register_1)&0xF)-((int)cpu_info->fetch_data&0xF)<0;
    c=((int)gb_cpu_read_register(cpu_info->current_instruction->register_1))-((int)cpu_info->fetch_data)<0;
    gb_cpu_set_register(cpu_info->current_instruction->register_1,value);
    gb_cpu_set_flags(cpu_info,z,n,h,c);

}
static void proc_sbc(struct gb_cpu_info* cpu_info)
{
    bool carry = get_cpu_c_flag(cpu_info);
    uint8_t value=cpu_info->fetch_data+carry;
    int z, n, h, c;
    z=gb_cpu_read_register(cpu_info->current_instruction->register_1)-value==0;
    n=1;
    h=((int)gb_cpu_read_register(cpu_info->current_instruction->register_1)&0xF)-((int)cpu_info->fetch_data&0xF)-(int)(carry)<0;
    c=(int)(gb_cpu_read_register(cpu_info->current_instruction->register_1))-((int)cpu_info->fetch_data)-(int)(carry)<0;
    gb_cpu_set_register(cpu_info->current_instruction->register_1,gb_cpu_read_register(cpu_info->current_instruction->register_1)-value);
    gb_cpu_set_flags(cpu_info,z,n,h,c);
}

static void proc_rlca(struct gb_cpu_info* cpu_info)//Rotate A left. Old bit 7 to Carry flag
{
    uint8_t temp = cpu_info->registers.a;
    bool carry=(temp>>7)&1;
    temp=(temp<<1)|carry;
    //Vidim da netko sam reseta taj Z, al vidit cemo...
    gb_cpu_set_flags(cpu_info,0,0,0,carry);
    cpu_info->registers.a=temp;
}
static void proc_rrca(struct gb_cpu_info* cpu_info)
{
    uint8_t temp = cpu_info->registers.a;
    bool carry=temp&1;
    temp=(temp>>1)|(carry<<7);
     //Vidim da netko sam reseta taj Z, al vidit cemo...
    gb_cpu_set_flags(cpu_info,0,0,0,carry);
    cpu_info->registers.a=temp;
}
static void proc_rla(struct gb_cpu_info* cpu_info)
{
    uint8_t temp = cpu_info->registers.a;
    bool old_carry=get_cpu_c_flag(cpu_info);
    bool carry=(temp>>7)&1;

    temp=(temp<<1)|old_carry;
    gb_cpu_set_flags(cpu_info,0,0,0,carry);
    cpu_info->registers.a=temp;
}
static void proc_rra(struct gb_cpu_info* cpu_info)
{
    uint8_t temp = cpu_info->registers.a;
    bool old_carry=get_cpu_c_flag(cpu_info);
    bool carry=temp&1;

    temp=(temp>>1)|(old_carry<<7);
    gb_cpu_set_flags(cpu_info,0,0,0,carry);
    cpu_info->registers.a=temp;
}
static void proc_daa(struct gb_cpu_info* cpu_info)
{
    //Decimal adjust register A.
    /*This instruction adjusts register A so that the
    correct representation of Binary Coded Decimal (BCD)
    is obtained*/
    uint8_t temp=0;
    bool new_carry = 0;
    bool n = get_cpu_n_flag(cpu_info);
    bool h = get_cpu_h_flag(cpu_info);
    bool c = get_cpu_c_flag(cpu_info);
    if(h ||(!n&&(cpu_info->registers.a&0xF)>9))
    {
        temp=6;
    }
    if(c ||(!n&&cpu_info->registers.a>0x99))
    {
        temp|=0x60;
        new_carry=1;
    }
    if(n)
    {
        cpu_info->registers.a+=-temp;
    }
    else
    {
        cpu_info->registers.a+=temp;
    }
    bool z=(cpu_info->registers.a==0);
    gb_cpu_set_flags(cpu_info,z,-1,0,new_carry);
}
static void proc_cpl(struct gb_cpu_info* cpu_info)
{
    //Flipping all the bits
    cpu_info->registers.a^=0b11111111;
    gb_cpu_set_flags(cpu_info,-1,1,1,-1);

}
static void proc_scf(struct gb_cpu_info* cpu_info)
{
    //Set carry flag, reset n and h
    gb_cpu_set_flags(cpu_info,-1,0,0,1);
}
static void proc_ccf(struct gb_cpu_info* cpu_info)
{
    //Complement carry flag, reset n and h
    bool c=get_cpu_c_flag(cpu_info)^1;
    gb_cpu_set_flags(cpu_info,-1,0,0,c);
}
static void proc_halt(struct gb_cpu_info* cpu_info)
{
    cpu_info->is_halted=true;
}
enum register_type register_lookup_cb[]={
    RT_B,
    RT_C,
    RT_D,
    RT_E,
    RT_H,
    RT_L,
    RT_HL,
    RT_A,
};
enum register_type decode_register_for_cb(uint8_t reg)
{
    if(reg>0b00000111)
    {
        return RT_NONE;
    }
    else
    {
        return register_lookup_cb[reg];
    }
}
static void proc_cb(struct gb_cpu_info* cpu_info)
{
    uint8_t cb_operation = cpu_info->fetch_data;
    enum register_type regi=decode_register_for_cb(cb_operation&0b00000111); //Getting which register we are working with from the opcode table
    uint8_t used_bit=(cb_operation>>3)&0b00000111;//Seeing what bit the operation needs
    uint8_t bit_operation = (cb_operation>>6)&0b00000011;
    uint8_t register_value = gb_cpu_read_register_cb(regi);
    bool z, n, h, c;
    gb_emulator_cycle(1);
    if(regi==RT_HL)
    {
        gb_emulator_cycle(2);

    }

    switch(bit_operation)
    {
        case 1://Test bit in register
        {
            z=!(register_value&(1<<used_bit));//Set if bit of register is 0
            n=0; //Reset
            h=1; //Set
           // c=-1; //not affected
            gb_cpu_set_flags(cpu_info,z,n,h,-1);
            return;
        }
        case 2:
        {
            //RST
            register_value&=~(1U<<used_bit);
            gb_cpu_set_register_cb(regi,register_value);
            return;
        }
        case 3:
        {
            //SET
            register_value|=(1U<<used_bit);
            gb_cpu_set_register_cb(regi,register_value);
            return;
        }
    }
    c=get_cpu_c_flag(cpu_info);
    switch(used_bit)
    {
        case 0:
            {
                //RLC
                c=false;
                c=(register_value&(1<<7));
                register_value=(register_value<<1)&0xFF;
                if(c)
                {
                    register_value|=1;//Setting the 1st bit as 1 if 7th bit was 1
                }
                z=(register_value==0);
                n=0;
                h=0;
                gb_cpu_set_flags(cpu_info,z,n,h,c);
                gb_cpu_set_register_cb(regi,register_value);
                return;
            }
            case 1:
            {
                //RRC
                c=(register_value&1);
                uint8_t old_register_value=register_value;
                register_value>>=1;
                register_value|=(old_register_value<<7);
                z=(register_value==0);
                n=0;
                h=0;
                gb_cpu_set_flags(cpu_info,z,n,h,c);
                gb_cpu_set_register_cb(regi,register_value);
                return;
            }
            case 2:
            {
                //RL
                uint8_t old_c = c;
                c=(register_value&(1<<7));//new carry is 7th bit of unprocessed register
                register_value=(register_value<<1);//shit register to left by 1
                register_value|=old_c;//least significant bit is bitwise OR-ed with carry
                z=(register_value==0);
                n=0;
                h=0;
                gb_cpu_set_flags(cpu_info,z,n,h,c);
                gb_cpu_set_register_cb(regi,register_value);
                return;
            }
            case 3:
            {
                //RR
                uint8_t old_c = c;
                c=(register_value&1);//new carry is 1st bit of unprocessed register
                register_value=(register_value>>1);//shit register to right by 1
                register_value|=(old_c<<7);//most significant bit is bitwise OR-ed with carry
                z=(register_value==0);
                n=0;
                h=0;
                gb_cpu_set_flags(cpu_info,z,n,h,c);
                gb_cpu_set_register_cb(regi,register_value);
                return;
            }
            case 4:
            {
                //SLA
                c=(register_value&(1<<7));
                register_value=(register_value<<1);
                z=(register_value==0);
                n=0;
                h=0;
                gb_cpu_set_flags(cpu_info,z,n,h,c);
                gb_cpu_set_register_cb(regi,register_value);
                return;
            }
            case 5://Shift n right into Carry. MSB doesn't change.
            {
                //SRA
                c=register_value&1;
                uint8_t temp=(int8_t)register_value>>1;
                n=0;
                h=0;
                gb_cpu_set_flags(cpu_info,!temp,n,h,c);
                gb_cpu_set_register_cb(regi,temp);
                return;
            }
            case 6: //switches high 4 bits and low 4 bits
            {
                uint8_t high=register_value&0x0F;
                uint8_t low=(register_value&0xF0)>>4;
                register_value=(high<<4)|low;
                gb_cpu_set_register_cb(regi,register_value);
                z=(register_value==0);
                n=0;
                h=0;
                c=0;
                gb_cpu_set_flags(cpu_info,z,n,h,c);
                return;
            }
            case 7://Shift n right into Carry. MSB set to 0.
            {
                //SRL
                c=register_value&1; //Mozda krivo, ali ono
                register_value=(register_value>>1);
                register_value&=~(1U<<7);
                z=(register_value==0);
                n=0;
                h=0;
                gb_cpu_set_flags(cpu_info,z,n,h,c);
                gb_cpu_set_register_cb(regi,register_value);
                return;
            }
        }


}

static IN_PROC processors[]={
    [IN_NONE]=proc_none,
    [IN_NOP]=proc_nop,
    [IN_LD]=proc_ld,
    [IN_LDH]=proc_ldh,
    [IN_JP]=proc_jp,
    [IN_DI]=proc_di, //Disables master interrupts
    [IN_EI]=proc_ei,
    [IN_XOR]=proc_xor,
    [IN_OR]=proc_or,
    [IN_AND]=proc_and,
    [IN_CP]=proc_cp,
    [IN_POP]=proc_pop,
    [IN_PUSH]=proc_push,
    [IN_CALL]=proc_call,
    [IN_JR]=proc_jr,
    [IN_RET]=proc_ret,
    [IN_RETI]=proc_reti, //returns and enables master interrupt
    [IN_RST]=proc_rst,
    [IN_INC]=proc_inc,
    [IN_DEC]=proc_dec,
    [IN_ADD]=proc_add,
    [IN_ADC]=proc_adc,
    [IN_SUB]=proc_sub,
    [IN_SBC]=proc_sbc,
    [IN_RRCA]=proc_rrca,
    [IN_RRA]=proc_rra,
    [IN_RLCA]=proc_rlca,
    [IN_RLA]=proc_rla,
    [IN_HALT]=proc_halt,
    [IN_DAA]=proc_daa,
    [IN_CPL]=proc_cpl,
    [IN_SCF]=proc_scf,
    [IN_CCF]=proc_ccf,
    [IN_CB]=proc_cb,
    [IN_STOP]=proc_stop,

};

IN_PROC instruction_get_processor(enum instruction_type type)
{
    return processors[type];
}