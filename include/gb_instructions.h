#ifndef GB_INSTRUCTIONS_H
#define GB_INSTRUCTIONS_H
#include <stdint.h>
enum address_mode{
    AM_IMP,  //address mode: implied (nothing needed to be done)
    AM_R_D16,//address mode: immediate 16-bit value to register
    AM_R_R,  //address mode: register to register
    AM_MR_R, //address mode: register to memory location of register
    AM_R,
    AM_R_D8,
    AM_R_MR,
    AM_R_HLI,
    AM_R_HLD,
    AM_HLI_R,
    AM_HLD_R,
    AM_R_A8,
    AM_A8_R,
    AM_HL_SPR,
    AM_D16,
    AM_D8,
    AM_D16_R,
    AM_MR_D8,
    AM_MR,
    AM_A16_R,
    AM_R_A16
};

enum register_type{
    RT_NONE,
    RT_A,
    RT_F,
    RT_B,
    RT_C,
    RT_D,
    RT_E,
    RT_H,
    RT_L,
    RT_AF,
    RT_BC,
    RT_DE,
    RT_HL,
    RT_SP,
    RT_PC
};

enum instruction_type{
    IN_NONE,
    IN_NOP,
    IN_LD,
    IN_INC,
    IN_DEC,
    IN_RLCA,
    IN_ADD,
    IN_RRCA,
    IN_STOP,
    IN_RLA,
    IN_JR,
    IN_RRA,
    IN_DAA,
    IN_CPL,
    IN_SCF,
    IN_CCF,
    IN_HALT,
    IN_ADC,
    IN_SUB,
    IN_SBC,
    IN_AND,
    IN_XOR,
    IN_OR,
    IN_CP,
    IN_POP,
    IN_JP,
    IN_PUSH,
    IN_RET,
    IN_CB,
    IN_CALL,
    IN_RETI,
    IN_LDH,
    IN_JPHL,
    IN_DI,
    IN_EI,
    IN_RST,
    IN_ERR,
    //CB instructions...
    IN_RLC, 
    IN_RRC,
    IN_RL, 
    IN_RR,
    IN_SLA, 
    IN_SRA,
    IN_SWAP, 
    IN_SRL,
    IN_BIT, 
    IN_RES, 
    IN_SET
};


enum condition_type{
    CT_NONE, 
    CT_NZ, 
    CT_Z,
    CT_NC, 
    CT_C
};

struct gb_instruction{
    enum instruction_type type;
    enum address_mode mode;
    enum register_type register_1;
    enum register_type register_2;
    enum condition_type condition;
    uint8_t param;
};

struct gb_instruction *instruction_by_opcode(uint8_t opcode);

char *inst_name(enum instruction_type inst_type);

#endif