//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __MIPS_ISA_H__
#define __MIPS_ISA_H__

//-----------------------------------------------------------------
// General:
//-----------------------------------------------------------------
enum ERegisters
{
    REG_0_ZERO,
    REG_1,
    REG_2,
    REG_3,
    REG_4,
    REG_5,
    REG_6,
    REG_7,
    REG_8,
    REG_9,
    REG_10,
    REG_11,
    REG_12,
    REG_13,
    REG_14,
    REG_15,
    REG_16,
    REG_17,
    REG_18,
    REG_19,
    REG_20,
    REG_21,
    REG_22,
    REG_23,
    REG_24,
    REG_25,
    REG_26,
    REG_27,
    REG_28_GP,
    REG_29_SP,
    REG_30_FP,
    REG_31_RA,
    REGISTERS
};

//--------------------------------------------------------------------
// Instruction Encoding
//--------------------------------------------------------------------
#define OPCODE_INST_MASK            0x3F        
#define OPCODE_INST_SHIFT           26

#define OPCODE_RS_MASK              0x1f
#define OPCODE_RS_SHIFT             21

#define OPCODE_RT_MASK              0x1f        
#define OPCODE_RT_SHIFT             16

#define OPCODE_RD_MASK              0x1f
#define OPCODE_RD_SHIFT             11

#define OPCODE_RE_MASK              0x1f        
#define OPCODE_RE_SHIFT             6

#define OPCODE_FUNC_MASK            0x3f
#define OPCODE_FUNC_SHIFT           0
// or
#define OPCODE_IMM_MASK             0xFFFF
#define OPCODE_IMM_SHIFT            0
// or
#define OPCODE_ADDR_SHIFT           0
#define OPCODE_ADDR_MASK            0x3FFFFFF

//--------------------------------------------------------------------
// R Type
//--------------------------------------------------------------------
#define INSTR_R_SLL         0x00
#define INSTR_R_SRL         0x02
#define INSTR_R_SRA         0x03
#define INSTR_R_SLLV        0x04
#define INSTR_R_SRLV        0x06
#define INSTR_R_SRAV        0x07
#define INSTR_R_JR          0x08
#define INSTR_R_JALR        0x09
#define INSTR_R_SYSCALL     0x0c
#define INSTR_R_BREAK       0x0d
#define INSTR_R_MFHI        0x10
#define INSTR_R_MTHI        0x11
#define INSTR_R_MFLO        0x12
#define INSTR_R_MTLO        0x13
#define INSTR_R_MULT        0x18
#define INSTR_R_MULTU       0x19
#define INSTR_R_DIV         0x1a
#define INSTR_R_DIVU        0x1b
#define INSTR_R_ADD         0x20
#define INSTR_R_ADDU        0x21
#define INSTR_R_SUB         0x22
#define INSTR_R_SUBU        0x23
#define INSTR_R_AND         0x24
#define INSTR_R_OR          0x25
#define INSTR_R_XOR         0x26
#define INSTR_R_NOR         0x27
#define INSTR_R_SLT         0x2a
#define INSTR_R_SLTU        0x2b

#define INSTR_COP0          0x10
#define INSTR_COP1          0x11
#define INSTR_COP2          0x12
#define INSTR_COP3          0x13

//--------------------------------------------------------------------
// J Type
//--------------------------------------------------------------------
#define INSTR_J_JAL         0x03
#define INSTR_J_J           0x02
#define INSTR_J_BEQ         0x04
#define INSTR_J_BNE         0x05
#define INSTR_J_BLEZ        0x06
#define INSTR_J_BGTZ        0x07

//--------------------------------------------------------------------
// I Type
//--------------------------------------------------------------------
#define INSTR_I_ADDI        0x08
#define INSTR_I_ADDIU       0x09
#define INSTR_I_SLTI        0x0a
#define INSTR_I_SLTIU       0x0b
#define INSTR_I_ANDI        0x0c
#define INSTR_I_ORI         0x0d
#define INSTR_I_XORI        0x0e
#define INSTR_I_LUI         0x0f
#define INSTR_I_LB          0x20
#define INSTR_I_LH          0x21
#define INSTR_I_LW          0x23
#define INSTR_I_LBU         0x24
#define INSTR_I_LHU         0x25
#define INSTR_I_SB          0x28
#define INSTR_I_SH          0x29
#define INSTR_I_SW          0x2b

#define INSTR_I_REGIMM      0x01
#define INSTR_I_COND_BLTZAL 0x10
#define INSTR_I_COND_BLTZ   0x00
#define INSTR_I_COND_BGEZAL 0x11
#define INSTR_I_COND_BGEZ   0x01

#define INSTR_I_LWL         0x22
#define INSTR_I_LWR         0x26
#define INSTR_I_SWL         0x2a
#define INSTR_I_SWR         0x2e

#define INSTR_I_LWC0        0x30
#define INSTR_I_LWC1        0x31
#define INSTR_I_LWC2        0x32
#define INSTR_I_LWC3        0x33
#define INSTR_I_SWC0        0x38
#define INSTR_I_SWC1        0x39
#define INSTR_I_SWC2        0x3a
#define INSTR_I_SWC3        0x3b

//--------------------------------------------------------------------
// COP0
//--------------------------------------------------------------------
#define COP0_RFE            0x10
#define COP0_MFC0           0x00
#define COP0_MTC0           0x04

#define COP0_STATUS         12 // Processor status and control
    #define COP0_SR_IEC         0 // Interrupt enable (current)
    #define COP0_SR_KUC         1 // User mode (current)
    #define COP0_SR_IEP         2 // Interrupt enable (previous)
    #define COP0_SR_KUP         3 // User mode (previous)
    #define COP0_SR_IEO         4 // Interrupt enable (old)
    #define COP0_SR_KUO         5 // User mode (old)
    #define COP0_SR_IM0         8 // Interrupt mask
    #define COP0_SR_IM_MASK     0xFF
    #define COP0_SR_CU0         28 // User mode enable to COPx
    #define COP0_SR_CU_MASK     0xF
    #define SR_BF_GET(v,f)      (((v) >> (COP0_SR_##f)) & 0x1)
    #define SR_BF_GETM(v,f,m)   (((v) >> (COP0_SR_##f)) & (m))
    #define SR_BF_SET(v,f,n)    (((v) & ~(1 << COP0_SR_##f)) | ((n) << COP0_SR_##f))
#define COP0_CAUSE          13 // Cause of last general exception
    #define COP0_CAUSE_EXC      2
    #define COP0_CAUSE_EXC_MASK 0x1F
    #define COP0_CAUSE_IP0      8
    #define COP0_CAUSE_IP_MASK  0xFF
    #define COP0_CAUSE_IV       23
    #define COP0_CAUSE_CE       28
    #define COP0_CAUSE_CE_MASK  0x7
    #define COP0_CAUSE_BD       31
    #define CAUSE_BF_GET(v,f,m)      (((v) >> (COP0_CAUSE_##f)) & (m))
    #define CAUSE_BF_SET(v,f,m,n)    (((v) & ~((m) << COP0_CAUSE_##f)) | ((n) << COP0_CAUSE_##f))
#define COP0_EPC            14 // Program counter at last exception
#define COP0_BADADDR        8  // Bad address value
#define COP0_COUNT          9  // Processor cycle count
#define COP0_PRID           15 // Processor identification and revision
#define COP0_EBASE          15 // Exception vector base register

//--------------------------------------------------------------------
// Exception Codes
//--------------------------------------------------------------------
#define EXC_INT       0  // Interrupt
#define EXC_ADEL      4  // Address error exception (load or instruction fetch)
#define EXC_ADES      5  // Address error exception (store)
#define EXC_IBE       6  // Bus error exception (instruction fetch)
#define EXC_DBE       7  // Bus error exception (data reference: load or store)
#define EXC_SYS       8  // Syscall exception
#define EXC_BP        9  // Breakpoint exception
#define EXC_RI        10 // Reserved instruction exception
#define EXC_CPU       11 // Coprocessor Unusable exception 
#define EXC_OV        12 // Arithmetic Overflow exception

#endif
