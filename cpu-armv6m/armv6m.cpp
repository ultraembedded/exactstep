//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "armv6m.h"
#include "armv6m_opcodes.h"

//-------------------------------------------------------------------
// Defines:
//-------------------------------------------------------------------
#define REG_SP      13
#define REG_LR      14
#define REG_PC      15
#define REGISTERS   16

#define APSR_N_SHIFT        (31)
#define APSR_Z_SHIFT        (30)
#define APSR_C_SHIFT        (29)
#define APSR_V_SHIFT        (28)

#define APSR_N          (1<<APSR_N_SHIFT)
#define APSR_Z          (1<<APSR_Z_SHIFT)
#define APSR_C          (1<<APSR_C_SHIFT)
#define APSR_V          (1<<APSR_V_SHIFT)

#define ALL_FLAGS   (APSR_N | APSR_Z | APSR_C | APSR_V)
#define FLAGS_NZC   (APSR_N | APSR_Z | APSR_C)

#define PRIMASK_PM          (1 << 0)

#define CONTROL_NPRIV       (1 << 0)
#define CONTROL_SPSEL       (1 << 1)
#define CONTROL_MASK        (CONTROL_NPRIV | CONTROL_SPSEL)

#define EXC_RETURN          0xFFFFFFE0

#define DPRINTF(l,a)        do { if (m_trace & l) printf a; } while (0)
#define TRACE_ENABLED(l)    (m_trace & l)

#define LOG_FETCH       (1 << 0)
#define LOG_REGISTERS   (1 << 1)
#define LOG_PUSHPOP     (1 << 2)
#define LOG_MEM         (1 << 3)
#define LOG_INST        (1 << 4)
#define LOG_FLAGS       (1 << 5)

//-----------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------
armv6m::armv6m(uint32_t baseAddr /*= 0*/, uint32_t len /*= 0*/)
{
    m_systick = new device_systick(0xE000E010, NULL, 0);    
    attach_device(m_systick);

    // Some memory defined
    if (len != 0)
        create_memory(baseAddr, len);

    reset(baseAddr);

}
//-----------------------------------------------------------------
// get_pc: Get PC
//-----------------------------------------------------------------
uint32_t armv6m::get_pc(void)
{
    return m_regfile[REG_PC];
}
//-----------------------------------------------------------------
// set_pc: Set PC
//-----------------------------------------------------------------
void armv6m::set_pc(uint32_t pc)
{
    m_regfile[REG_PC] = pc;
}
//-----------------------------------------------------------------
// set_register: Set register value
//-----------------------------------------------------------------
void armv6m::set_register(int r, uint32_t val)
{
    if (r < REGISTERS)
        m_regfile[r] = val;
}
//-----------------------------------------------------------------
// get_register: Get register value
//-----------------------------------------------------------------
uint32_t armv6m::get_register(int r)
{
    if (r < REGISTERS)
        return m_regfile[r];
    return 0;
}
//-----------------------------------------------------------------
// set_flag: Adjust flag fields
//-----------------------------------------------------------------
void armv6m::set_flag(uint32_t flag, bool val)
{
    if (val)
        m_apsr |= flag;
    else
        m_apsr &= ~flag;
}
//-----------------------------------------------------------------
// get_flag: Get particular flag
//-----------------------------------------------------------------
bool armv6m::get_flag(uint32_t flag)
{
    return (m_apsr & flag) != 0;
}
//-----------------------------------------------------------------
// reset: Reset CPU state
//-----------------------------------------------------------------
void armv6m::reset(uint32_t start_addr)
{
    // TODO: cpu::reset();

    for (int i=0;i<REGISTERS;i++)
        m_regfile[i] = 0; 

    m_apsr = 0;

    m_entry_point = start_addr;

    // Fetch SP & boot PC from vector table
    m_regfile[REG_SP] = read32(m_entry_point); 
    m_regfile[REG_LR] = 0;
    m_regfile[REG_PC] = read32(m_entry_point + 4) & ~1;

    // Start in thread mode with main stack selected
    m_msp = m_regfile[REG_SP];
    m_psp = 0;
    m_current_mode = MODE_THREAD;
    m_control = 0; // (SPSEL = 0, nPRIV = 0)
    m_primask = 0;

    // Misc
    m_ipsr = 0;
    m_epsr = 0;
    m_systick_irq = false;
}
//-----------------------------------------------------------------
// get_opcode: Get instruction from address
//-----------------------------------------------------------------
uint32_t armv6m::get_opcode(uint32_t address)
{
    return read32(address);
}
//-----------------------------------------------------------------
// step: Step through one instruction
//-----------------------------------------------------------------
void armv6m::step(void)
{
    uint16_t inst;
    uint16_t inst2;
    int inst_32_bit;

    // EXC_RETURN value in PC
    if ((m_regfile[REG_PC] & EXC_RETURN) == EXC_RETURN)
        armv6m_exc_return(m_regfile[REG_PC]);

    // Fetch
    inst = armv6m_read_inst(m_regfile[REG_PC]);

    // Decode
    inst_32_bit = armv6m_decode(inst);

    // [32-bit instruction] Fetch another half word
    if (inst_32_bit)
        inst2 = armv6m_read_inst(m_regfile[REG_PC]+2);
    else
        inst2 = 0;

    DPRINTF(LOG_FETCH, ("%08X: 0x%04X \n",m_regfile[REG_PC],inst));
    
    if (TRACE_ENABLED(LOG_INST))
        armv6m_dump_inst(inst);

    uint32_t pc_x = m_regfile[REG_PC];

    // Execute
    armv6m_execute(inst, inst2);

    // Monitor executed instructions
    commit_pc(pc_x);

    if (TRACE_ENABLED(LOG_REGISTERS))
    {
        int i;
        for (i=0;i<REGISTERS;i+=4)
        {
            printf("%d: ", i);
            if (i == 12)
                printf("%08x %08x\n", m_regfile[i+0], m_regfile[i+1]);
            else
                printf("%08x %08x %08x %08x\n", m_regfile[i+0], m_regfile[i+1], m_regfile[i+2], m_regfile[i+3]);
        }
        printf("Flags = %c%c%c%c\n", m_apsr & APSR_N ? 'N':'-', 
                                     m_apsr & APSR_Z ? 'Z':'-',
                                     m_apsr & APSR_C ? 'C':'-',
                                     m_apsr & APSR_V ? 'V':'-');
    }

    if (TRACE_ENABLED(LOG_FLAGS))
    {
        static uint32_t old_apsr = 0;

        if (m_apsr != old_apsr)
        {
            printf("%08X: Flags = %c%c%c%c\n", m_regfile[REG_PC],
                                    m_apsr & APSR_N ? 'N':'-', 
                                     m_apsr & APSR_Z ? 'Z':'-',
                                     m_apsr & APSR_C ? 'C':'-',
                                     m_apsr & APSR_V ? 'V':'-');
            old_apsr = m_apsr;
        }
    }

    // Systick
// TODO: Verify likely to be incorrect...
    if (m_systick_irq && (m_current_mode == MODE_THREAD) && !(m_primask & PRIMASK_PM))
    {
        m_regfile[REG_PC] = armv6m_exception(m_regfile[REG_PC], 15);
        m_systick_irq = false;
    }

    // Dump state
    if (TRACE_ENABLED(LOG_REGISTERS))
    {
        // Register trace
        int i;
        for (i=0;i<REGISTERS;i+=4)
        {
            DPRINTF(LOG_REGISTERS,( " %d: ", i));
            DPRINTF(LOG_REGISTERS,( " %08x %08x %08x %08x\n", m_regfile[i+0], m_regfile[i+1], m_regfile[i+2], m_regfile[i+3]));
        }
    }

    cpu::step();
}
//-----------------------------------------------------------------
// set_interrupt: Register pending interrupt
//-----------------------------------------------------------------
void armv6m::set_interrupt(int irq)
{
    // TODO: Decode id...
    m_systick_irq = true;
}

//-------------------------------------------------------------------
// armv6m_read_inst:
//-------------------------------------------------------------------
uint16_t armv6m::armv6m_read_inst(uint32_t addr)
{
    uint32_t val = 0;
    
    if (addr & 0x2)
        val = (read32(addr) >> 16) & 0xFFFF;
    else
        val = (read32(addr) >> 0) & 0xFFFF;

    return val;
}
//-------------------------------------------------------------------
// armv6m_update_sp:
//-------------------------------------------------------------------
void armv6m::armv6m_update_sp(uint32_t sp)
{
    m_regfile[REG_SP] = sp;

    // Update shadow stack pointer (depending on mode)
    if ((m_control & CONTROL_SPSEL) && (m_current_mode == MODE_THREAD))
        m_psp = sp;
    else
        m_msp = sp;
}
//-------------------------------------------------------------------
// armv6m_update_n_z_flags:
//-------------------------------------------------------------------
void armv6m::armv6m_update_n_z_flags(uint32_t rd)
{
    // Zero
    if (rd == 0)
        m_apsr |= APSR_Z; 
    else 
        m_apsr &=~APSR_Z;

    // Negative
    if (rd & 0x80000000)
        m_apsr |=APSR_N; 
    else 
        m_apsr &=~APSR_N;
}
//-------------------------------------------------------------------
// armv6m_add_with_carry:
//-------------------------------------------------------------------
uint32_t armv6m::armv6m_add_with_carry(uint32_t rn, uint32_t rm, uint32_t carry_in, uint32_t mask)
{
    uint32_t res;

    // Perform addition
    res = rn + rm + carry_in;

    // Zero
    if (mask & APSR_Z)
    {
        if ((res & 0xFFFFFFFF) == 0)
            m_apsr |= APSR_Z; 
        else 
            m_apsr &=~APSR_Z;
    }

    // Negative
    if (mask & APSR_N)
    {
        if (res & 0x80000000)
            m_apsr |=APSR_N; 
        else 
            m_apsr &=~APSR_N;
    }

    // Carry
    if (mask & APSR_C)
    {
        uint64_t unsigned_sum = (uint64_t)rn + (uint64_t)rm + carry_in;
        if (unsigned_sum == (uint64_t)res)
            m_apsr &=~APSR_C;
        else
            m_apsr |= APSR_C;
    }

    // Overflow
    if (mask & APSR_V)
    {
        int64_t signed_sum = (int64_t)(int32_t)rn + (int64_t)(int32_t)rm + carry_in;

        if (signed_sum == (int64_t)(int32_t)res)
            m_apsr &=~APSR_V;
        else
            m_apsr |= APSR_V;
    }

    return (uint32_t)res;
}
//-------------------------------------------------------------------
// armv6m_shift_left:
//-------------------------------------------------------------------
uint32_t armv6m::armv6m_shift_left(uint32_t val, uint32_t shift, uint32_t mask)
{
    uint64_t res = val;
    res <<= shift;

    // Carry Out (res[32])
    if (mask & APSR_C)
    {
        if (res & ((uint64_t)1 << 32))
            m_apsr |= APSR_C;
        else
            m_apsr &=~APSR_C;
    }

    // Zero
    if ((res & 0xFFFFFFFF) == 0)
        m_apsr |= APSR_Z; 
    else 
        m_apsr &=~APSR_Z;

    // Negative
    if (res & 0x80000000)
        m_apsr |=APSR_N; 
    else 
        m_apsr &=~APSR_N;

    return (uint32_t)res;
}
//-------------------------------------------------------------------
// armv6m_shift_right:
//-------------------------------------------------------------------
uint32_t armv6m::armv6m_shift_right(uint32_t val, uint32_t shift, uint32_t mask)
{
    uint32_t res = (shift >= 32) ? 0 : val;

    // Carry Out (val[shift-1])
    if ((mask & APSR_C) && (shift > 0))
    {
        // Last lost bit shifted right
        if ((val & (1 << (shift-1))) && (shift <= 32))
            m_apsr |= APSR_C;
        else
            m_apsr &=~APSR_C;
    }

    res >>= shift;

    // Zero
    if ((res & 0xFFFFFFFF) == 0)
        m_apsr |= APSR_Z; 
    else 
        m_apsr &=~APSR_Z;

    // Negative
    if (res & 0x80000000)
        m_apsr |=APSR_N; 
    else 
        m_apsr &=~APSR_N;

    return res;
}
//-------------------------------------------------------------------
// armv6m_arith_shift_right:
//-------------------------------------------------------------------
uint32_t armv6m::armv6m_arith_shift_right(uint32_t val, uint32_t shift, uint32_t mask)
{
    signed int res = val;

    // Carry Out (val[shift-1])
    if ((mask & APSR_C) && (shift > 0))
    {
        // Last lost bit shifted right
        if (val & (1 << (shift-1)))
            m_apsr |= APSR_C;
        else
            m_apsr &=~APSR_C;
    }

    res >>= shift;

    // Zero
    if ((res & 0xFFFFFFFF) == 0)
        m_apsr |= APSR_Z; 
    else 
        m_apsr &=~APSR_Z;

    // Negative
    if (res & 0x80000000)
        m_apsr |=APSR_N; 
    else 
        m_apsr &=~APSR_N;

    return res;
}
//-------------------------------------------------------------------
// armv6m_rotate_right:
//-------------------------------------------------------------------
uint32_t armv6m::armv6m_rotate_right(uint32_t val, uint32_t shift, uint32_t mask)
{
    uint32_t res;

    if (shift == 0)
        res = val;
    else
    {
        shift &= 0x1F;

        res = val >> shift;
        res |= (val << (32 - shift));
    }

    // Carry out
    if (res & 0x80000000)
        m_apsr |= APSR_C;
    else
        m_apsr &=~APSR_C;

    // Zero
    if ((res & 0xFFFFFFFF) == 0)
        m_apsr |= APSR_Z; 
    else 
        m_apsr &=~APSR_Z;

    // Negative
    if (res & 0x80000000)
        m_apsr |=APSR_N; 
    else 
        m_apsr &=~APSR_N;

    return res;
}
//-------------------------------------------------------------------
// armv6m_sign_extend:
//-------------------------------------------------------------------
uint32_t armv6m::armv6m_sign_extend(uint32_t val, int offset)
{
    if(val & (1 << (offset-1)))
        val |= (~0) << offset;
    else
        val &= ~((~0) << offset);

    return val;
}
//-------------------------------------------------------------------
// armv6m_dump_inst:
//-------------------------------------------------------------------
void armv6m::armv6m_dump_inst(uint16_t inst)
{
    int i = 0;

    while (instr_details[i].desc)
    {
        if ((inst & instr_details[i].mask) == instr_details[i].opcode)
        {
            DPRINTF(LOG_INST, (" %x: %s\n", m_regfile[REG_PC], instr_details[i].desc));
            break;
        }

        i++;
    }
}
//-------------------------------------------------------------------
// armv6m_exception: Handle exception, push registers to stack &
// adjust operating mode.
//-------------------------------------------------------------------
uint32_t armv6m::armv6m_exception(uint32_t pc, uint32_t exception)
{
    uint32_t sp;

    // Retrieve shadow stack pointer (depending on mode)
    if ((m_control & CONTROL_SPSEL) && (m_current_mode == MODE_THREAD))
        sp = m_psp;
    else
        sp = m_msp;

    // Push frame onto current stack
    sp-=4;
    write32(sp, m_apsr);
    sp-=4;
    write32(sp, m_regfile[REG_PC]);
    sp-=4;
    write32(sp, m_regfile[REG_LR]);
    sp-=4; 
    write32(sp, m_regfile[12]);
    sp-=4; 
    write32(sp, m_regfile[3]);
    sp-=4; 
    write32(sp, m_regfile[2]);
    sp-=4; 
    write32(sp, m_regfile[1]);
    sp-=4; 
    write32(sp, m_regfile[0]);
    m_regfile[REG_SP] = sp;

    // Record exception
    m_ipsr = exception & 0x3F;

    // Fetch exception vector address into PC
    m_regfile[REG_PC] = read32(m_entry_point + (exception * 4)) & ~1;

    // LR = Return to handler mode (recursive interrupt?)
    if (m_current_mode == MODE_HANDLER)
        m_regfile[REG_LR] = 0xFFFFFFF1;   
    else
    {
        // LR = Return to thread mode (with main stack)
        if ((m_control & CONTROL_SPSEL) == 0)
            m_regfile[REG_LR] = 0xFFFFFFF9;
        // LR = Return to thread mode (with process stack)
        else
            m_regfile[REG_LR] = 0xFFFFFFFD;
    }

    // Swap to handler mode
    m_current_mode = MODE_HANDLER;

    // Current stack is now main
    m_control &= ~CONTROL_SPSEL;

    return m_regfile[REG_PC];
}
//-------------------------------------------------------------------
// armv6m_exc_return: Handle returning from an exception
//-------------------------------------------------------------------
void armv6m::armv6m_exc_return(uint32_t pc)
{
    // Handler (exception) mode
    if(m_current_mode != MODE_HANDLER)
        return ;

    // EXC_RETURN value
    if((pc & EXC_RETURN) == EXC_RETURN)
    {
        uint32_t sp;

        // TODO: Should be 0x1F...
        switch (pc & 0xF)
        {
        // Return to handler mode (with main stack)
        case 0x1:
            m_current_mode = MODE_HANDLER;
            m_control &= ~CONTROL_SPSEL;
            break;
        // Return to thread mode (with main stack)
        case 0x9:
            m_current_mode = MODE_THREAD;
            m_control &= ~CONTROL_SPSEL;
            break;
        // Return to thread mode (with process stack)
        case 0xD:
            m_current_mode = MODE_THREAD;
            m_control |= CONTROL_SPSEL;
            break;
        default:
            assert(!"Not handled");
            break;
        }

        // Pop exception context
        sp = m_regfile[REG_SP];
        m_regfile[0] = read32(sp); 
        sp+=4;
        m_regfile[1] = read32(sp); 
        sp+=4;
        m_regfile[2] = read32(sp); 
        sp+=4;
        m_regfile[3] = read32(sp); 
        sp+=4;
        m_regfile[12] = read32(sp);
        sp+=4;
        m_regfile[REG_LR] = read32(sp);
        sp+=4;
        m_regfile[REG_PC] = read32(sp);
        sp+=4;
        m_apsr = read32(sp);
        sp+=4;
        armv6m_update_sp(sp);
    }
}
//-------------------------------------------------------------------
// armv6m_decode: Decode ARMv6m instruction
// Returns:
//  0 = 16-bit instruction, execute
//  1 = 32-bit instruction, fetch next word
//-------------------------------------------------------------------
int armv6m::armv6m_decode(uint16_t inst)
{
    int res = 0;
    int v_decoded = 0;

    m_rd = 0;
    m_rt = 0;
    m_rm = 0;
    m_rn = 0;
    m_imm = 0;
    m_reglist = 0;
    m_cond = 0;

    // Group 0?
    if (!v_decoded)
    {
        v_decoded = 1;
        m_inst_group = INST_IGRP0;
        switch(inst & INST_IGRP0_MASK)
        {
            // BCC - BCC <label>
            // 1 1 0 1 cond imm8
            case INST_BCC_OPCODE:
            {
                m_cond = (inst >> 8) & 0x0F;
                m_imm  = (inst >> 0) & 0xFF;
            }
            break;
            default:
                v_decoded = 0;
            break;
        }
    }

    // Group 1?
    if (!v_decoded)
    {
        v_decoded = 1;
        m_inst_group = INST_IGRP1;
        switch(inst & INST_IGRP1_MASK)
        {
            // ADDS - ADDS <Rdn>,#<imm8>
            // 0 0 1 1 0 Rdn imm8
            case INST_ADDS_1_OPCODE:
            // SUBS - SUBS <Rdn>,#<imm8>
            // 0 0 1 11 Rdn imm8
            case INST_SUBS_1_OPCODE:
            {
                m_rd = (inst >> 8) & 0x7;
                m_rn = m_rd;
                m_imm= (inst >> 0) & 0xFF;
            }
            break;

            // ADR - ADR <Rd>,<label>
            // 1 0 1 0 0 Rd imm8
            case INST_ADR_OPCODE:
            // MOVS - MOVS <Rd>,#<imm8>
            // 0 0 1 0 0 Rd imm8
            case INST_MOVS_OPCODE:
            {
                m_rd = (inst >> 8) & 0x7;
                m_imm= (inst >> 0) & 0xFF;
            }
            break;

            // ASRS - ASRS <Rd>,<Rm>,#<imm5>
            // 0 0 0 1 0 imm5 Rm Rd
            case INST_ASRS_OPCODE:
            // LSLS - LSLS <Rd>,<Rm>,#<imm5>
            // 0 0 0 0 0 imm5 Rm Rd
            case INST_LSLS_OPCODE:
            // LSRS - LSRS <Rd>,<Rm>,#<imm5>
            // 0 0 0 0 1 imm5 Rm Rd
            case INST_LSRS_OPCODE:
            {
                m_imm= (inst >> 6) & 0x1F;
                m_rm = (inst >> 3) & 0x7;
                m_rd = (inst >> 0) & 0x7;
            }
            break;

            // B - B <label>
            // 1 1 1 0 0 imm11
            case INST_B_OPCODE:
            {
                m_imm= (inst >> 0) & 0x7FF;
            }
            break;

            // BL - BL <label>
            // 1 1 1 01 S imm10 1 1 J1 1 J2 imm11
            case INST_BL_OPCODE:
            {
                // 32-bit instruction
                res = 1;
                m_imm = (inst >> 0) & 0x7FF;
                m_rd = REG_LR; // Implicit

                // TODO: Clean this up
                // Check next instruction to work out if this is a BL or MSR
                if ((armv6m_read_inst(m_regfile[REG_PC]+2) & 0xC000) != 0xC000)
                    v_decoded = 0;
            }
            break;

            // CMP - CMP <Rn>,#<imm8>
            // 0 0 1 0 1 Rn imm8
            case INST_CMP_OPCODE:
            {
                m_rn = (inst >> 8) & 0x7;
                m_imm= (inst >> 0) & 0xFF;
            }
            break;

            // LDM - LDM <Rn>!,<registers> <Rn> not included in <registers>
            // 1 1 0 0 1 Rn register_list
            // LDM - LDM <Rn>,<registers> <Rn> included in <registers>
            // 1 1 0 0 1 Rn register_list
            //case INST_LDM_1_OPCODE:
            case INST_LDM_OPCODE:   
            // STM - STM <Rn>!,<registers>
            // 1 1 0 0 0 Rn register_list
            case INST_STM_OPCODE:
            {
                m_rn = (inst >> 8) & 0x7;
                m_rd = m_rn;
                m_reglist = (inst >> 0) & 0xFF;
            }
            break;

            // LDR - LDR <Rt>, [<Rn>{,#<imm5>}]
            // 0 1 1 0 1 imm5 Rn Rt
            case INST_LDR_OPCODE:
            // LDRB - LDRB <Rt>,[<Rn>{,#<imm5>}]
            // 0 1 1 1 1 imm5 Rn Rt
            case INST_LDRB_OPCODE:
            // LDRH - LDRH <Rt>,[<Rn>{,#<imm5>}]
            // 1 0 0 0 1 imm5 Rn Rt
            case INST_LDRH_OPCODE:
            // STR - STR <Rt>, [<Rn>{,#<imm5>}]
            // 0 1 1 0 0 imm5 Rn Rt
            case INST_STR_OPCODE:
            // STRB - STRB <Rt>,[<Rn>,#<imm5>]
            // 0 1 1 1 0 imm5 Rn Rt
            case INST_STRB_OPCODE:
            // STRH - STRH <Rt>,[<Rn>{,#<imm5>}]
            // 1 0 0 0 0 imm5 Rn Rt
            case INST_STRH_OPCODE:
            {
                m_imm= (inst >> 6) & 0x1F;
                m_rn = (inst >> 3) & 0x7;
                m_rt = (inst >> 0) & 0x7;
                m_rd = m_rt;
            }
            break;

            // LDR - LDR <Rt>,<label>
            // 0 1 0 0 1 Rt imm8
            case INST_LDR_2_OPCODE:
            {
                m_rt = (inst >> 8) & 0x7;
                m_rd = m_rt;
                m_imm= (inst >> 0) & 0xFF;
            }
            break;

            // LDR - LDR <Rt>,[SP{,#<imm8>}]
            // 1 0 0 1 1 Rt imm8
            case INST_LDR_1_OPCODE:
            // STR - STR <Rt>,[SP,#<imm8>]
            // 1 0 0 1 0 Rt imm8
            case INST_STR_1_OPCODE:
            // ADD - ADD <Rd>,SP,#<imm8>
            // 1 0 1 0 1 Rd imm8
            case INST_ADD_1_OPCODE:
            {
                m_rt = (inst >> 8) & 0x7;
                m_rn = REG_SP;
                m_rd = m_rt;
                m_imm= (inst >> 0) & 0xFF;
            }
            break;

            default:
                v_decoded = 0;
            break;
        }
    }

    // Group 2?
    if (!v_decoded)
    {
        v_decoded = 1;
        m_inst_group = INST_IGRP2;
        switch(inst & INST_IGRP2_MASK)
        {

            // ADDS - ADDS <Rd>,<Rn>,#<imm3>
            // 0 0 0 1 1 1 0 imm3 Rn Rd
            case INST_ADDS_OPCODE:
            // SUBS - SUBS <Rd>,<Rn>,#<imm3>
            // 0 0 0 11 1 1 imm3 Rn Rd
            case INST_SUBS_OPCODE:
            {
                m_imm= (inst >> 6) & 0x7;
                m_rn = (inst >> 3) & 0x7;
                m_rd = (inst >> 0) & 0x7;
            }
            break;

            // ADDS - ADDS <Rd>,<Rn>,<Rm>
            // 0 0 0 1 1 0 0 Rm Rn Rd
            case INST_ADDS_2_OPCODE:
            // SUBS - SUBS <Rd>,<Rn>,<Rm>
            // 0 0 0 11 0 1 Rm Rn Rd
            case INST_SUBS_2_OPCODE:
            {
                m_rm = (inst >> 6) & 0x7;
                m_rn = (inst >> 3) & 0x7;
                m_rd = (inst >> 0) & 0x7;
            }
            break;

            // LDR - LDR <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 1 0 0 Rm Rn Rt
            case INST_LDR_3_OPCODE:
            // LDRB - LDRB <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 1 1 0 Rm Rn Rt
            case INST_LDRB_1_OPCODE:
            // LDRH - LDRH <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 1 0 1 Rm Rn Rt
            case INST_LDRH_1_OPCODE:
            // LDRSB - LDRSB <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 0 1 1 Rm Rn Rt
            case INST_LDRSB_OPCODE:
            // LDRSH - LDRSH <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 1 1 1 Rm Rn Rt
            case INST_LDRSH_OPCODE:
            // STR - STR <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 0 00 Rm Rn Rt
            case INST_STR_2_OPCODE:
            // STRB - STRB <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 0 1 0 Rm Rn Rt
            case INST_STRB_1_OPCODE:
            // STRH - STRH <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 0 0 1 Rm Rn Rt
            case INST_STRH_1_OPCODE:
            {
                m_rm = (inst >> 6) & 0x7;
                m_rn = (inst >> 3) & 0x7;
                m_rt = (inst >> 0) & 0x7;
                m_rd = m_rt;
            }
            break;

            // POP - POP <registers>
            // 1 0 1 1 1 1 0 P register_list
            case INST_POP_OPCODE:
            {
                m_reglist = (inst >> 0) & 0xFF;
                if (inst & (1 << 8))
                    m_reglist |= (1 << REG_PC);
            }
            break;

            // PUSH - PUSH <registers>
            // 1 0 1 1 0 1 0 M register_list
            case INST_PUSH_OPCODE:
            {
                m_reglist = (inst >> 0) & 0xFF;
                if (inst & (1 << 8))
                    m_reglist |= (1 << REG_LR);
            }
            break;

            default:
                v_decoded = 0;
            break;
        }
    }

    // Group 3?
    if (!v_decoded)
    {
        v_decoded = 1;
        m_inst_group = INST_IGRP3;
        switch(inst & INST_IGRP3_MASK)
        {
            // ADD - ADD <Rdn>,<Rm>
            // 0 1 0 0 0 1 0 0 Rm Rdn
            case INST_ADD_OPCODE:
            {
                m_rm = (inst >> 3) & 0xF;
                m_rd = (inst >> 0) & 0x7;
                m_rd|= (inst >> 4) & 0x8;
                m_rn = m_rd;
            }
            break;

            // BKPT - BKPT #<imm8>
            // 1 0 1 1 1 1 1 0 imm8
            case INST_BKPT_OPCODE:
            // SVC - SVC #<imm8>
            // 1 1 0 1 111 1 imm8
            case INST_SVC_OPCODE:
            // UDF - UDF #<imm8>
            // 1 1 0 1 1 1 1 0 imm8
            case INST_UDF_OPCODE:
            {
                m_imm = (inst >> 0) & 0xFF;
            }
            break;

            // CMP - CMP <Rn>,<Rm> <Rn> and <Rm> not both from R0-R7
            // 0 1 0 0 0 1 0 1 N Rm Rn
            case INST_CMP_2_OPCODE:
            {
                m_rm = (inst >> 3) & 0xF;
                m_rn = (inst >> 0) & 0x7;
                m_rn|= (inst >> 4) & 0x8;
            }
            break;

            // MOV - MOV <Rd>,<Rm> Otherwise all versions of the Thumb instruction set.
            // 0 1 0 0 0 1 1 0 D Rm Rd
            case INST_MOV_OPCODE:
            {
                m_rm = (inst >> 3) & 0xF;
                m_rd = (inst >> 0) & 0x7;
                m_rd|= (inst >> 4) & 0x8;
            }
            break;
            default:
                v_decoded = 0;
            break;
        }
    }

    // Group 4?
    if (!v_decoded)
    {
        v_decoded = 1;
        m_inst_group = INST_IGRP4;
        switch(inst & INST_IGRP4_MASK)
        {
            // ADD - ADD SP,SP,#<imm7>
            // 1 0 1 1 0 0 0 0 0 imm7
            case INST_ADD_2_OPCODE:
            // SUB - SUB SP,SP,#<imm7>
            // 1 0 1 1 000 0 1 imm7
            case INST_SUB_OPCODE:
            {
                m_rn = REG_SP; // Implicit
                m_rd = REG_SP; // Implicit
                m_imm= (inst >> 0) & 0x7F;
            }
            break;

            // BLX - BLX <Rm>
            // 0 1 0 0 0 1 1 1 1 Rm (0) (0) (0)
            case INST_BLX_OPCODE:
            {
                m_rm = (inst >> 3) & 0xF;
                m_rd = REG_LR;
            }
            break;

            // BX - BX <Rm>
            // 0 1 0 0 0 1 1 1 0 Rm (0) (0) (0)
            case INST_BX_OPCODE:
            {
                m_rm = (inst >> 3) & 0xF;
            }
            break;

            default:
                v_decoded = 0;
            break;
        }
    }

    // Group 5?
    if (!v_decoded)
    {
        v_decoded = 1;
        m_inst_group = INST_IGRP5;
        switch(inst & INST_IGRP5_MASK)
        {
            // ADCS - ADCS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 1 0 1 Rm Rdn
            case INST_ADCS_OPCODE:
            // ANDS - ANDS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 0 0 0 Rm Rdn
            case INST_ANDS_OPCODE:
            // ASRS - ASRS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 1 0 0 Rm Rdn
            case INST_ASRS_1_OPCODE:
            // BICS - BICS <Rdn>,<Rm>
            // 0 1 0 0 0 0 1 1 1 0 Rm Rdn
            case INST_BICS_OPCODE:
            // EORS - EORS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 0 0 1 Rm Rdn
            case INST_EORS_OPCODE:
            // LSLS - LSLS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 0 1 0 Rm Rdn
            case INST_LSLS_1_OPCODE:
            // LSRS - LSRS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 0 1 1 Rm Rdn
            case INST_LSRS_1_OPCODE:
            // ORRS - ORRS <Rdn>,<Rm>
            // 0 1 0 0 0 0 1 1 0 0 Rm Rdn
            case INST_ORRS_OPCODE:
            // RORS - RORS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 1 1 1 Rm Rdn
            case INST_RORS_OPCODE:
            // SBCS - SBCS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 1 1 0 Rm Rdn
            case INST_SBCS_OPCODE:
            {
                m_rm = (inst >> 3) & 0x7;
                m_rd = (inst >> 0) & 0x7;
                m_rn = m_rd;
            }
            break;

            // CMN - CMN <Rn>,<Rm>
            // 0 1 0 0 0 0 1 0 1 1 Rm Rn
            case INST_CMN_OPCODE:
            // CMP - CMP <Rn>,<Rm> <Rn> and <Rm> both from R0-R7
            // 0 1 0 0 0 0 1 0 1 0 Rm Rn
            case INST_CMP_1_OPCODE:
            // TST - TST <Rn>,<Rm>
            // 000 1 0 0 1 0 0 0 Rm Rn
            case INST_TST_OPCODE:
            {
                m_rm = (inst >> 3) & 0x7;
                m_rn = (inst >> 0) & 0x7;
            }
            break;

            // MULS - MULS <Rdm>,<Rn>,<Rdm>
            // 0 1 0 0 0 0 1 1 0 1 Rn Rdm
            case INST_MULS_OPCODE:
            {
                m_rn = (inst >> 3) & 0x7;
                m_rd = (inst >> 0) & 0x7;
                m_rm = m_rd;
            }
            break;

            // MVNS - MVNS <Rd>,<Rm>
            // 0 1 0 0 0 0 1 1 1 1 Rm Rd
            case INST_MVNS_OPCODE:
            // REV - REV <Rd>,<Rm>
            // 1 0 1 1 1 0 1 0 0 0 Rm Rd
            case INST_REV_OPCODE:
            // REV16 - REV16 <Rd>,<Rm>
            // 1 0 1 1 1 0 1 0 0 1 Rm Rd
            case INST_REV16_OPCODE:
            // REVSH - REVSH <Rd>,<Rm>
            // 1 0 1 1 1 0 1 0 1 1 Rm Rd
            case INST_REVSH_OPCODE:
            // SXTB - SXTB <Rd>,<Rm>
            // 1 0 1 1 100 0 0 1 Rm Rd
            case INST_SXTB_OPCODE:
            // SXTH - SXTH <Rd>,<Rm>
            // 1 0 1 1 100 0 0 0 Rm Rd
            case INST_SXTH_OPCODE:
            // UXTB - UXTB <Rd>,<Rm>
            // 1 0 1 1 100 0 1 1 Rm Rd
            case INST_UXTB_OPCODE:
            // UXTH - UXTH <Rd>,<Rm>
            // 1 0 1 1 100 0 1 0 Rm Rd
            case INST_UXTH_OPCODE:
            {
                m_rm = (inst >> 3) & 0x7;
                m_rd = (inst >> 0) & 0x7;
            }
            break;

            // RSBS - RSBS <Rd>,<Rn>,#0
            // 0 1 0 0 0 0 1 0 0 1 Rn Rd
            case INST_RSBS_OPCODE:
            {
                m_rn = (inst >> 3) & 0x7;
                m_rd = (inst >> 0) & 0x7;
            }
            break;

            default:
                v_decoded = 0;
            break;
        }
    }

    // Group 6?
    if (!v_decoded)
    {
        v_decoded = 1;
        m_inst_group = INST_IGRP6;
        switch(inst & INST_IGRP6_MASK)
        {
            // MRS - MRS <Rd>,<spec_reg>
            // 1 1 1 01 0 1 1 1 1 1 (0) (1) (1) (1) (1) 1 0 (0) 0 Rd SYSm
            case INST_MRS_OPCODE:
            {
                // 32-bit instruction
                res = 1;
            }
            break;
            // MSR - MSR <spec_reg>,<Rn>
            // 1 1 1 01 0 1 1 1 0 0 (0) Rn 1 0 (0) 0 (1) (0) (0) (0) SYSm
            case INST_MSR_OPCODE:
            {
                m_rn = (inst >> 0) & 0xF;

                // 32-bit instruction
                res = 1;                
            }
            break;
            // CPS - CPS<effect> i
            // 1 0 1 1 0 1 1 0 0 1 1 im (0) (0) (1) (0)
            case INST_CPS_OPCODE:
            {
                m_imm = (inst >> 4) & 0x1;
            }
            break;
            default:
                v_decoded = 0;
            break;
        }
    }

    // Group 7?
    if (!v_decoded)
    {
        v_decoded = 1;
        m_inst_group = INST_IGRP7;
        switch(inst & INST_IGRP7_MASK)
        {
            // DMB - DMB #<option>
            // 1 1 1 01 0 1 1 1 0 1 1 (1) (1) (1) (1) 1 0 (0) 0 (1) (1) (1) (1) 0 1 0 1 option
            //case INST_DMB_OPCODE:
            // DSB - DSB #<option>
            // 1 1 1 01 0 1 1 1 0 1 1 (1) (1) (1) (1) 1 0 (0) 0 (1) (1) (1) (1) 0 1 0 0 option
            //case INST_DSB_OPCODE:
            // ISB - ISB #<option>
            // 1 1 1 01 0 1 1 1 0 1 1 (1) (1) (1) (1) 1 0 (0) 0 (1) (1) (1) (1) 0 1 1 0 option
            case INST_ISB_OPCODE:
            {
                // 32-bit instruction
                res = 1;

                // Do nothing
            }
            break;
            // UDF_W - UDF_W #<imm16>
            // 1 11 1 0 1 1 1 1 1 1 1 imm4 1 0 1 0 imm12
            case INST_UDF_W_OPCODE:
            {
                // 32-bit instruction
                res = 1;

                // Do nothing
            }
            break;
            default:
                v_decoded = 0;
            break;
        }
    }

    // Group 8?
    if (!v_decoded)
    {
        v_decoded = 1;
        m_inst_group = INST_IGRP8;
        switch(inst & INST_IGRP8_MASK)
        {
            // NOP - NOP
            // 1 0 1 1 1 1 1 1 0 0 0 0 0 0 0 0
            case INST_NOP_OPCODE:
            {
                // Do nothing
            }
            break;
            // SEV - SEV
            // 1 0 1 1 1 1 1 1 0 1 0 0 0 0 0 0
            case INST_SEV_OPCODE:
            {
                // Do nothing
            }
            break;
            // WFE - WFE
            // 1 0 1 1 1 1 1 1 0 0 1 0 0 0 0 0
            case INST_WFE_OPCODE:
            {
                // Do nothing
            }
            break;
            // WFI - WFI
            // 1 0 1 1 1 1 1 1 0 0 1 1 0 0 0 0
            case INST_WFI_OPCODE:
            {
                // Do nothing
                assert(0);
            }
            break;
            // YIELD - YIELD
            // 1 0 1 1 1 1 1 1 0 0 0 1 0 0 0 0
            case INST_YIELD_OPCODE:
            {
                // Do nothing
                assert(0);
            }
            break;
            default:
                v_decoded = 0;
            break;
        }
    }

    if (!v_decoded)
    {
        assert(!"Instruction decode failed");
    }

    return res;
}
//-------------------------------------------------------------------
// armv6m_execute:
//-------------------------------------------------------------------
void armv6m::armv6m_execute(uint16_t inst, uint16_t inst2)
{
    uint32_t reg_rm = m_regfile[m_rm];
    uint32_t reg_rn = m_regfile[m_rn];
    uint32_t reg_rd = 0;
    uint32_t pc = m_regfile[REG_PC];
    uint32_t offset = 0;
    int write_rd = 0;

    // Increment PC to next location
    pc += 2;

    switch (m_inst_group)
    {
    case INST_IGRP0:
    {
        switch(inst & INST_IGRP0_MASK)
        {
            // BCC - BCC <label>
            // 1 1 0 1 m_cond imm8
            case INST_BCC_OPCODE:
            {
                // Sign extend offset
                offset = armv6m_sign_extend(m_imm, 8);

                // Convert to words
                offset = offset << 1;

                // Make relative to PC + 4
                offset = offset + pc + 2;               

                switch (m_cond)
                {
                    case 0: // EQ
                        if (m_apsr & APSR_Z)
                            pc = offset;
                        break;
                    case 1: // NE
                        if ((m_apsr & APSR_Z) == 0)
                            pc = offset;
                        break;
                    case 2: // CS/HS
                        if (m_apsr & APSR_C)
                            pc = offset;
                        break;
                    case 3: // CC/LO
                        if ((m_apsr & APSR_C) == 0)
                            pc = offset;
                        break;
                    case 4: // MI
                        if (m_apsr & APSR_N)
                            pc = offset;
                        break;
                    case 5: // PL
                        if ((m_apsr & APSR_N) == 0)
                            pc = offset;
                        break;
                    case 6: // VS
                        if (m_apsr & APSR_V)
                            pc = offset;
                        break;
                    case 7: // VC
                        if ((m_apsr & APSR_V) == 0)
                            pc = offset;
                        break;
                    case 8: // HI
                        if ((m_apsr & APSR_C) && ((m_apsr & APSR_Z) == 0))
                            pc = offset;
                        break;
                    case 9: // LS
                        if (((m_apsr & APSR_C) == 0) || (m_apsr & APSR_Z))
                            pc = offset;

                        break;
                    case 10: // GE
                        if (((m_apsr & APSR_N) >> APSR_N_SHIFT) == ((m_apsr & APSR_V) >> APSR_V_SHIFT))
                            pc = offset;
                        break;
                    case 11: // LT
                        if (((m_apsr & APSR_N) >> APSR_N_SHIFT) != ((m_apsr & APSR_V) >> APSR_V_SHIFT))
                            pc = offset;
                        break;
                    case 12: // GT
                        if (((m_apsr & APSR_Z) == 0) && (((m_apsr & APSR_N) >> APSR_N_SHIFT) == ((m_apsr & APSR_V) >> APSR_V_SHIFT)))
                            pc = offset;
                        break;
                    case 13: // LE
                        if ((m_apsr & APSR_Z) || (((m_apsr & APSR_N) >> APSR_N_SHIFT) != ((m_apsr & APSR_V) >> APSR_V_SHIFT)))
                            pc = offset;
                        break;
                    case 14: // AL
                        pc = offset;
                        break;
                    case 15: // SVC
                        pc = armv6m_exception(pc, 11);
                        break;
                    default:
                        assert(!"Bad condition code");
                        break;
                }
            }
            break;
        }
    }
    break;
    case INST_IGRP1:
    {
        switch(inst & INST_IGRP1_MASK)
        {
            // ADDS - ADDS <Rdn>,#<imm8>
            // 0 0 1 1 0 Rdn imm8
            case INST_ADDS_1_OPCODE:
            {
                reg_rd = armv6m_add_with_carry(reg_rn, m_imm, 0, ALL_FLAGS);
                write_rd = 1;
            }
            break;
            // ADD - ADD <Rd>,SP,#<imm8>
            // 1 0 1 0 1 Rd imm8
            case INST_ADD_1_OPCODE:
            {
                reg_rd = reg_rn + (m_imm << 2);
                write_rd = 1;
            }
            break;
            // ADR - ADR <Rd>,<label>
            // 1 0 1 0 0 Rd imm8
            case INST_ADR_OPCODE:
            {
                reg_rd = pc + m_imm + 2;
                write_rd = 1;
            }
            break;
            // ASRS - ASRS <Rd>,<Rm>,#<imm5>
            // 0 0 0 1 0 imm5 Rm Rd
            case INST_ASRS_OPCODE:
            {
                if (m_imm == 0)
                    m_imm = 32;

                reg_rd = armv6m_arith_shift_right(reg_rm, m_imm, FLAGS_NZC);
                write_rd = 1;
            }
            break;
            // B - B <label>
            // 1 1 1 0 0 imm11
            case INST_B_OPCODE:
            {
                // Sign extend offset
                offset = armv6m_sign_extend(m_imm, 11);

                // Convert to words
                offset = offset << 1;

                // Make relative to PC + 4
                offset = offset + pc + 2;

                pc = offset;
            }
            break;
            // BL - BL <label>
            // 1 1 1 01 S imm10 1 1 J1 1 J2 imm11
            case INST_BL_OPCODE:
            {
                // Sign extend
                offset = armv6m_sign_extend(m_imm, 11);
                offset <<= 11;

                // Additional range
                m_imm = (inst2 >> 0) & 0x7FF;
                offset |= m_imm;

                // Make relative to PC
                offset <<= 1;
                offset += pc;

                // m_rd = REG_LR
                reg_rd = (pc + 2) | 1;
                write_rd = 1;

                pc = offset + 2;
            }
            break;
            // CMP - CMP <Rn>,#<imm8>
            // 0 0 1 0 1 Rn imm8
            case INST_CMP_OPCODE:
            {
                reg_rd = armv6m_add_with_carry(reg_rn, ~m_imm, 1, ALL_FLAGS);
                // No writeback
            }
            break;
            // LDM - LDM <Rn>!,<registers> <Rn> not included in <registers>
            // 1 1 0 0 1 Rn register_list
            // LDM - LDM <Rn>,<registers> <Rn> included in <registers>
            // 1 1 0 0 1 Rn register_list
            //case INST_LDM_1_OPCODE:
            case INST_LDM_OPCODE:   
            {
                int i;

                for (i=0;i<REGISTERS && m_reglist != 0;i++)
                {
                    if (m_reglist & (1 << i))
                    {
                        m_regfile[i] = read32(reg_rn);
                        if (i == REG_PC)
                        {
                            if ((m_regfile[i] & EXC_RETURN) != EXC_RETURN)
                                m_regfile[i] &= ~1;
                            pc = m_regfile[i];
                        }
                        reg_rn += 4;
                        m_reglist &= ~(1 << i);
                    }               
                }

                m_regfile[m_rd] = reg_rn;
                assert(m_rd != REG_PC);
            }
            break;
            // LDR - LDR <Rt>, [<Rn>{,#<imm5>}]
            // 0 1 1 0 1 imm5 Rn Rt
            case INST_LDR_OPCODE:
            {
                m_regfile[m_rt] = read32(reg_rn + (m_imm << 2));
                assert(m_rd != REG_PC);
            }
            break;
            // LDR - LDR <Rt>,[SP{,#<imm8>}]
            // 1 0 0 1 1 Rt imm8
            case INST_LDR_1_OPCODE:
            {
                m_regfile[m_rt] = read32(reg_rn + (m_imm << 2));
                assert(m_rd != REG_PC);
            }
            break;
            // LDR - LDR <Rt>,<label>
            // 0 1 0 0 1 Rt imm8
            case INST_LDR_2_OPCODE:
            {
                m_regfile[m_rt] = read32((m_regfile[REG_PC] & 0xFFFFFFFC) + (m_imm << 2) + 4);
                assert(m_rd != REG_PC);
            }
            break;
            // LDRB - LDRB <Rt>,[<Rn>{,#<imm5>}]
            // 0 1 1 1 1 imm5 Rn Rt
            case INST_LDRB_OPCODE:
            {
                m_regfile[m_rt] = read(reg_rn + m_imm);
            }
            break;
            // LDRH - LDRH <Rt>,[<Rn>{,#<imm5>}]
            // 1 0 0 0 1 imm5 Rn Rt
            case INST_LDRH_OPCODE:
            {
                m_regfile[m_rt] = read16(reg_rn + (m_imm << 1));
            }
            break;
            // LSLS - LSLS <Rd>,<Rm>,#<imm5>
            // 0 0 0 0 0 imm5 Rm Rd
            // MOVS - MOVS <Rd>,<Rm>
            // 0 0 0 0 0 0 0 0 0 0 Rm Rd
            //case INST_MOVS_1_OPCODE:
            case INST_LSLS_OPCODE:
            {
                // MOVS <Rd>,<Rm>
                if (m_imm == 0)
                {
                    reg_rd = reg_rm;
                    write_rd = 1;

                    // Update N & Z
                    armv6m_update_n_z_flags(reg_rd);
                }
                // LSLS <Rd>,<Rm>,#<imm5>
                else
                {
                    reg_rd = armv6m_shift_left(reg_rm, m_imm, FLAGS_NZC);
                    write_rd = 1;
                }
            }
            break;
            // LSRS - LSRS <Rd>,<Rm>,#<imm5>
            // 0 0 0 0 1 imm5 Rm Rd
            case INST_LSRS_OPCODE:
            {
                if (m_imm == 0)
                    m_imm = 32;

                reg_rd = armv6m_shift_right(reg_rm, m_imm, FLAGS_NZC);
                write_rd = 1;
            }
            break;
            // MOVS - MOVS <Rd>,#<imm8>
            // 0 0 1 0 0 Rd imm8
            case INST_MOVS_OPCODE:
            {
                reg_rd = m_imm;
                write_rd = 1;

                armv6m_update_n_z_flags(reg_rd);
            }
            break;
            // STM - STM <Rn>!,<registers>
            // 1 1 0 0 0 Rn register_list
            case INST_STM_OPCODE:
            {
                int i;
                uint32_t addr = reg_rn;

                for (i=0;i<REGISTERS && m_reglist != 0;i++)
                {
                    if (m_reglist & (1 << i))
                    {
                        write32(addr, m_regfile[i]);
                        addr+=4;
                        m_reglist &= ~(1 << i);
                    }               
                }

                reg_rd = addr;
                write_rd = 1;
            }
            break;
            // STR - STR <Rt>, [<Rn>{,#<imm5>}]
            // 0 1 1 0 0 imm5 Rn Rt
            case INST_STR_OPCODE:
            {
                write32(reg_rn + (m_imm << 2), m_regfile[m_rt]);
            }
            break;
            // STR - STR <Rt>,[SP,#<imm8>]
            // 1 0 0 1 0 Rt imm8
            case INST_STR_1_OPCODE:
            {
                write32(reg_rn + (m_imm << 2), m_regfile[m_rt]);
            }
            break;
            // STRB - STRB <Rt>,[<Rn>,#<imm5>]
            // 0 1 1 1 0 imm5 Rn Rt
            case INST_STRB_OPCODE:
            {
                write(reg_rn + m_imm, m_regfile[m_rt]);
            }
            break;
            // STRH - STRH <Rt>,[<Rn>{,#<imm5>}]
            // 1 0 0 0 0 imm5 Rn Rt
            case INST_STRH_OPCODE:
            {
                write16(reg_rn + (m_imm << 1), m_regfile[m_rt]);
            }
            break;
            // SUBS - SUBS <Rdn>,#<imm8>
            // 0 0 1 11 Rdn imm8
            case INST_SUBS_1_OPCODE:
            {
                reg_rd = armv6m_add_with_carry(reg_rn, ~m_imm, 1, ALL_FLAGS);
                write_rd = 1;
            }
            break;
        }
    }
    break;
    case INST_IGRP2:
    {
        switch(inst & INST_IGRP2_MASK)
        {
            // ADDS - ADDS <Rd>,<Rn>,#<imm3>
            // 0 0 0 1 1 1 0 imm3 Rn Rd
            case INST_ADDS_OPCODE:
            {
                reg_rd = armv6m_add_with_carry(reg_rn, m_imm, 0, ALL_FLAGS);
                write_rd = 1;
            }
            break;
            // ADDS - ADDS <Rd>,<Rn>,<Rm>
            // 0 0 0 1 1 0 0 Rm Rn Rd
            case INST_ADDS_2_OPCODE:
            {
                reg_rd = armv6m_add_with_carry(reg_rn, reg_rm, 0, ALL_FLAGS);
                write_rd = 1;
            }
            break;
            // LDR - LDR <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 1 0 0 Rm Rn Rt
            case INST_LDR_3_OPCODE:
            {
                m_regfile[m_rt] = read32(reg_rn + reg_rm);
                assert(m_rt != REG_PC);
            }
            break;
            // LDRB - LDRB <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 1 1 0 Rm Rn Rt
            case INST_LDRB_1_OPCODE:
            {
                m_regfile[m_rt] = read(reg_rn + reg_rm);
            }
            break;
            // LDRH - LDRH <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 1 0 1 Rm Rn Rt
            case INST_LDRH_1_OPCODE:
            {
                m_regfile[m_rt] = read16(reg_rn + reg_rm);
            }
            break;
            // LDRSB - LDRSB <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 0 1 1 Rm Rn Rt
            case INST_LDRSB_OPCODE:
            {
                reg_rd = read(reg_rn + reg_rm);
                m_regfile[m_rt] = armv6m_sign_extend(reg_rd, 8);
            }
            break;
            // LDRSH - LDRSH <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 1 1 1 Rm Rn Rt
            case INST_LDRSH_OPCODE:
            {
                reg_rd = read16(reg_rn + reg_rm);
                m_regfile[m_rt] = armv6m_sign_extend(reg_rd, 16);
            }
            break;
            // POP - POP <registers>
            // 1 0 1 1 1 1 0 P register_list
            case INST_POP_OPCODE:
            {
                int i;
                uint32_t sp = m_regfile[REG_SP];
                
                for (i=0;i<REGISTERS && m_reglist != 0;i++)
                {
                    if (m_reglist & (1 << i))
                    {                       
                        m_regfile[i] = read32(sp);
                        DPRINTF(LOG_PUSHPOP, ("STACK: POP R%d (%x) from %x\n",i,m_regfile[i], sp));

                        sp+=4;

                        if (i == REG_PC)
                        {
                            if ((m_regfile[i] & EXC_RETURN) != EXC_RETURN)
                                m_regfile[i] &= ~1;
                            pc = m_regfile[i];
                        }
                        
                        m_reglist &= ~(1 << i);
                    }               
                }

                armv6m_update_sp(sp);
            }
            break;
            // PUSH - PUSH <registers>
            // 1 0 1 1 0 1 0 M register_list
            case INST_PUSH_OPCODE:
            {
                int i;
                uint32_t sp = m_regfile[REG_SP];
                uint32_t addr = sp;
                int bits_set = 0;

                for (i=0;i<REGISTERS;i++)
                    if (m_reglist & (1 << i))
                        bits_set++;

                addr -= (4 * bits_set);

                for (i=0;i<REGISTERS && m_reglist != 0;i++)
                {
                    if (m_reglist & (1 << i))
                    {
                        DPRINTF(LOG_PUSHPOP, ("STACK: PUSH R%d (%x) to %x\n",i,m_regfile[i], addr));
                        write32(addr, m_regfile[i]);
                        sp-=4;
                        addr+=4;
                        m_reglist &= ~(1 << i);
                    }               
                }

                armv6m_update_sp(sp);
            }
            break;
            // STR - STR <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 0 00 Rm Rn Rt
            case INST_STR_2_OPCODE:
            {
                write32(reg_rn + reg_rm, m_regfile[m_rt]);
            }
            break;
            // STRB - STRB <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 0 1 0 Rm Rn Rt
            case INST_STRB_1_OPCODE:
            {
                write(reg_rn + reg_rm, m_regfile[m_rt]);
            }
            break;
            // STRH - STRH <Rt>,[<Rn>,<Rm>]
            // 0 1 0 1 0 0 1 Rm Rn Rt
            case INST_STRH_1_OPCODE:
            {
                write16(reg_rn + reg_rm, m_regfile[m_rt]);
            }
            break;
            // SUBS - SUBS <Rd>,<Rn>,#<imm3>
            // 0 0 0 11 1 1 imm3 Rn Rd
            case INST_SUBS_OPCODE:
            {
                reg_rd = armv6m_add_with_carry(reg_rn, ~m_imm, 1, ALL_FLAGS);
                write_rd = 1;
            }
            break;
            // SUBS - SUBS <Rd>,<Rn>,<Rm>
            // 0 0 0 11 0 1 Rm Rn Rd
            case INST_SUBS_2_OPCODE:
            {
                reg_rd = armv6m_add_with_carry(reg_rn, ~reg_rm, 1, ALL_FLAGS);
                write_rd = 1;
            }
            break;
        }
    }
    break;
    case INST_IGRP3:
    {
        switch(inst & INST_IGRP3_MASK)
        {
            // ADD - ADD <Rdn>,<Rm>
            // 0 1 0 0 0 1 0 0 Rm Rdn
            case INST_ADD_OPCODE:
            {
                reg_rd = reg_rn + reg_rm;
                write_rd = 1;
            }
            break;
            // BKPT - BKPT #<imm8>
            // 1 0 1 1 1 1 1 0 imm8
            case INST_BKPT_OPCODE:
            {
                // Instruction used for program exit
                printf("Exit code = %d\n", m_imm);
                // Abnormal exit
                if (m_imm)
                    exit(m_imm);
                else
                    m_stopped = true;
            }
            break;
            // CMP - CMP <Rn>,<Rm> <Rn> and <Rm> not both from R0-R7
            // 0 1 0 0 0 1 0 1 N Rm Rn
            case INST_CMP_2_OPCODE:
            {
                reg_rd = armv6m_add_with_carry(reg_rn, ~reg_rm, 1, ALL_FLAGS);
            }
            break;
            // MOV - MOV <Rd>,<Rm> Otherwise all versions of the Thumb instruction set.
            // 0 1 0 0 0 1 1 0 D Rm Rd
            case INST_MOV_OPCODE:
            {
                // Write to PC
                if (m_rd == REG_PC)
                {
                    pc = reg_rm & ~1;

                    // Don't do normal writeback
                    write_rd = 0;
                }
                // Normal register
                else
                {
                    reg_rd = reg_rm;
                    write_rd = 1;
                }
            }
            break;
            // SVC - SVC #<imm8>
            // 1 1 0 1 111 1 imm8
            case INST_SVC_OPCODE:
            {
                pc = armv6m_exception(pc, 11);
            }
            break;
            // UDF - UDF #<imm8>
            // 1 1 0 1 1 1 1 0 imm8
            case INST_UDF_OPCODE:
            {
                assert(!"Not implemented");
            }
            break;
        }
    }
    break;
    case INST_IGRP4:
    {
        switch(inst & INST_IGRP4_MASK)
        {
            // ADD - ADD SP,SP,#<imm7>
            // 1 0 1 1 0 0 0 0 0 imm7
            case INST_ADD_2_OPCODE:
            {
                reg_rd = reg_rn + (m_imm << 2);
                write_rd = 1;
            }
            break;
            // BLX - BLX <Rm>
            // 0 1 0 0 0 1 1 1 1 Rm (0) (0) (0)
            case INST_BLX_OPCODE:
            {
                // m_rd = REG_LR
                reg_rd = pc | 1;
                write_rd = 1;

                pc = reg_rm & ~1;
            }
            break;
            // BX - BX <Rm>
            // 0 1 0 0 0 1 1 1 0 Rm (0) (0) (0)
            case INST_BX_OPCODE:
            {
                pc = reg_rm & ~1;
            }
            break;
            // SUB - SUB SP,SP,#<imm7>
            // 1 0 1 1 000 0 1 imm7
            case INST_SUB_OPCODE:
            {
                reg_rd = reg_rn - (m_imm << 2);
                write_rd = 1;
            }
            break;
        }
    }
    break;
    case INST_IGRP5:
    {
        switch(inst & INST_IGRP5_MASK)
        {
            // ADCS - ADCS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 1 0 1 Rm Rdn
            case INST_ADCS_OPCODE:
            {       
                reg_rd = armv6m_add_with_carry(reg_rn, reg_rm, (m_apsr & APSR_C) ? 1 : 0, ALL_FLAGS);
                write_rd = 1;
            }
            break;
            // ANDS - ANDS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 0 0 0 Rm Rdn
            case INST_ANDS_OPCODE:
            {
                reg_rd = reg_rn & reg_rm;
                write_rd = 1;

                armv6m_update_n_z_flags(reg_rd);
            }
            break;
            // ASRS - ASRS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 1 0 0 Rm Rdn
            case INST_ASRS_1_OPCODE:
            {
                reg_rd = armv6m_arith_shift_right(reg_rn, reg_rm, FLAGS_NZC);
                write_rd = 1;
            }
            break;
            // BICS - BICS <Rdn>,<Rm>
            // 0 1 0 0 0 0 1 1 1 0 Rm Rdn
            case INST_BICS_OPCODE:
            {
                reg_rd = reg_rn & (~reg_rm);
                write_rd = 1;

                armv6m_update_n_z_flags(reg_rd);
            }
            break;
            // CMN - CMN <Rn>,<Rm>
            // 0 1 0 0 0 0 1 0 1 1 Rm Rn
            case INST_CMN_OPCODE:
            {
                reg_rd = armv6m_add_with_carry(reg_rn, reg_rm, 0, ALL_FLAGS);
            }
            break;
            // CMP - CMP <Rn>,<Rm> <Rn> and <Rm> both from R0-R7
            // 0 1 0 0 0 0 1 0 1 0 Rm Rn
            case INST_CMP_1_OPCODE:
            {
                reg_rd = armv6m_add_with_carry(reg_rn, ~reg_rm, 1, ALL_FLAGS);
            }
            break;
            // EORS - EORS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 0 0 1 Rm Rdn
            case INST_EORS_OPCODE:
            {
                reg_rd = reg_rn ^ reg_rm;
                write_rd = 1;

                armv6m_update_n_z_flags(reg_rd);
            }
            break;
            // LSLS - LSLS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 0 1 0 Rm Rdn
            case INST_LSLS_1_OPCODE:
            {
                if (reg_rm == 0)
                {
                    reg_rd   = reg_rn;
                    write_rd = 1;

                    // Update N & Z
                    armv6m_update_n_z_flags(reg_rd);
                }
                else
                {
                    reg_rd = armv6m_shift_left(reg_rn, reg_rm, FLAGS_NZC);
                    write_rd = 1;
                }
            }
            break;
            // LSRS - LSRS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 0 1 1 Rm Rdn
            case INST_LSRS_1_OPCODE:
            {
                reg_rd = armv6m_shift_right(reg_rn, reg_rm & 0xFF, FLAGS_NZC);
                write_rd = 1;
            }
            break;
            // MULS - MULS <Rdm>,<Rn>,<Rdm>
            // 0 1 0 0 0 0 1 1 0 1 Rn Rdm
            case INST_MULS_OPCODE:
            {
                reg_rd = reg_rn * reg_rm;
                write_rd = 1;

                armv6m_update_n_z_flags(reg_rd);
            }
            break;
            // MVNS - MVNS <Rd>,<Rm>
            // 0 1 0 0 0 0 1 1 1 1 Rm Rd
            case INST_MVNS_OPCODE:
            {
                reg_rd = ~reg_rm;
                write_rd = 1;

                armv6m_update_n_z_flags(reg_rd);
            }
            break;
            // ORRS - ORRS <Rdn>,<Rm>
            // 0 1 0 0 0 0 1 1 0 0 Rm Rdn
            case INST_ORRS_OPCODE:
            {
                reg_rd = reg_rn | reg_rm;
                write_rd = 1;

                armv6m_update_n_z_flags(reg_rd);
            }
            break;
            // REV - REV <Rd>,<Rm>
            // 1 0 1 1 1 0 1 0 0 0 Rm Rd
            case INST_REV_OPCODE:
            {
                reg_rd =((reg_rm>> 0)&0xFF)<<24;
                reg_rd|=((reg_rm>> 8)&0xFF)<<16;
                reg_rd|=((reg_rm>>16)&0xFF)<< 8;
                reg_rd|=((reg_rm>>24)&0xFF)<< 0;
                write_rd = 1;
            }
            break;
            // REV16 - REV16 <Rd>,<Rm>
            // 1 0 1 1 1 0 1 0 0 1 Rm Rd
            case INST_REV16_OPCODE:
            {
                reg_rd =((reg_rm>> 0)&0xFF)<< 8;
                reg_rd|=((reg_rm>> 8)&0xFF)<< 0;
                reg_rd|=((reg_rm>>16)&0xFF)<<24;
                reg_rd|=((reg_rm>>24)&0xFF)<<16;
                write_rd = 1;
            }
            break;
            // REVSH - REVSH <Rd>,<Rm>
            // 1 0 1 1 1 0 1 0 1 1 Rm Rd
            case INST_REVSH_OPCODE:
            {
                reg_rd =((reg_rm>> 0)&0xFF)<< 8;
                reg_rd|=((reg_rm>> 8)&0xFF)<< 0;
                reg_rd = armv6m_sign_extend(reg_rd, 16);
                write_rd = 1;
            }
            break;
            // RORS - RORS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 1 1 1 Rm Rdn
            case INST_RORS_OPCODE:
            {
                reg_rd = armv6m_rotate_right(reg_rn, reg_rm & 0xFF, FLAGS_NZC);
                write_rd = 1;
            }
            break;
            // RSBS - RSBS <Rd>,<Rn>,#0
            // 0 1 0 0 0 0 1 0 0 1 Rn Rd
            case INST_RSBS_OPCODE:
            {
                reg_rd = armv6m_add_with_carry(~reg_rn, 0, 1, ALL_FLAGS);

                write_rd = 1;
            }
            break;
            // SBCS - SBCS <Rdn>,<Rm>
            // 0 1 0 0 0 0 0 1 1 0 Rm Rdn
            case INST_SBCS_OPCODE:
            {
                reg_rd = armv6m_add_with_carry(reg_rn, ~reg_rm, (m_apsr & APSR_C) ? 1 : 0, ALL_FLAGS);
                write_rd = 1;
            }
            break;
            // SXTB - SXTB <Rd>,<Rm>
            // 1 0 1 1 100 0 0 1 Rm Rd
            case INST_SXTB_OPCODE:
            {
                reg_rd = reg_rm & 0xFF;
                if(reg_rd & 0x80) 
                    reg_rd|=(~0)<<8;
                write_rd = 1;
            }
            break;
            // SXTH - SXTH <Rd>,<Rm>
            // 1 0 1 1 100 0 0 0 Rm Rd
            case INST_SXTH_OPCODE:
            {
                reg_rd = reg_rm & 0xFFFF;
                if(reg_rd & 0x8000) 
                    reg_rd|=(~0)<<16;
                write_rd = 1;
            }
            break;
            // TST - TST <Rn>,<Rm>
            // 000 1 0 0 1 0 0 0 Rm Rn
            case INST_TST_OPCODE:
            {
                reg_rd = reg_rn & reg_rm;
                // No writeback
                armv6m_update_n_z_flags(reg_rd);
            }
            break;
            // UXTB - UXTB <Rd>,<Rm>
            // 1 0 1 1 100 0 1 1 Rm Rd
            case INST_UXTB_OPCODE:
            {
                reg_rd = reg_rm & 0xFF;
                write_rd = 1;
            }
            break;
            // UXTH - UXTH <Rd>,<Rm>
            // 1 0 1 1 100 0 1 0 Rm Rd
            case INST_UXTH_OPCODE:
            {
                reg_rd = reg_rm & 0xFFFF;
                write_rd = 1;
            }
            break;
        }
    }
    break;
    case INST_IGRP6:
    {
        switch(inst & INST_IGRP6_MASK)
        {
            // MRS - MRS <Rd>,<spec_reg>
            // 1 1 1 01 0 1 1 1 1 1 (0) (1) (1) (1) (1) 1 0 (0) 0 Rd SYSm
            case INST_MRS_OPCODE:
            {
                uint32_t sysm = (inst2>>0) & 0xFF;
                m_rd = (inst2>>8) & 0xF;              

                // Increment PC past second instruction word
                pc += 2;

                switch ((sysm >> 3) & 0x1F)
                {
                case 0:
                {
                    uint32_t val = 0;

                    if (sysm & 0x1)
                        val |= m_ipsr&0x1FF;

                    if (!(sysm & 0x4))
                        val |= (m_apsr & 0xF8000000);

                    val|= m_ipsr;
                    val|= m_epsr;

                    reg_rd = val;
                    write_rd = 1;
                }
                break;
                case 1:
                {
                    switch (sysm & 0x7)
                    {
                    case 0:
                        // Main SP
                        reg_rd = m_msp;
                        write_rd = 1;
                        break;
                    case 1:
                        // Process SP
                        reg_rd = m_psp;
                        write_rd = 1;
                        break;
                    }           
                }
                break;
                case 2:
                {
                    switch (sysm & 0x7)
                    {
                    case 0:
                        // PRIMASK.PM
                        reg_rd = m_primask & PRIMASK_PM;
                        write_rd = 1;
                        break;
                    case 4:
                        // Control<1:0>
                        reg_rd = m_control & CONTROL_MASK;
                        write_rd = 1;
                        break;
                    }           
                }
                break;
                }

            }
            break;
            // MSR - MSR <spec_reg>,<Rn>
            // 1 1 1 01 0 1 1 1 0 0 (0) Rn 1 0 (0) 0 (1) (0) (0) (0) SYSm
            case INST_MSR_OPCODE:
            {
                uint32_t sysm = (inst2 >> 0) & 0xFF;

                // Increment PC past second instruction word
                pc += 2;

                switch ((sysm >> 3) & 0x1F)
                {
                case 0:
                {
                    if (!(sysm & 0x4))
                        m_apsr = reg_rn & 0xF8000000;
                }
                break;
                case 1:
                {
                    // TODO: Only if priviledged...
                    switch (sysm & 0x7)
                    {
                    case 0:
                        // Main SP
                        m_msp = reg_rn;
                        break;
                    case 1:
                        // Process SP
                        m_psp = reg_rn;
                        break;
                    }           
                }
                break;
                case 2:
                {
                    // TODO: Only if priviledged...
                    switch (sysm&0x7)
                    {
                    case 0:
                        // PRIMASK.PM
                        m_primask = reg_rn & PRIMASK_PM;
                        break;
                    case 4:
                        // Control<1:0>
                        if (m_current_mode == MODE_THREAD)
                        {
                            m_control = reg_rn & CONTROL_MASK;

                            // Allow switching of current SP
                            //if (m_control & CONTROL_SPSEL)
                            //  spsel = SP_MSP;
                            //else
                            //  spsel = SP_PSP;
                        }
                        break;
                    }           
                }
                break;
                }
            }
            break;
            // CPS - CPS<effect> i
            // 1 0 1 1 0 1 1 0 0 1 1 im (0) (0) (1) (0)
            case INST_CPS_OPCODE:
            {       
                // TODO: Only if priviledged...

                // Enable
                if (m_imm == 0)
                    m_primask&= ~PRIMASK_PM;
                // Disable
                else
                    m_primask|= PRIMASK_PM;
            }
            break;
        }
    }
    break;
    case INST_IGRP7:
    {
        switch(inst & INST_IGRP7_MASK)
        {
            // DMB - DMB #<option>
            // 1 1 1 01 0 1 1 1 0 1 1 (1) (1) (1) (1) 1 0 (0) 0 (1) (1) (1) (1) 0 1 0 1 option
            //case INST_DMB_OPCODE:
            // DSB - DSB #<option>
            // 1 1 1 01 0 1 1 1 0 1 1 (1) (1) (1) (1) 1 0 (0) 0 (1) (1) (1) (1) 0 1 0 0 option
            //case INST_DSB_OPCODE:
            // ISB - ISB #<option>
            // 1 1 1 01 0 1 1 1 0 1 1 (1) (1) (1) (1) 1 0 (0) 0 (1) (1) (1) (1) 0 1 1 0 option
            case INST_ISB_OPCODE:
            {
                // Increment PC past second instruction word
                pc += 2;
            }
            break;
            // UDF_W - UDF_W #<imm16>
            // 1 11 1 0 1 1 1 1 1 1 1 imm4 1 0 1 0 imm12
            case INST_UDF_W_OPCODE:
            {
                // Increment PC past second instruction word
                pc += 2;
            }
            break;
        }
    }
    break;
    case INST_IGRP8:
    {
        switch(inst & INST_IGRP8_MASK)
        {
            // NOP - NOP
            // 1 0 1 1 1 1 1 1 0 0 0 0 0 0 0 0
            case INST_NOP_OPCODE:
            {
                // Not implemented
            }
            break;
            // SEV - SEV
            // 1 0 1 1 1 1 1 1 0 1 0 0 0 0 0 0
            case INST_SEV_OPCODE:
            {
                // Not implemented
            }
            break;
            // WFE - WFE
            // 1 0 1 1 1 1 1 1 0 0 1 0 0 0 0 0
            case INST_WFE_OPCODE:
            {
                // Not implemented
            }
            break;
            // WFI - WFI
            // 1 0 1 1 1 1 1 1 0 0 1 1 0 0 0 0
            case INST_WFI_OPCODE:
            {
                assert(!"Not implemented");
            }
            break;
            // YIELD - YIELD
            // 1 0 1 1 1 1 1 1 0 0 0 1 0 0 0 0
            case INST_YIELD_OPCODE:
            {
                assert(!"Not implemented");
            }
            break;
        }
    }
    break;
    }

    if (write_rd)
    {
        if (m_rd == REG_SP)
            armv6m_update_sp(reg_rd);
        else
            m_regfile[m_rd] = reg_rd;
    }

    // Can't perform a writeback to PC using normal mechanism as 
    // this is a special register...
    if (write_rd)
    {
        assert(m_rd != REG_PC);
    }

    m_regfile[REG_PC] = pc;
}