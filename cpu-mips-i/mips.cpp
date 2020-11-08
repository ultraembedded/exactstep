//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include "mips.h"
#include "mips_isa.h"

//-----------------------------------------------------------------
// Defines:
//-----------------------------------------------------------------
#define LOG_INST            (1 << 0)
#define LOG_OPCODES         (1 << 1)
#define LOG_REGISTERS       (1 << 2)
#define LOG_MEM             (1 << 3)
#define LOG_MMU             (1 << 4)

#define DPRINTF(l,a)        do { if (m_trace & l) printf a; } while (0)
#define TRACE_ENABLED(l)    (m_trace & l)
#define INST_STAT(l)

#define SET_LOAD_DELAY_SLOT(_reg)

#define ARITH_OVERFLOW(_op1, _op2, _res) ((((_op1) & 0x80000000) == ((_op2) & 0x80000000)) && \
                                          (((_op1) & 0x80000000) != ((_res) & 0x80000000)))

//-----------------------------------------------------------------
// Tables:
//-----------------------------------------------------------------
enum eMipRegisters
{
    MIPS_REG_ZERO,
    MIPS_REG_AT,
    MIPS_REG_V0,
    MIPS_REG_V1,
    MIPS_REG_A0,
    MIPS_REG_A1,
    MIPS_REG_A2,
    MIPS_REG_A3,
    MIPS_REG_T0,
    MIPS_REG_T1,
    MIPS_REG_T2,
    MIPS_REG_T3,
    MIPS_REG_T4,
    MIPS_REG_T5,
    MIPS_REG_T6,
    MIPS_REG_T7,
    MIPS_REG_S0,
    MIPS_REG_S1,
    MIPS_REG_S2,
    MIPS_REG_S3,
    MIPS_REG_S4,
    MIPS_REG_S5,
    MIPS_REG_S6,
    MIPS_REG_S7,
    MIPS_REG_T8,
    MIPS_REG_T9,
    MIPS_REG_K0,
    MIPS_REG_K1,
    MIPS_REG_GP,
    MIPS_REG_SP,
    MIPS_REG_FP,
    MIPS_REG_RA,
    MIPS_REG_GPR_MAX = MIPS_REG_RA,

    // Special
    MIPS_REG_SR,
    MIPS_REG_LO,
    MIPS_REG_HI,
    MIPS_REG_BAD,
    MIPS_REG_CAUSE,
    MIPS_REG_PC,
    MIPS_REG_MAX
};

static const char * reg_name[] = 
{
    "zero", "at", "v0", "v1", "a0", "a1",   
    "a2",   "a3", "t0", "t1", "t2", "t3",   
    "t4",   "t5", "t6", "t7", "s0", "s1",   
    "s2",   "s3", "s4", "s5", "s6", "s7",   
    "t8",   "t9", "k0", "k1", "gp", "sp",   
    "fp",   "ra", 0
};

//-----------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------
mips_i::mips_i(uint32_t baseAddr /*= 0*/, uint32_t len /*= 0*/): cpu()
{
    m_enable_mem_errors  = true;

    // Some memory defined
    if (len != 0)
        create_memory(baseAddr, len);

    reset(baseAddr);
}
//-----------------------------------------------------------------
// set_register: Write register
//-----------------------------------------------------------------
void mips_i::set_register(int reg, uint32_t val)
{
    if (reg <= MIPS_REG_GPR_MAX)
        m_gpr[reg] = val;
    else switch (reg)
    {
        case MIPS_REG_SR:
            m_status = val;
            break;
        case MIPS_REG_LO:
            m_lo = val;
            break;
        case MIPS_REG_HI:
            m_hi = val;
            break;
        case MIPS_REG_BAD:
            break;
        case MIPS_REG_CAUSE:
            m_cause = val;
            break;
        case MIPS_REG_PC:
            m_pc        = val;
            m_pc_next   = val;
            m_pc_x      = val;
            break;
    }
}

//-----------------------------------------------------------------
// get_register: Read register
//-----------------------------------------------------------------
uint32_t mips_i::get_register(int reg)
{
    if (reg <= MIPS_REG_GPR_MAX)
        return m_gpr[reg];
    else switch (reg)
    {
        case MIPS_REG_SR:
            return m_status;
        case MIPS_REG_LO:
            return m_lo;
        case MIPS_REG_HI:
            return m_hi;
        case MIPS_REG_CAUSE:
            return m_cause;
        case MIPS_REG_PC:
            return m_pc;
    }

    return 0;
}
//-----------------------------------------------------------------
// reset: Reset CPU state
//-----------------------------------------------------------------
void mips_i::reset(uint32_t start_addr)
{
    uint32_t isr_vector = start_addr;

    for (int i=0;i<REGISTERS;i++)
        m_gpr[i] = 0;

    m_epc       = 0;
    m_status    = 0;
    m_cause     = 0;
    m_hi        = 0;
    m_lo        = 0;
    m_isr_vector= isr_vector;
    m_branch_ds = false;
    m_take_excpn= false;
    m_cycles    = 0;

    m_fault     = false;
    m_break     = false;
    m_trace     = 0;

    m_pc        = start_addr;
    m_pc_next   = start_addr;
    m_pc_x      = start_addr;

    stats_reset();
}
//-----------------------------------------------------------------
// get_opcode: Get instruction from address
//-----------------------------------------------------------------
uint32_t mips_i::get_opcode(uint32_t pc)
{
    return read32(pc);
}
//-----------------------------------------------------------------
// load: Perform a load operation
//-----------------------------------------------------------------
int mips_i::load(uint32_t pc, uint32_t address, uint32_t *result, int width, bool signedLoad)
{
    uint32_t physical = address;

    DPRINTF(LOG_MEM, ("LOAD: VA 0x%08x PA 0x%08x Width %d\n", address, physical, width));

    // Detect misaligned load
    if (((address & 3) != 0 && width == 4) || ((address & 1) != 0 && width == 2))
    {
        exception(EXC_ADEL, pc, address);
        return 0;
    }

    m_stats[STATS_LOADS]++;
    *result = 0;

    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(physical))
        {
            switch (width)
            {
                case 4:
                    mem->read32(physical, *result);
                    break;
                case 2:
                {
                    uint16_t dh = 0;
                    mem->read16(physical, dh);
                    *result |= dh;

                    if (signedLoad && ((*result) & (1 << 15)))
                         *result |= 0xFFFF0000;
                }
                break;
                case 1:
                {
                    uint8_t db = 0;
                    mem->read8(physical + 0, db);
                    *result |= ((uint32_t)db << 0);

                    if (signedLoad && ((*result) & (1 << 7)))
                         *result |= 0xFFFFFF00;
                }
                break;
                default:
                    assert(!"Invalid");
                    break;
            }

            DPRINTF(LOG_MEM, ("LOAD_RESULT: 0x%08x\n",*result));
            return 1;
        }

    if (m_enable_mem_errors)
    {
        exception(EXC_DBE, pc, address);
        return 0;
    }

    error(false, "%08x: Bad memory access 0x%x\n", pc, address);
    return 0;
}
//-----------------------------------------------------------------
// store: Perform a store operation
//-----------------------------------------------------------------
int mips_i::store(uint32_t pc, uint32_t address, uint32_t data, int width, uint8_t mask)
{
    uint32_t physical = address;

    DPRINTF(LOG_MEM, ("STORE: VA 0x%08x PA 0x%08x Value 0x%08x Mask %x\n", address, physical, data, mask));

    // Detect misaligned store
    if (((address & 3) != 0 && (mask == 0xF)) || ((address & 1) != 0 && (mask == 0xc || mask == 0x3)))
    {
        exception(EXC_ADES, pc, address);
        return 0;
    }

    m_stats[STATS_STORES]++;

    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(physical))
        {
            switch (width)
            {
                case 4:
                {
                    switch (mask)
                    {
                        case 0xF:
                            mem->write32(physical, data);
                            break;
                        // SWR/SWL patterns
                        case 0x7:
                            mem->write8(physical + 0, data >> 0);
                            mem->write8(physical + 1, data >> 8);
                            mem->write8(physical + 2, data >> 16);
                            break;
                        case 0xe:
                            mem->write8(physical + 1, data >> 8);
                            mem->write8(physical + 2, data >> 16);
                            mem->write8(physical + 3, data >> 24);
                            break;
                        case 0xc:
                            mem->write8(physical + 2, data >> 16);
                            mem->write8(physical + 3, data >> 24);
                            break;
                        case 0x3:
                            mem->write8(physical + 0, data >> 0);
                            mem->write8(physical + 1, data >> 8);
                            break;
                        case 0x8:
                            mem->write8(physical + 3, data >> 24);
                            break;
                        case 0x4:
                            mem->write8(physical + 2, data >> 16);
                            break;
                        case 0x2:
                            mem->write8(physical + 1, data >> 8);
                            break;
                        case 0x1:
                            mem->write8(physical + 0, data >> 0);
                            break;
                        default:
                            assert(!"Invalid");
                            break;
                    }
                }
                break;
                case 2:
                    mem->write16(physical, data & 0xFFFF);
                    break;
                case 1:
                    mem->write8(physical, data & 0xFF);
                    break;
                default:
                    assert(!"Invalid");
                    break;
            }        
            return 1;
        }

    if (m_enable_mem_errors)
    {
        exception(EXC_DBE, pc, address);
        return 0;
    }

    error(false, "%08x: Bad memory access 0x%x\n", pc, address);
    return 0;
}
//-----------------------------------------------------------------
// copro0_inst: Coprocessor0 instruction
//-----------------------------------------------------------------
int mips_i::copro0_inst(uint32_t pc, uint32_t opc, uint32_t reg_rs, uint32_t reg_rt, int &wb_reg, uint32_t &result)
{
    uint32_t rs     = (opc >> OPCODE_RS_SHIFT)   & OPCODE_RS_MASK;
    uint32_t rt     = (opc >> OPCODE_RT_SHIFT)   & OPCODE_RT_MASK;
    uint32_t rd     = (opc >> OPCODE_RD_SHIFT)   & OPCODE_RD_MASK;

    // RFE
    if (rs == COP0_RFE)
    {
        DPRINTF(LOG_INST,("%08x: RFE\n",  m_pc));

        // STATUS: Interrupt enable stack pop
        m_status = SR_BF_SET(m_status, IEC, SR_BF_GET(m_status, IEP));
        m_status = SR_BF_SET(m_status, IEP, SR_BF_GET(m_status, IEO));

        // STATUS: User mode stack pop
        m_status = SR_BF_SET(m_status, KUC, SR_BF_GET(m_status, KUP));
        m_status = SR_BF_SET(m_status, KUP, SR_BF_GET(m_status, KUO));
    }
    // Move from CP0
    else if (rs == COP0_MFC0)
    {
        DPRINTF(LOG_INST,("%08x: MFC0 $%d, %d\n",  m_pc, rt, rd));
        switch (rd)
        {
        case COP0_STATUS:
            result = m_status;
            wb_reg = rt;
            break;
        case COP0_CAUSE:
            result = m_cause;
            wb_reg = rt;
            break;
        case COP0_EPC:
            result = m_epc;
            wb_reg = rt;
            break;
        case COP0_BADADDR:
            result = m_badaddr;
            wb_reg = rt;
            break;
        case COP0_EBASE:
            result = m_isr_vector;
            wb_reg = rt;
            break;
        case COP0_COUNT: // Non-std?
            result = m_cycles;
            wb_reg = rt;
            break;
        default:
            fprintf (stderr,"Fault @ PC %x\n", m_pc);
            exception(EXC_RI, m_pc);
            return 0;
        }
    }
    // Move to CP0
    else if (rs == COP0_MTC0)
    {
        DPRINTF(LOG_INST,("%08x: MTC0 $%d, %d\n",  m_pc, rt, rd));
        // RD = target
        // rt = src reg num
        switch (rd)
        {
        case COP0_STATUS:
            m_status = reg_rt;
            break;
        case COP0_CAUSE:
            m_cause = reg_rt;
            break;
        case COP0_EPC:
            m_epc = reg_rt;
            break;
        case COP0_EBASE:
            m_isr_vector = reg_rt;
            break;
        default:
            fprintf (stderr,"Fault @ PC %x\n", m_pc);
            exception(EXC_RI, m_pc);
            return 0;
        }
    }
    else
    {
        fprintf (stderr,"Fault @ PC %x\n", m_pc);
        exception(EXC_RI, m_pc);
        return 0;
    }

    return 1;
}
//-----------------------------------------------------------------
// copro_inst: Coprocessor instruction (not COP0)
//-----------------------------------------------------------------
int mips_i::copro_inst(int cop, uint32_t pc, uint32_t opc, uint32_t reg_rs, uint32_t reg_rt, int &wb_reg, uint32_t &result)
{
    printf("COPRO%d: PC=%08x OPC=%08x RS=%08x RT=%08x\n", cop, pc, opc, reg_rs, reg_rt);
    result = 0;
    return 1;
}
//-----------------------------------------------------------------
// exception: Take exception
//-----------------------------------------------------------------
void mips_i::exception(uint32_t cause, uint32_t pc, uint32_t badaddr)
{
    // STATUS: Interrupt enable stack push
    m_status = SR_BF_SET(m_status, IEO, SR_BF_GET(m_status, IEP));
    m_status = SR_BF_SET(m_status, IEP, SR_BF_GET(m_status, IEC));
    m_status = SR_BF_SET(m_status, IEC, 0);

    // STATUS: User mode stack push
    m_status = SR_BF_SET(m_status, KUO, SR_BF_GET(m_status, KUP));
    m_status = SR_BF_SET(m_status, KUP, SR_BF_GET(m_status, KUC));
    m_status = SR_BF_SET(m_status, KUC, 0);

    // CAUSE: Set exception cause
    m_cause = CAUSE_BF_SET(m_cause, EXC, COP0_CAUSE_EXC_MASK, cause);

    // CAUSE: Record if this exception was in a branch delay slot
    m_cause = CAUSE_BF_SET(m_cause, BD, 1, m_branch_ds);

    // TODO: Stop on bad instruction for now...
    if (cause == EXC_RI)
        m_fault = true;
    else if (cause == EXC_BP)
    {
        m_break    = true;
        m_stopped  = true;
    }

    m_badaddr = badaddr;
    m_epc     = m_branch_ds ? pc - 4 : pc;
}
//-----------------------------------------------------------------
// Execute: Execute a single instruction
//-----------------------------------------------------------------
bool mips_i::execute(void)
{
    int take_branch = 0;
    int take_jmp    = 0;
    int take_excpn  = 0;
    int wb_reg      = 0;
    uint32_t opcode = 0;

    // Misaligned PC
    if (m_pc & 0x3)
    {
        exception(EXC_ADEL, m_pc);
        take_excpn = true;

        // NOP
        opcode = 0x00000000;
    }
    // Fetch instruction
    else
        opcode = get_opcode(m_pc);

    // Decode opcode
    uint32_t inst   = (opcode >> OPCODE_INST_SHIFT) & OPCODE_INST_MASK;
    uint32_t rs     = (opcode >> OPCODE_RS_SHIFT)   & OPCODE_RS_MASK;
    uint32_t rt     = (opcode >> OPCODE_RT_SHIFT)   & OPCODE_RT_MASK;
    uint32_t rd     = (opcode >> OPCODE_RD_SHIFT)   & OPCODE_RD_MASK;
    uint32_t re     = (opcode >> OPCODE_RE_SHIFT)   & OPCODE_RE_MASK;
    uint32_t func   = (opcode >> OPCODE_FUNC_SHIFT) & OPCODE_FUNC_MASK;
    uint32_t imm    = (opcode >> OPCODE_IMM_SHIFT)  & OPCODE_IMM_MASK;
    uint32_t target = (opcode >> OPCODE_ADDR_SHIFT) & OPCODE_ADDR_MASK;

    // Load source registers
    uint32_t reg_rs = m_gpr[rs];
    uint32_t reg_rt = m_gpr[rt];

    DPRINTF(LOG_OPCODES,( "%08x: %08x\n", m_pc, opcode));
    DPRINTF(LOG_OPCODES,( "        rd(%d) r%d = %d, r%d = %d\n", rd, rs, reg_rs, rt, reg_rt));

    // Stats
    if (opcode == 0x00000000)
        m_stats[STATS_NOP]++;

    // Signed & unsigned imm -> 32-bits
    uint32_t imm_int32  = (unsigned int)(signed short)imm;
    uint32_t imm_uint32 = imm;

    // Zero result
    uint32_t result = 0;

    // Update PC to next value
    uint32_t pc = m_pc_next;

    // Increment next PC value (might be overriden by branch)
    uint32_t pc_next = m_pc_next + 4;

    #define INST_TRACE_R3(inst)       DPRINTF(LOG_INST,("%08x: " inst " $%d, $%d, $%d\n",  m_pc, rd, rs, rt))
    #define INST_TRACE_I(inst)        DPRINTF(LOG_INST,("%08x: " inst " $%d, $%d, %d\n",   m_pc, rt, rs, imm))
    #define INST_TRACE_BIR(inst)      DPRINTF(LOG_INST,("%08x: " inst " $%d, $%d, 0x%x\n", m_pc, rs, rt, m_pc_next + (imm_int32 << 2)))
    #define INST_TRACE_BIZ(inst)      DPRINTF(LOG_INST,("%08x: " inst " $%d, 0, 0x%x\n",   m_pc, rs, m_pc_next + (imm_int32 << 2)))
    #define INST_TRACE_MULDIV(inst)   DPRINTF(LOG_INST,("%08x: " inst " $%d, $%d\n",       m_pc, rs, rt))
    #define INST_TRACE_STORE(inst)    DPRINTF(LOG_INST,("%08x: " inst " $%d, %d($%d)\n",   m_pc, rt, imm, rs))
    #define INST_TRACE_LOAD(inst)     DPRINTF(LOG_INST,("%08x: " inst " $%d, %d($%d)\n",   m_pc, rt, imm, rs))

    // Execute instruction
    switch (inst)
    {
        case 0x00:/*SPECIAL*/
            switch (func)
            {
            case INSTR_R_SLL:
                DPRINTF(LOG_INST,("%08x: sll $%d,$%d,%d\n",  m_pc, rd, rt, re));
                result = reg_rt << re;
                wb_reg = rd;
                break;
            case INSTR_R_SRL:
                DPRINTF(LOG_INST,("%08x: srl $%d,$%d,%d\n",  m_pc, rd, rt, re));
                result = reg_rt >> re;
                wb_reg = rd;
                break;
            case INSTR_R_SRA:
                DPRINTF(LOG_INST,("%08x: sra $%d,$%d,%d\n",  m_pc, rd, rt, re));
                result = (int)reg_rt >> re;
                wb_reg = rd;
                break;
            case INSTR_R_SLLV:
                INST_TRACE_R3("sllv");
                result = reg_rt << reg_rs;
                wb_reg = rd;
                break;
            case INSTR_R_SRLV:
                INST_TRACE_R3("srlv");
                result = reg_rt >> reg_rs;
                wb_reg = rd;
                break;
            case INSTR_R_SRAV:
                INST_TRACE_R3("srav");
                result = (int)reg_rt >> reg_rs;
                wb_reg = rd;
                break;
            case INSTR_R_JR:
                DPRINTF(LOG_INST,("%08x: jr $%d\n",  m_pc, rs));
                pc_next  = reg_rs;
                take_jmp = 1;
                break;
            case INSTR_R_JALR:
                DPRINTF(LOG_INST,("%08x: jalr $%d\n",  m_pc, rs));
                result   = pc_next;
                pc_next  = reg_rs;
                wb_reg   = rd;
                take_jmp = 1;
                break;
            case INSTR_R_SYSCALL:
                DPRINTF(LOG_INST,("%08x: syscall\n",  m_pc));
                exception(EXC_SYS, m_pc);
                take_excpn = true;
                break;
            case INSTR_R_BREAK:
                DPRINTF(LOG_INST,("%08x: break\n",  m_pc));
                exception(EXC_BP, m_pc);
                take_excpn = true;
                break;
            case INSTR_R_MFHI:
                DPRINTF(LOG_INST,("%08x: mfhi $%d\n",  m_pc, rd));
                result = m_hi;
                wb_reg = rd;
                break;
            case INSTR_R_MTHI:
                DPRINTF(LOG_INST,("%08x: mthi $%d\n",  m_pc, rd));
                m_hi = reg_rs;
                break;
            case INSTR_R_MFLO:
                DPRINTF(LOG_INST,("%08x: mflo $%d\n",  m_pc, rd));
                result = m_lo;
                wb_reg = rd;
                break;
            case INSTR_R_MTLO:
                DPRINTF(LOG_INST,("%08x: mtlo $%d\n",  m_pc, rd));
                m_lo = reg_rs;
                break;
            case INSTR_R_MULT:
            {
                INST_TRACE_MULDIV("mult");
                long long res = ((long long) (int)reg_rs) * ((long long)(int)reg_rt);
                m_hi = res >> 32;
                m_lo = res >> 0;
            }
            break;
            case INSTR_R_MULTU:
            {
                INST_TRACE_MULDIV("multu");
                unsigned long long res = ((unsigned long long) (unsigned)reg_rs) * ((unsigned long long)(unsigned)reg_rt);
                m_hi = res >> 32;
                m_lo = res >> 0;
            }
            break;
            case INSTR_R_DIV:
            {
                INST_TRACE_MULDIV("div");

                // Div 0
                if (reg_rt == 0)
                {
                    m_lo = ((signed)reg_rs < 0) ? 1 : -1;
                    m_hi = reg_rs;
                }
                // Divide min signed value by -1
                else if (reg_rs == 0x80000000 && reg_rt == 0xffffffff)
                {
                    m_lo = 0x80000000;
                    m_hi = 0x00000000;
                }
                else
                {
                    m_lo = (signed)reg_rs / (signed)reg_rt;
                    m_hi = (signed)reg_rs % (signed)reg_rt;
                }
            }
            break;
            case INSTR_R_DIVU:
            {
                INST_TRACE_MULDIV("divu");

                // Div 0
                if (reg_rt == 0)
                {
                    m_lo = -1;
                    m_hi = reg_rs;
                }
                else
                {
                    m_lo = reg_rs / reg_rt;
                    m_hi = reg_rs % reg_rt;
                }
            }
            break;
            case INSTR_R_ADD:
                INST_TRACE_R3("add");
                result = reg_rs + reg_rt;
                if (ARITH_OVERFLOW(reg_rs, reg_rt, result))
                {
                    exception(EXC_OV, m_pc);
                    take_excpn = true;
                }
                else
                    wb_reg = rd;
                break;
            case INSTR_R_ADDU:
                INST_TRACE_R3("addu");
                result = reg_rs + reg_rt;
                wb_reg = rd;
                break;
            case INSTR_R_SUB:
                INST_TRACE_R3("sub");
                result = reg_rs - reg_rt;
                if (ARITH_OVERFLOW(reg_rs, reg_rt, result))
                {
                    exception(EXC_OV, m_pc);
                    take_excpn = true;
                }
                else
                    wb_reg = rd;
                break;
            case INSTR_R_SUBU:
                INST_TRACE_R3("subu");
                result = reg_rs - reg_rt;
                wb_reg = rd;
                break;
            case INSTR_R_AND:
                INST_TRACE_R3("and");
                result = reg_rs & reg_rt;
                wb_reg = rd;
                break;
            case INSTR_R_OR:
                INST_TRACE_R3("or");
                result = reg_rs | reg_rt;
                wb_reg = rd;
                break;
            case INSTR_R_XOR:
                INST_TRACE_R3("xor");
                result = reg_rs ^ reg_rt;
                wb_reg = rd;
                break;
            case INSTR_R_NOR:
                INST_TRACE_R3("nor");
                result = ~(reg_rs | reg_rt);
                wb_reg = rd;
                break;
            case INSTR_R_SLT:
                INST_TRACE_R3("slt");
                result = (int)reg_rs < (int)reg_rt;
                wb_reg = rd;
                break;
            case INSTR_R_SLTU:
                INST_TRACE_R3("sltu");
                result = reg_rs < reg_rt;
                wb_reg = rd;
                break;
            default:
                fprintf (stderr,"Fault @ PC %x\n", m_pc);           
                exception(EXC_RI, m_pc);
                take_excpn = true;
                break;
            }
        break;
        case INSTR_I_REGIMM:
            switch(rt) 
            {
            case INSTR_I_COND_BLTZAL:
                INST_TRACE_BIZ("bltzal");
                result      = pc_next;
                take_branch = ((int)reg_rs < 0);
                wb_reg      = MIPS_REG_RA;
                break;
            case INSTR_I_COND_BLTZ:
                INST_TRACE_BIZ("bltz");
                take_branch = ((int)reg_rs < 0);
                break;
            case INSTR_I_COND_BGEZAL:
                INST_TRACE_BIZ("bgezal");
                result      = pc_next;
                wb_reg      = MIPS_REG_RA;
                take_branch = ((int)reg_rs >= 0);
                break;
            case INSTR_I_COND_BGEZ:
                INST_TRACE_BIZ("bgez");
                take_branch = ((int)reg_rs >= 0);
                break;
            default:
                fprintf (stderr,"Fault @ PC %x\n", m_pc);
                exception(EXC_RI, m_pc);
                take_excpn = true;
                break;
            }
            break;
        case INSTR_J_JAL:
            result   = pc_next;
            wb_reg   = MIPS_REG_RA;
            pc_next  = (pc & 0xf0000000) | (target << 2);
            take_jmp = 1;
            DPRINTF(LOG_INST,("%08x: jal 0x%x\n",  m_pc, pc_next));
            break;
        case INSTR_J_J:
            pc_next  = (pc & 0xf0000000) | (target << 2);
            take_jmp = 1;
            DPRINTF(LOG_INST,("%08x: j 0x%x\n",  m_pc, pc_next));
            break;
        case INSTR_J_BEQ:
            INST_TRACE_BIR("beq");
            take_branch = (reg_rs == reg_rt);
            break;
        case INSTR_J_BNE:
            INST_TRACE_BIR("bne");
            take_branch = (reg_rs != reg_rt);
            break;
        case INSTR_J_BLEZ:
            INST_TRACE_BIZ("blez");
            take_branch = ((int)reg_rs <= 0);
            break;
        case INSTR_J_BGTZ:
            INST_TRACE_BIZ("bgtz");
            take_branch = ((int)reg_rs > 0);
            break;
        case INSTR_I_ADDI:
            INST_TRACE_I("addi");
            result = reg_rs + (signed short)imm;
            if (ARITH_OVERFLOW(reg_rs, (signed short)imm, result))
            {
                exception(EXC_OV, m_pc);
                take_excpn = true;
            }
            else
                wb_reg = rt;
            break;
        case INSTR_I_ADDIU:
            INST_TRACE_I("addiu");
            result = reg_rs + (signed short)imm;
            wb_reg = rt;
            break;
        case INSTR_I_SLTI:
            INST_TRACE_I("slti");
            result = (int)reg_rs < (signed short)imm;
            wb_reg = rt;
            break;
        case INSTR_I_SLTIU:
            INST_TRACE_I("sltiu");
            result = reg_rs < (unsigned int)(signed short)imm;
            wb_reg = rt;
            break;
        case INSTR_I_ANDI:
            INST_TRACE_I("andi");
            result = reg_rs & imm;
            wb_reg = rt;
            break;
        case INSTR_I_ORI:
            INST_TRACE_I("ori");
            result = reg_rs | imm;
            wb_reg = rt;
            break;
        case INSTR_I_XORI:
            INST_TRACE_I("xori");
            result = reg_rs ^ imm;
            wb_reg = rt;
            break;
        case INSTR_I_LUI:
            DPRINTF(LOG_INST,("%08x: lui $%d,0x%x\n",  m_pc, rt, (imm<<16)));
            result = (imm << 16);
            wb_reg = rt;
            break;
        case INSTR_I_LB:
            INST_TRACE_LOAD("lb");
            wb_reg = rt;
            if (!load(m_pc, reg_rs + (signed short)imm, &result, 1, true))
                take_excpn = true;
            else
            {
                // Now in load delay slot
                SET_LOAD_DELAY_SLOT(rt);
            }
            break;        
        case INSTR_I_LH:
            INST_TRACE_LOAD("lw");
            wb_reg = rt;
            if (!load(m_pc, reg_rs + (signed short)imm, &result, 2, true))
                take_excpn = true;
            else
            {
                // Now in load delay slot
                SET_LOAD_DELAY_SLOT(rt);
            }
            break; 
        case INSTR_I_LW:
            INST_TRACE_LOAD("lw");
            wb_reg = rt;
            if (!load(m_pc, reg_rs + (signed short)imm, &result, 4, true))
                take_excpn = true;
            else
            {
                // Now in load delay slot
                SET_LOAD_DELAY_SLOT(rt);
            }
            break; 
        case INSTR_I_LBU:
            INST_TRACE_LOAD("lbu");
            wb_reg = rt;
            if (!load(m_pc, reg_rs + (signed short)imm, &result, 1, false))
                take_excpn = true;
            else
            {
                // Now in load delay slot
                SET_LOAD_DELAY_SLOT(rt);
            }
            break; 
        case INSTR_I_LHU:
            INST_TRACE_LOAD("lhu");
            wb_reg = rt;
            if (!load(m_pc, reg_rs + (signed short)imm, &result, 2, false))
                take_excpn = true;
            else
            {
                // Now in load delay slot
                SET_LOAD_DELAY_SLOT(rt);
            }
            break;

        case INSTR_I_LWL:
        {
            INST_TRACE_LOAD("lwl");
            wb_reg = rt;

            uint32_t addr     = reg_rs + (signed short)imm;
            uint32_t load_val = 0;
            if (!load(m_pc, addr & ~3, &load_val, 4, false))
                take_excpn = true;
            else
            {
                uint32_t mask = 0x00FFFFFF >> ((addr & 3) * 8);
                load_val <<= ((3-(addr & 3)) * 8);
                result = (reg_rt & mask) | load_val;

                // Now in load delay slot
                SET_LOAD_DELAY_SLOT(rt);
            }
        }
        break;

        case INSTR_I_LWR:
        {
            INST_TRACE_LOAD("lwr");
            wb_reg = rt;

            uint32_t addr     = reg_rs + (signed short)imm;
            uint32_t load_val = 0;
            if (!load(m_pc, addr & ~3, &load_val, 4, false))
                take_excpn = true;
            else
            {
                uint32_t mask = 0xFFFFFF00 << ((3-(addr & 3)) * 8);
                load_val >>= ((addr & 3) * 8);
                result = (reg_rt & mask) | load_val;

                // Now in load delay slot
                SET_LOAD_DELAY_SLOT(rt);
            }
        }
        break;

        case INSTR_I_SB:
        {
            INST_TRACE_STORE("sb");
            uint32_t addr = reg_rs + (signed short)imm;
            if (!store(m_pc, addr, reg_rt, 1, 1 << (addr & 3)))
                take_excpn = true;
        }
        break;

        case INSTR_I_SH:
        {
            INST_TRACE_STORE("sh");
            uint32_t addr = reg_rs + (signed short)imm;
            if (!store(m_pc, addr, reg_rt, 2, 0x3 << (addr & 2)))
                take_excpn = true;
        }
        break;

        case INSTR_I_SW:
            INST_TRACE_STORE("sw");
            if (!store(m_pc, reg_rs + (signed short)imm, reg_rt, 4, 0xF))
                take_excpn = true;
            break;

        case INSTR_I_SWL:
        {
            INST_TRACE_STORE("swl");
            uint32_t addr = reg_rs + (signed short)imm;
            switch (addr & 3)
            {
                case 0: reg_rt = (reg_rt >> 24); break;
                case 1: reg_rt = (reg_rt >> 16); break;
                case 2: reg_rt = (reg_rt >> 8);  break;
                case 3: reg_rt = (reg_rt >> 0);  break;
            }
            uint8_t mask = 0xF;
            switch (addr & 3)
            {
                case 0: mask = 0x1; break;
                case 1: mask = 0x3; break;
                case 2: mask = 0x7; break;
                case 3: mask = 0xF; break;
            }
            if (!store(m_pc, addr & ~3, reg_rt, 4, mask))
                take_excpn = true;
        }
        break;

        case INSTR_I_SWR:
        {
            INST_TRACE_STORE("swr");
            uint32_t addr = reg_rs + (signed short)imm;
            switch (addr & 3)
            {
                case 0: reg_rt = (reg_rt << 0);  break;
                case 1: reg_rt = (reg_rt << 8);  break;
                case 2: reg_rt = (reg_rt << 16); break;
                case 3: reg_rt = (reg_rt << 24); break;
            }
            uint8_t mask = 0xF;
            switch (addr & 3)
            {
                case 0: mask = 0xf; break;
                case 1: mask = 0xe; break;
                case 2: mask = 0xc; break;
                case 3: mask = 0x8; break;
            }
            if (!store(m_pc, addr & ~3, reg_rt, 4, mask))
                take_excpn = true;
        }
        break;

        case INSTR_COP0:
            m_stats[STATS_COPRO]++;
            if (!copro0_inst(m_pc, opcode, reg_rs, reg_rt, wb_reg, result))
                take_excpn = true;
            break;
        case INSTR_COP1:
        case INSTR_COP2:
        case INSTR_COP3:
            m_stats[STATS_COPRO]++;
            if (!copro_inst(inst-INSTR_COP0, m_pc, opcode, reg_rs, reg_rt, wb_reg, result))
                take_excpn = true;
            break;

        case INSTR_I_LWC1:
        case INSTR_I_LWC2:
        case INSTR_I_LWC3:
            INST_TRACE_LOAD("lwc");
            if (!load(m_pc, reg_rs + (signed short)imm, &result, 4, true))
                take_excpn = true;
            else
            {
                m_stats[STATS_COPRO]++;
                if (!copro_inst(inst-INSTR_I_LWC0, m_pc, opcode, result, 0, wb_reg, result))
                    take_excpn = true;
            }
            wb_reg = 0;
            break;

        case INSTR_I_SWC1:
        case INSTR_I_SWC2:
        case INSTR_I_SWC3:        
            INST_TRACE_STORE("swc");
            m_stats[STATS_COPRO]++;
            if (!copro_inst(inst-INSTR_I_SWC0, m_pc, opcode, 0, 0, wb_reg, result))
                take_excpn = true;
            else if (!store(m_pc, reg_rs + (signed short)imm, result, 4, 0xF))
                take_excpn = true;
            wb_reg = 0;
            break;

        default:
            fprintf (stderr,"Fault @ PC %x\n", m_pc);
            exception(EXC_RI, m_pc);
            take_excpn = true;
            break;
    }

    m_take_excpn = false;

    // If not handling an exception
    bool take_irq = false;
    if (!take_excpn)
    {
        // Interrupt pending and enabled (and not already taking a trap)
        if (SR_BF_GET(m_status, IEC) && CAUSE_BF_GET(m_cause, IP0, 0xFF) != 0)
        {
            uint32_t mask    = SR_BF_GETM(m_status,  IM0, 0xFF);
            uint32_t pending = CAUSE_BF_GET(m_cause, IP0, 0xFF);

            // Interrupt pending and enabled
            pending &= mask;
            if (pending)
            {
                // Current instruction was executed, return to next
                exception(EXC_INT, m_pc);

                // Jump to exception handler
                pc      = m_isr_vector;
                pc_next = pc + 4;

                m_stats[STATS_EXCEPTIONS]++;
                m_stats[STATS_BRANCHES]++;
                m_take_excpn = true;

                take_irq    = true;

                // Squash writeback
                wb_reg  = 0;
            }
        }
    }

    // Trap (faults, syscall, etc)
    if (take_excpn)
    {
        // Jump to exception handler
        pc      = m_isr_vector;
        pc_next = pc + 4;

        // Squash writeback
        wb_reg  = 0;

        m_stats[STATS_EXCEPTIONS]++;
        m_stats[STATS_BRANCHES]++;
        m_take_excpn = true;
    }
    else if (take_irq)
        ;
    // Handle branches (b****)
    else if (take_branch)
    {
        uint32_t offset = imm_int32;
        offset = offset << 2;
        pc_next = m_pc_next + offset;
        m_stats[STATS_BRANCHES]++;
        m_branch_ds = true;
    }
    // Jumps (J, JAL, JR, JALR)
    else if (take_jmp)
    {
        m_stats[STATS_BRANCHES]++;
        m_branch_ds = true;
    }
    else
        m_branch_ds = false;

    // Update registers with variable values
    m_pc_x      = m_pc;
    m_pc        = pc;
    m_pc_next   = pc_next;

    // Register writeback required?
    if (wb_reg != 0)
        m_gpr[wb_reg] = result;

    return !take_irq;
}
//-----------------------------------------------------------------
// step: Step through one instruction
//-----------------------------------------------------------------
void mips_i::step(void)
{
    m_stats[STATS_INSTRUCTIONS]++;

    // Execute instruction at current PC
    int max_steps = 2;
    while (max_steps-- && !execute())
        ;

    // Increment timer counter
    m_cycles++;

    // Dump state
    if (TRACE_ENABLED(LOG_REGISTERS))
    {
        // Register trace
        int i;
        for (i=0;i<REGISTERS;i+=4)
        {
            DPRINTF(LOG_REGISTERS,( " %d: ", i));
            DPRINTF(LOG_REGISTERS,( " %08x %08x %08x %08x\n", m_gpr[i+0], m_gpr[i+1], m_gpr[i+2], m_gpr[i+3]));
        }
    }

    cpu::step();
}
//-----------------------------------------------------------------
// set_interrupt: Register pending interrupt
//-----------------------------------------------------------------
void mips_i::set_interrupt(int irq)
{
    assert(irq >= 0 && irq < 8);
    m_cause = CAUSE_BF_SET(m_cause, IP0, 0xFF, 1 << irq);
}
//-----------------------------------------------------------------
// stats_reset: Reset runtime stats
//-----------------------------------------------------------------
void mips_i::stats_reset(void)
{
    // Clear stats
    for (int i=STATS_MIN;i<STATS_MAX;i++)
        m_stats[i] = 0;
}
//-----------------------------------------------------------------
// stats_dump: Show execution stats
//-----------------------------------------------------------------
void mips_i::stats_dump(void)
{  
    printf("Runtime Stats:\n");
    printf("- Total Instructions %d\n", m_stats[STATS_INSTRUCTIONS]);
    if (m_stats[STATS_INSTRUCTIONS] > 0)
    {
        printf("- Loads %d (%d%%)\n",  m_stats[STATS_LOADS],  (m_stats[STATS_LOADS] * 100)  / m_stats[STATS_INSTRUCTIONS]);
        printf("- Stores %d (%d%%)\n", m_stats[STATS_STORES], (m_stats[STATS_STORES] * 100) / m_stats[STATS_INSTRUCTIONS]);
        printf("- Co-processor Operations %d (%d%%)\n", m_stats[STATS_COPRO], (m_stats[STATS_COPRO] * 100) / m_stats[STATS_INSTRUCTIONS]);
        printf("- NOPS %d (%d%%)\n", m_stats[STATS_NOP], (m_stats[STATS_NOP] * 100) / m_stats[STATS_INSTRUCTIONS]);
        printf("- Branches Operations %d (%d%%)\n", m_stats[STATS_BRANCHES], (m_stats[STATS_BRANCHES] * 100) / m_stats[STATS_INSTRUCTIONS]);
        printf("- Exceptions %d (%d%%)\n", m_stats[STATS_EXCEPTIONS], (m_stats[STATS_EXCEPTIONS] * 100) / m_stats[STATS_INSTRUCTIONS]);
    }

    stats_reset();
}
