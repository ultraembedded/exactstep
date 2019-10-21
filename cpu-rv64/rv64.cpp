#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "rv64.h"
#include "rv64_isa.h"

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

//-----------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------
rv64::rv64(uint32_t baseAddr /*= 0*/, uint32_t len /*= 0*/): cpu()
{
    m_enable_mem_errors  = false;
    m_compliant_csr      = false;

    // Some memory defined
    if (len != 0)
        create_memory(baseAddr, len);

    reset(baseAddr);
}
//-----------------------------------------------------------------
// set_pc: Set PC
//-----------------------------------------------------------------
void rv64::set_pc(uint64_t pc)
{
    m_pc        = pc;
    m_pc_x      = pc;
}
void rv64::set_pc(uint32_t pc)  { set_pc((uint64_t)pc); }
//-----------------------------------------------------------------
// set_register: Set register value
//-----------------------------------------------------------------
void rv64::set_register(int r, uint64_t val)
{
    if (r <= RISCV_REGNO_GPR31)   m_gpr[r] = val;
    else if (r == RISCV_REGNO_PC) m_pc     = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MEPC)) m_csr_mepc = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MCAUSE)) m_csr_mcause = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MSTATUS)) m_csr_msr = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTVEC)) m_csr_mevec = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIE)) m_csr_mie = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIP)) m_csr_mip = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTIME)) m_csr_mtime = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTIMEH)) m_csr_mtimecmp = val; // Non-std
    else if (r == (RISCV_REGNO_CSR0 + CSR_MSCRATCH)) m_csr_mscratch = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIDELEG)) m_csr_mideleg = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MEDELEG)) m_csr_medeleg = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SEPC)) m_csr_sepc = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_STVEC)) m_csr_sevec = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SCAUSE)) m_csr_scause = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_STVAL)) m_csr_stval = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SATP)) m_csr_satp = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SSCRATCH)) m_csr_sscratch = val;
    else if (r == RISCV_REGNO_PRIV) m_csr_mpriv = val;  
}
void rv64::set_register(int r, uint32_t val) { set_register(r, (uint64_t)val); }
//-----------------------------------------------------------------
// get_register: Get register value
//-----------------------------------------------------------------
uint32_t rv64::get_register(int r)
{
    if (r <= RISCV_REGNO_GPR31)   return m_gpr[r];
    else if (r == RISCV_REGNO_PC) return m_pc;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MEPC)) return m_csr_mepc;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MCAUSE)) return m_csr_mcause;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MSTATUS)) return m_csr_msr;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTVEC)) return m_csr_mevec;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIE)) return m_csr_mie;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIP)) return m_csr_mip;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MCYCLE)) return m_csr_mtime;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTIME)) return m_csr_mtime;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTIMEH)) return m_csr_mtimecmp; // Non-std
    else if (r == (RISCV_REGNO_CSR0 + CSR_MSCRATCH)) return m_csr_mscratch;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIDELEG)) return m_csr_mideleg;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MEDELEG)) return m_csr_medeleg;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SEPC)) return m_csr_sepc;
    else if (r == (RISCV_REGNO_CSR0 + CSR_STVEC)) return m_csr_sevec;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SCAUSE)) return m_csr_scause;
    else if (r == (RISCV_REGNO_CSR0 + CSR_STVAL)) return m_csr_stval;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SATP)) return m_csr_satp;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SSCRATCH)) return m_csr_sscratch;
    else if (r == RISCV_REGNO_PRIV) return m_csr_mpriv;

    return 0;
}
//-----------------------------------------------------------------
// get_register64: Get register value
//-----------------------------------------------------------------
uint64_t rv64::get_register64(int r)
{
    if (r <= RISCV_REGNO_GPR31)   return m_gpr[r];
    else if (r == RISCV_REGNO_PC) return m_pc;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MEPC)) return m_csr_mepc;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MCAUSE)) return m_csr_mcause;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MSTATUS)) return m_csr_msr;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTVEC)) return m_csr_mevec;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIE)) return m_csr_mie;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIP)) return m_csr_mip;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MCYCLE)) return m_csr_mtime;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTIME)) return m_csr_mtime;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTIMEH)) return m_csr_mtimecmp; // Non-std
    else if (r == (RISCV_REGNO_CSR0 + CSR_MSCRATCH)) return m_csr_mscratch;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIDELEG)) return m_csr_mideleg;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MEDELEG)) return m_csr_medeleg;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SEPC)) return m_csr_sepc;
    else if (r == (RISCV_REGNO_CSR0 + CSR_STVEC)) return m_csr_sevec;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SCAUSE)) return m_csr_scause;
    else if (r == (RISCV_REGNO_CSR0 + CSR_STVAL)) return m_csr_stval;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SATP)) return m_csr_satp;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SSCRATCH)) return m_csr_sscratch;
    else if (r == RISCV_REGNO_PRIV) return m_csr_mpriv;

    return 0;
}
//-----------------------------------------------------------------
// reset: Reset CPU state
//-----------------------------------------------------------------
void rv64::reset(uint32_t start_addr)
{
    m_pc        = start_addr;
    m_pc_x      = start_addr;

    for (int i=0;i<REGISTERS;i++)
        m_gpr[i] = 0;

    m_csr_mpriv    = PRIV_MACHINE;
    m_csr_msr      = 0;
    m_csr_mideleg  = 0;
    m_csr_medeleg  = 0;

    m_csr_mepc     = 0;
    m_csr_mie      = 0;
    m_csr_mip      = 0;
    m_csr_mcause   = 0;
    m_csr_mevec    = 0;
    m_csr_mtime    = 0;
    m_csr_mtimecmp = 0;
    m_csr_mscratch = 0;

    m_csr_sepc     = 0;
    m_csr_sevec    = 0;
    m_csr_scause   = 0;
    m_csr_stval    = 0;
    m_csr_satp     = 0;
    m_csr_sscratch = 0;

    m_fault       = false;
    m_break       = false;
    m_trace       = 0;

    stats_reset();
}
//-----------------------------------------------------------------
// get_opcode: Get instruction from address
//-----------------------------------------------------------------
uint32_t rv64::get_opcode(uint64_t address)
{
    return read32(address);
}
//-----------------------------------------------------------------
// load: Perform a load operation (with optional MMU lookup)
//-----------------------------------------------------------------
int rv64::load(uint64_t pc, uint64_t address, uint64_t *result, int width, bool signedLoad)
{
    uint32_t physical = address;

    DPRINTF(LOG_MEM, ("LOAD: VA 0x%08x PA 0x%08x Width %d\n", address, physical, width));

    // Detect misaligned load
    if (((address & 7) != 0 && width == 8) || ((address & 3) != 0 && width == 4) || ((address & 1) != 0 && width == 2))
    {
        exception(MCAUSE_MISALIGNED_LOAD, pc, address);
        return 0;
    }

    m_stats[STATS_LOADS]++;
    *result = 0;

    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(physical))
        {
            switch (width)
            {
                case 8:
                {
                    uint32_t dw = 0;
                    mem->read32(physical + 0, dw);
                    *result |= ((uint64_t)dw << 0);
                    mem->read32(physical + 4, dw);
                    *result |= ((uint64_t)dw << 32);
                }
                break;
                case 4:
                {
                    uint32_t dw = 0;
                    mem->read32(physical, dw);
                    *result = dw;

                    if (signedLoad && ((*result) & (1 << 31)))
                         *result |= 0xFFFFFFFF00000000;                    
                }
                break;
                case 2:
                {
                    uint8_t db = 0;
                    mem->read8(physical + 0, db);
                    *result |= ((uint32_t)db << 0);
                    mem->read8(physical + 1, db);
                    *result |= ((uint32_t)db << 8);

                    if (signedLoad && ((*result) & (1 << 15)))
                         *result |= 0xFFFFFFFFFFFF0000;
                }
                break;
                case 1:
                {
                    uint8_t db = 0;
                    mem->read8(physical + 0, db);
                    *result |= ((uint32_t)db << 0);

                    if (signedLoad && ((*result) & (1 << 7)))
                         *result |= 0xFFFFFFFFFFFFFF00;
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
        exception(MCAUSE_FAULT_LOAD, pc, address);
        return 0;
    }

    error(false, "%08x: Bad memory access 0x%x\n", pc, address);
    return 0;
}
//-----------------------------------------------------------------
// store: Perform a store operation (with optional MMU lookup)
//-----------------------------------------------------------------
int rv64::store(uint64_t pc, uint64_t address, uint64_t data, int width)
{
    uint32_t physical = address;

    DPRINTF(LOG_MEM, ("STORE: VA 0x%08x PA 0x%08x Value 0x%08x Width %d\n", address, physical, data, width));

    // Detect misaligned load
    if (((address & 7) != 0 && width == 8) || ((address & 3) != 0 && width == 4) || ((address & 1) != 0 && width == 2))
    {
        exception(MCAUSE_MISALIGNED_STORE, pc, address);
        return 0;
    }

    m_stats[STATS_STORES]++;

    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(physical))
        {
            switch (width)
            {
                case 8:
                    mem->write32(physical + 0, data >> 0);
                    mem->write32(physical + 4, data >> 32);
                    break;                
                case 4:
                    mem->write32(physical, data);
                    break;
                case 2:
                    mem->write8(physical + 0, data >> 0);
                    mem->write8(physical + 1, data >> 8);
                    break;
                case 1:
                    mem->write8(physical + 0, data & 0xFF);
                    break;
                default:
                    assert(!"Invalid");
                    break;
            }
            return 1;
        }

    if (m_enable_mem_errors)
    {
        exception(MCAUSE_FAULT_STORE, pc, address);
        return 0;
    }

    error(false, "%08x: Bad memory access 0x%x\n", pc, address);
    return 0;
}
//-----------------------------------------------------------------
// access_csr: Perform CSR access
//-----------------------------------------------------------------
bool rv64::access_csr(uint64_t address, uint64_t data, bool set, bool clr, uint64_t &result)
{
    result = 0;

#define CSR_STD(name, var_name) \
    case CSR_ ##name: \
    { \
        data       &= CSR_ ##name## _MASK; \
        result      = var_name & CSR_ ##name## _MASK; \
        if (set && clr) \
            var_name  = data; \
        else if (set) \
            var_name |= data; \
        else if (clr) \
            var_name &= ~data; \
    } \
    break;

#define CSR_CONST(name, value) \
    case CSR_ ##name: \
    { \
        result      = value; \
    } \
    break;

    // Apply CSR access permissions
    if (m_compliant_csr)
    {
        // Get permissions required inferred from the CSR address
        uint32_t csr_priv      = (address >> 8) & 0x3;
        uint32_t csr_read_only = ((address >> 10) & 0x3) == 3;

        if (((set || clr) && csr_read_only) || m_csr_mpriv < csr_priv)
        {
            DPRINTF(LOG_INST, ("-> CSR %08x access fault - permission level %d required %d\n", address & 0xFFF, m_csr_mpriv, csr_priv));
            return true;
        }
    }

    switch (address & 0xFFF)
    {
        //--------------------------------------------------------
        // Simulation control
        //--------------------------------------------------------
        case CSR_DSCRATCH:
            switch (data & 0xFF000000)
            {
                case CSR_SIM_CTRL_EXIT:
                    stats_dump();
                    printf("Exit code = %d\n", (char)(data & 0xFF));
                    exit(data & 0xFF);
                    break;
                case CSR_SIM_CTRL_PUTC:
                    if (m_console)
                        m_console->putchar(data & 0xFF);
                    else
                        fprintf(stderr, "%c", (data & 0xFF));
                    break;
                case CSR_SIM_CTRL_GETC:
                    if (m_console)
                        result = m_console->getchar();
                    else
                        result = 0;
                    break;
                case CSR_SIM_CTRL_TRACE:
                    enable_trace(data & 0xFF);
                    break;
                case CSR_SIM_PRINTF:
                {
                    uint32_t fmt_addr = m_gpr[10];
                    uint32_t arg1     = m_gpr[11];
                    uint32_t arg2     = m_gpr[12];
                    uint32_t arg3     = m_gpr[13];
                    uint32_t arg4     = m_gpr[14];

                    char fmt_str[1024];
                    int idx = 0;
                    while (idx < (sizeof(fmt_str)-1))
                    {
                        fmt_str[idx] = read(fmt_addr++);
                        if (fmt_str[idx] == 0)
                            break;
                        idx += 1;
                    }

                    char out_str[1024];
                    sprintf(out_str, fmt_str, arg1, arg2, arg3, arg4);
                    printf("%s",out_str);
                }
                break;
            }
         break;
        //--------------------------------------------------------
        // Standard - Machine
        //--------------------------------------------------------
        CSR_STD(MEPC,    m_csr_mepc)
        CSR_STD(MTVEC,   m_csr_mevec)
        CSR_STD(MCAUSE,  m_csr_mcause)
        CSR_STD(MSTATUS, m_csr_msr)
        CSR_STD(MIP,     m_csr_mip)
        CSR_STD(MIE,     m_csr_mie)
        CSR_CONST(MISA,  MISA_VALUE)
        CSR_STD(MIDELEG, m_csr_mideleg)
        CSR_STD(MEDELEG, m_csr_medeleg)
        CSR_STD(MSCRATCH,m_csr_mscratch)
        CSR_CONST(MHARTID,  MHARTID_VALUE)
        //--------------------------------------------------------
        // Standard - Supervisor
        //--------------------------------------------------------
        CSR_STD(SEPC,    m_csr_sepc)
        CSR_STD(STVEC,   m_csr_sevec)
        CSR_STD(SCAUSE,  m_csr_scause)
        CSR_STD(SIP,     m_csr_mip)
        CSR_STD(SIE,     m_csr_mie)
        CSR_STD(SATP,    m_csr_satp)
        CSR_STD(STVAL,   m_csr_stval)
        CSR_STD(SSCRATCH,m_csr_sscratch)
        CSR_STD(SSTATUS, m_csr_msr)
        //--------------------------------------------------------
        // Extensions
        //-------------------------------------------------------- 
        case CSR_MTIME:
            data       &= CSR_MTIME_MASK;
            result      = m_csr_mtime;

            // Non-std behaviour - write to CSR_TIME gives next interrupt threshold            
            if (!m_compliant_csr && set)
            {
                m_csr_mtimecmp = data;

                // Clear interrupt pending
                if (m_csr_mideleg & SR_IP_STIP)
                    m_csr_mip &= ~SR_IP_STIP;
                else
                    m_csr_mip &= ~SR_IP_MTIP;
            }
            break;
    
        case CSR_MTIMEH:
            result      = m_csr_mtime >> 32;
            break;
       case CSR_MCYCLE:
            result      = m_csr_mtime;
            break;
        default:
            error(false, "*** CSR address not supported %08x [PC=%08x]\n", address, m_pc);
            break;
    }
    return false;
}
//-----------------------------------------------------------------
// exception: Handle an exception or interrupt
//-----------------------------------------------------------------
void rv64::exception(uint64_t cause, uint64_t pc, uint64_t badaddr /*= 0*/)
{
    uint32_t deleg;
    uint32_t bit;

    // Interrupt
    if (cause >= MCAUSE_INTERRUPT)
    {
        deleg = m_csr_mideleg;
        bit   = 1 << (cause - MCAUSE_INTERRUPT);
    }
    // Exception
    else
    {
        deleg = m_csr_medeleg;
        bit   = 1 << cause;
    }

    // Exception delegated to supervisor mode
    if (m_csr_mpriv <= PRIV_SUPER && (deleg & bit))
    {
        uint32_t s = m_csr_msr;

        // Interrupt save and disable
        s &= ~SR_SPIE;
        s |= (s & SR_SIE) ? SR_SPIE : 0;
        s &= ~SR_SIE;

        // Record previous priviledge level
        s &= ~SR_SPP;
        s |= (m_csr_mpriv == PRIV_SUPER) ? SR_SPP : 0;

        // Raise priviledge to supervisor level
        m_csr_mpriv  = PRIV_SUPER;

        m_csr_msr    = s;
        m_csr_sepc   = pc;
        m_csr_scause = cause;
        m_csr_stval  = badaddr;

        // Set new PC
        m_pc         = m_csr_sevec;
    }
    // Machine mode
    else
    {
        uint32_t s = m_csr_msr;

        // Interrupt save and disable
        s &= ~SR_MPIE;
        s |= (s & SR_MIE) ? SR_MPIE : 0;
        s &= ~SR_MIE;

        // Record previous priviledge level
        s &= ~SR_MPP;
        s |= (m_csr_mpriv << SR_MPP_SHIFT);

        // Raise priviledge to machine level
        m_csr_mpriv  = PRIV_MACHINE;

        m_csr_msr    = s;
        m_csr_mepc   = pc;
        m_csr_mcause = cause;

        // Set new PC
        m_pc         = m_csr_mevec; // TODO: This should be a product of the except num
    }
}
//-----------------------------------------------------------------
// execute: Instruction execution stage
//-----------------------------------------------------------------
void rv64::execute(void)
{
    uint64_t phy_pc = m_pc;

    // Get opcode at current PC
    uint32_t opcode = get_opcode(phy_pc);
    m_pc_x = m_pc;

    // Extract registers
    int rd          = (opcode & OPCODE_RD_MASK)  >> OPCODE_RD_SHIFT;
    int rs1         = (opcode & OPCODE_RS1_MASK) >> OPCODE_RS1_SHIFT;
    int rs2         = (opcode & OPCODE_RS2_MASK) >> OPCODE_RS2_SHIFT;

    // Extract immediates
    int64_t typei_imm   = SEXT32(opcode & OPCODE_TYPEI_IMM_MASK) >> OPCODE_TYPEI_IMM_SHIFT;
    int64_t typeu_imm   = SEXT32(opcode & OPCODE_TYPEU_IMM_MASK) >> OPCODE_TYPEU_IMM_SHIFT;
    int64_t imm20       = typeu_imm << OPCODE_TYPEU_IMM_SHIFT;
    int64_t imm12       = typei_imm;
    int64_t bimm        = OPCODE_SBTYPE_IMM(opcode);
    int64_t jimm20      = OPCODE_UJTYPE_IMM(opcode);
    int64_t storeimm    = OPCODE_STYPE_IMM(opcode);
    int     shamt       = ((signed)(opcode & OPCODE_SHAMT_MASK)) >> OPCODE_SHAMT_SHIFT;

    // Retrieve registers
    uint64_t reg_rd  = 0;
    uint64_t reg_rs1 = m_gpr[rs1];
    uint64_t reg_rs2 = m_gpr[rs2];
    uint64_t pc      = m_pc;

    bool take_exception = false;

    DPRINTF(LOG_OPCODES,( "%08x: %08x\n", pc, opcode));
    DPRINTF(LOG_OPCODES,( "        rd(%d) r%d = %d, r%d = %d\n", rd, rs1, reg_rs1, rs2, reg_rs2));

    // As RVC is not supported, fault on opcode which is all zeros
    if (opcode == 0)
    {
        error(false, "Bad instruction @ %x\n", pc);

        exception(MCAUSE_ILLEGAL_INSTRUCTION, pc);
        m_fault = true;
        take_exception = true;        
    }
    else if ((opcode & INST_ANDI_MASK) == INST_ANDI)
    {
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: andi r%d, r%d, %d\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_ANDI);
        reg_rd = reg_rs1 & imm12;
        pc += 4;        
    }
    else if ((opcode & INST_ORI_MASK) == INST_ORI)
    {
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: ori r%d, r%d, %d\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_ORI);
        reg_rd = reg_rs1 | imm12;
        pc += 4;        
    }
    else if ((opcode & INST_XORI_MASK) == INST_XORI)
    {
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: xori r%d, r%d, %d\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_XORI);
        reg_rd = reg_rs1 ^ imm12;
        pc += 4;        
    }
    else if ((opcode & INST_ADDI_MASK) == INST_ADDI)
    {
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: addi r%d, r%d, %d\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_ADDI);
        reg_rd = reg_rs1 + imm12;
        pc += 4;
    }
    else if ((opcode & INST_SLTI_MASK) == INST_SLTI)
    {
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: slti r%d, r%d, %d\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_SLTI);
        reg_rd = (int64_t)reg_rs1 < (int64_t)imm12;
        pc += 4;        
    }
    else if ((opcode & INST_SLTIU_MASK) == INST_SLTIU)
    {
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: sltiu r%d, r%d, %d\n", pc, rd, rs1, (uint64_t)imm12));
        INST_STAT(ENUM_INST_SLTIU);
        reg_rd = (uint64_t)reg_rs1 < (uint64_t)imm12;
        pc += 4;        
    }
    else if ((opcode & INST_SLLI_MASK) == INST_SLLI)
    {
        // ['rd', 'rs1']
        DPRINTF(LOG_INST,("%08x: slli r%d, r%d, %d\n", pc, rd, rs1, shamt));
        INST_STAT(ENUM_INST_SLLI);
        reg_rd = reg_rs1 << shamt;
        pc += 4;        
    }
    else if ((opcode & INST_SRLI_MASK) == INST_SRLI)
    {
        // ['rd', 'rs1', 'shamt']
        DPRINTF(LOG_INST,("%08x: srli r%d, r%d, %d\n", pc, rd, rs1, shamt));
        INST_STAT(ENUM_INST_SRLI);
        reg_rd = (uint64_t)reg_rs1 >> shamt;
        pc += 4;        
    }
    else if ((opcode & INST_SRAI_MASK) == INST_SRAI)
    {
        // ['rd', 'rs1', 'shamt']
        DPRINTF(LOG_INST,("%08x: srai r%d, r%d, %d\n", pc, rd, rs1, shamt));
        INST_STAT(ENUM_INST_SRAI);
        reg_rd = (int64_t)reg_rs1 >> shamt;
        pc += 4;        
    }
    else if ((opcode & INST_LUI_MASK) == INST_LUI)
    {
        // ['rd', 'imm20']
        DPRINTF(LOG_INST,("%08x: lui r%d, 0x%x\n", pc, rd, imm20));
        INST_STAT(ENUM_INST_LUI);
        reg_rd = imm20;
        pc += 4;        
    }
    else if ((opcode & INST_AUIPC_MASK) == INST_AUIPC)
    {
        // ['rd', 'imm20']
        DPRINTF(LOG_INST,("%08x: auipc r%d, 0x%x\n", pc, rd, imm20));
        INST_STAT(ENUM_INST_AUIPC);
        reg_rd = imm20 + pc;
        pc += 4;        
    }
    else if ((opcode & INST_ADD_MASK) == INST_ADD)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: add r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_ADD);
        reg_rd = reg_rs1 + reg_rs2;
        pc += 4;        
    }
    else if ((opcode & INST_SUB_MASK) == INST_SUB)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: sub r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_SUB);
        reg_rd = reg_rs1 - reg_rs2;
        pc += 4;        
    }
    else if ((opcode & INST_SLT_MASK) == INST_SLT)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: slt r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_SLT);
        reg_rd = (int64_t)reg_rs1 < (int64_t)reg_rs2;
        pc += 4;        
    }
    else if ((opcode & INST_SLTU_MASK) == INST_SLTU)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: sltu r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_SLTU);
        reg_rd = (uint64_t)reg_rs1 < (uint64_t)reg_rs2;
        pc += 4;        
    }
    else if ((opcode & INST_XOR_MASK) == INST_XOR)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: xor r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_XOR);
        reg_rd = reg_rs1 ^ reg_rs2;
        pc += 4;        
    }
    else if ((opcode & INST_OR_MASK) == INST_OR)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: or r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_OR);
        reg_rd = reg_rs1 | reg_rs2;
        pc += 4;        
    }
    else if ((opcode & INST_AND_MASK) == INST_AND)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: and r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_AND);
        reg_rd = reg_rs1 & reg_rs2;
        pc += 4;        
    }
    else if ((opcode & INST_SLL_MASK) == INST_SLL)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: sll r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_SLL);
        reg_rd = reg_rs1 << reg_rs2;
        pc += 4;        
    }
    else if ((opcode & INST_SRL_MASK) == INST_SRL)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: srl r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_SRL);
        reg_rd = (uint64_t)reg_rs1 >> reg_rs2;
        pc += 4;        
    }
    else if ((opcode & INST_SRA_MASK) == INST_SRA)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: sra r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_SRA);
        reg_rd = (int64_t)reg_rs1 >> reg_rs2;
        pc += 4;        
    }
    else if ((opcode & INST_JAL_MASK) == INST_JAL)
    {
        // ['rd', 'jimm20']
        DPRINTF(LOG_INST,("%08x: jal r%d, %d\n", pc, rd, jimm20));
        INST_STAT(ENUM_INST_JAL);
        reg_rd = pc + 4;
        pc+= jimm20;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_JALR_MASK) == INST_JALR)
    {
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: jalr r%d, r%d\n", pc, rs1, imm12));
        INST_STAT(ENUM_INST_JALR);
        reg_rd = pc + 4;
        pc = (reg_rs1 + imm12) & ~1;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_BEQ_MASK) == INST_BEQ)
    {
        // ['bimm12hi', 'rs1', 'rs2', 'bimm12lo']
        DPRINTF(LOG_INST,("%08x: beq r%d, r%d, %d\n", pc, rs1, rs2, bimm));
        INST_STAT(ENUM_INST_BEQ);
        if (reg_rs1 == reg_rs2)
            pc += bimm;
        else
            pc += 4;

        // No writeback
        rd = 0;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_BNE_MASK) == INST_BNE)
    {
        // ['bimm12hi', 'rs1', 'rs2', 'bimm12lo']
        DPRINTF(LOG_INST,("%08x: bne r%d, r%d, %d\n", pc, rs1, rs2, bimm));
        INST_STAT(ENUM_INST_BNE);
        if (reg_rs1 != reg_rs2)
            pc += bimm;
        else
            pc += 4;

        // No writeback
        rd = 0;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_BLT_MASK) == INST_BLT)
    {
        // ['bimm12hi', 'rs1', 'rs2', 'bimm12lo']
        DPRINTF(LOG_INST,("%08x: blt r%d, r%d, %d\n", pc, rs1, rs2, bimm));
        INST_STAT(ENUM_INST_BLT);
        if ((int64_t)reg_rs1 < (int64_t)reg_rs2)
            pc += bimm;
        else
            pc += 4;

        // No writeback
        rd = 0;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_BGE_MASK) == INST_BGE)
    {
        // ['bimm12hi', 'rs1', 'rs2', 'bimm12lo']
        DPRINTF(LOG_INST,("%08x: bge r%d, r%d, %d\n", pc, rs1, rs2, bimm));
        INST_STAT(ENUM_INST_BGE);
        if ((int64_t)reg_rs1 >= (int64_t)reg_rs2)
            pc += bimm;
        else
            pc += 4;

        // No writeback
        rd = 0;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_BLTU_MASK) == INST_BLTU)
    {
        // ['bimm12hi', 'rs1', 'rs2', 'bimm12lo']
        DPRINTF(LOG_INST,("%08x: bltu r%d, r%d, %d\n", pc, rs1, rs2, bimm));
        INST_STAT(ENUM_INST_BLTU);
        if ((uint64_t)reg_rs1 < (uint64_t)reg_rs2)
            pc += bimm;
        else
            pc += 4;

        // No writeback
        rd = 0;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_BGEU_MASK) == INST_BGEU)
    {
        // ['bimm12hi', 'rs1', 'rs2', 'bimm12lo']
        DPRINTF(LOG_INST,("%08x: bgeu r%d, r%d, %d\n", pc, rs1, rs2, bimm));
        INST_STAT(ENUM_INST_BGEU);
        if ((uint64_t)reg_rs1 >= (uint64_t)reg_rs2)
            pc += bimm;
        else
            pc += 4;

        // No writeback
        rd = 0;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_LB_MASK) == INST_LB)
    {
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: lb r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        INST_STAT(ENUM_INST_LB);
        if (load(pc, reg_rs1 + imm12, &reg_rd, 1, true))
            pc += 4;
        else
            return;
    }
    else if ((opcode & INST_LH_MASK) == INST_LH)
    {
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: lh r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        INST_STAT(ENUM_INST_LH);
        if (load(pc, reg_rs1 + imm12, &reg_rd, 2, true))
            pc += 4;
        else
            return;
    }
    else if ((opcode & INST_LW_MASK) == INST_LW)
    {
        // ['rd', 'rs1', 'imm12']        
        INST_STAT(ENUM_INST_LW);
        DPRINTF(LOG_INST,("%08x: lw r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        if (load(pc, reg_rs1 + imm12, &reg_rd, 4, true))
            pc += 4;
        else
            return;
    }
    else if ((opcode & INST_LBU_MASK) == INST_LBU)
    {
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: lbu r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        INST_STAT(ENUM_INST_LBU);
        if (load(pc, reg_rs1 + imm12, &reg_rd, 1, false))
            pc += 4;
        else
            return;
    }
    else if ((opcode & INST_LHU_MASK) == INST_LHU)
    {
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: lhu r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        INST_STAT(ENUM_INST_LHU);
        if (load(pc, reg_rs1 + imm12, &reg_rd, 2, false))
            pc += 4;
        else
            return;
    }
    else if ((opcode & INST_LWU_MASK) == INST_LWU)
    {
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: lwu r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        INST_STAT(ENUM_INST_LWU);
        if (load(pc, reg_rs1 + imm12, &reg_rd, 4, false))
            pc += 4;
        else
            return;
    }
    else if ((opcode & INST_SB_MASK) == INST_SB)
    {
        // ['imm12hi', 'rs1', 'rs2', 'imm12lo']
        DPRINTF(LOG_INST,("%08x: sb %d(r%d), r%d\n", pc, storeimm, rs1, rs2));
        INST_STAT(ENUM_INST_SB);
        if (store(pc, reg_rs1 + storeimm, reg_rs2, 1))
            pc += 4;
        else
            return ;

        // No writeback
        rd = 0;
    }
    else if ((opcode & INST_SH_MASK) == INST_SH)
    {
        // ['imm12hi', 'rs1', 'rs2', 'imm12lo']
        DPRINTF(LOG_INST,("%08x: sh %d(r%d), r%d\n", pc, storeimm, rs1, rs2));
        INST_STAT(ENUM_INST_SH);
        if (store(pc, reg_rs1 + storeimm, reg_rs2, 2))
            pc += 4;
        else
            return ;

        // No writeback
        rd = 0;
    }
    else if ((opcode & INST_SW_MASK) == INST_SW)
    {
        // ['imm12hi', 'rs1', 'rs2', 'imm12lo']
        DPRINTF(LOG_INST,("%08x: sw %d(r%d), r%d\n", pc, storeimm, rs1, rs2));
        INST_STAT(ENUM_INST_SW);
        if (store(pc, reg_rs1 + storeimm, reg_rs2, 4))
            pc += 4;
        else
            return ;

        // No writeback
        rd = 0;
    }
    else if ((opcode & INST_MUL_MASK) == INST_MUL)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: mul r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_MUL);
        reg_rd = (int64_t)reg_rs1 * (int64_t)reg_rs2;
        pc += 4;        
    }
    else if ((opcode & INST_MULH_MASK) == INST_MULH)
    {
        // ['rd', 'rs1', 'rs2']
        long long res = ((long long) (int64_t)reg_rs1) * ((long long)(int64_t)reg_rs2);
        INST_STAT(ENUM_INST_MULH);
        DPRINTF(LOG_INST,("%08x: mulh r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        reg_rd = (int)(res >> 32);
        pc += 4;
    }
    else if ((opcode & INST_MULHSU_MASK) == INST_MULHSU)
    {
        // ['rd', 'rs1', 'rs2']
        long long res = ((long long) (int)reg_rs1) * ((unsigned long long)(unsigned)reg_rs2);
        INST_STAT(ENUM_INST_MULHSU);
        DPRINTF(LOG_INST,("%08x: mulhsu r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        reg_rd = (int)(res >> 32);
        pc += 4;
    }
    else if ((opcode & INST_MULHU_MASK) == INST_MULHU)
    {
        // ['rd', 'rs1', 'rs2']
        unsigned long long res = ((unsigned long long) (unsigned)reg_rs1) * ((unsigned long long)(unsigned)reg_rs2);
        INST_STAT(ENUM_INST_MULHU);
        DPRINTF(LOG_INST,("%08x: mulhu r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        reg_rd = (int)(res >> 32);
        pc += 4;
    }
    else if ((opcode & INST_DIV_MASK) == INST_DIV)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: div r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_DIV);
        if ((int64_t)reg_rs1 == INT64_MIN && (int64_t)reg_rs2 == -1)
            reg_rd = reg_rs1;
        else if (reg_rs2 != 0)
            reg_rd = (int64_t)reg_rs1 / (int64_t)reg_rs2;
        else
            reg_rd = (uint64_t)-1;
        pc += 4;        
    }
    else if ((opcode & INST_DIVU_MASK) == INST_DIVU)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: divu r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_DIVU);
        if (reg_rs2 != 0)
            reg_rd = (uint64_t)reg_rs1 / (uint64_t)reg_rs2;
        else
            reg_rd = (uint64_t)-1;
        pc += 4;        
    }
    else if ((opcode & INST_REM_MASK) == INST_REM)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: rem r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_REM);

        if((int64_t)reg_rs1 == INT64_MIN && (int64_t)reg_rs2 == -1)
            reg_rd = 0;
        else if (reg_rs2 != 0)
            reg_rd = (int64_t)reg_rs1 % (int64_t)reg_rs2;
        else
            reg_rd = reg_rs1;
        pc += 4;        
    }
    else if ((opcode & INST_REMU_MASK) == INST_REMU)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: remu r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_REMU);
        if (reg_rs2 != 0)
            reg_rd = (uint64_t)reg_rs1 % (uint64_t)reg_rs2;
        else
            reg_rd = reg_rs1;
        pc += 4;        
    }
    else if ((opcode & INST_ECALL_MASK) == INST_ECALL)
    {
        DPRINTF(LOG_INST,("%08x: ecall\n", pc));
        INST_STAT(ENUM_INST_ECALL);

        exception(MCAUSE_ECALL_U + m_csr_mpriv, pc);
        take_exception   = true;
    }
    else if ((opcode & INST_EBREAK_MASK) == INST_EBREAK)
    {
        DPRINTF(LOG_INST,("%08x: ebreak\n", pc));
        INST_STAT(ENUM_INST_EBREAK);

        exception(MCAUSE_BREAKPOINT, pc);
        take_exception   = true;
        m_break          = true;
    }
    else if ((opcode & INST_MRET_MASK) == INST_MRET)
    {
        DPRINTF(LOG_INST,("%08x: mret\n", pc));
        INST_STAT(ENUM_INST_MRET);

        assert(m_csr_mpriv == PRIV_MACHINE);

        uint32_t s        = m_csr_msr;
        uint32_t prev_prv = SR_GET_MPP(m_csr_msr);

        // Interrupt enable pop
        s &= ~SR_MIE;
        s |= (s & SR_MPIE) ? SR_MIE : 0;
        s |= SR_MPIE;

        // Set next MPP to user mode
        s &= ~SR_MPP;
        s |=  SR_MPP_U;

        // Set privilege level to previous MPP
        m_csr_mpriv   = prev_prv;
        m_csr_msr     = s;

        // Return to EPC
        pc          = m_csr_mepc;
    }
    else if ((opcode & INST_SRET_MASK) == INST_SRET)
    {
        DPRINTF(LOG_INST,("%08x: sret\n", pc));
        INST_STAT(ENUM_INST_SRET);

        assert(m_csr_mpriv == PRIV_SUPER);

        uint32_t s        = m_csr_msr;
        uint32_t prev_prv = (m_csr_msr & SR_SPP) ? PRIV_SUPER : PRIV_USER;

        // Interrupt enable pop
        s &= ~SR_SIE;
        s |= (s & SR_SPIE) ? SR_SIE : 0;
        s |= SR_SPIE;

        // Set next SPP to user mode
        s &= ~SR_SPP;

        // Set privilege level to previous MPP
        m_csr_mpriv   = prev_prv;
        m_csr_msr     = s;

        // Return to EPC
        pc          = m_csr_sepc;
    }
    else if ( ((opcode & INST_SFENCE_MASK) == INST_SFENCE) ||
              ((opcode & INST_FENCE_MASK) == INST_FENCE) ||
              ((opcode & INST_IFENCE_MASK) == INST_IFENCE))
    {
        DPRINTF(LOG_INST,("%08x: fence\n", pc));
        INST_STAT(ENUM_INST_FENCE);
        pc += 4;
    }
    else if ((opcode & INST_CSRRW_MASK) == INST_CSRRW)
    {
        DPRINTF(LOG_INST,("%08x: csrw r%d, r%d, 0x%x\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_CSRRW);
        take_exception = access_csr(imm12, reg_rs1, true, true, reg_rd);
        if (take_exception)
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc);
        else
            pc += 4;
    }    
    else if ((opcode & INST_CSRRS_MASK) == INST_CSRRS)
    {
        DPRINTF(LOG_INST,("%08x: csrs r%d, r%d, 0x%x\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_CSRRS);
        take_exception = access_csr(imm12, reg_rs1, (rs1 != 0), false, reg_rd);
        if (take_exception)
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc);
        else
            pc += 4;
    }
    else if ((opcode & INST_CSRRC_MASK) == INST_CSRRC)
    {
        DPRINTF(LOG_INST,("%08x: csrc r%d, r%d, 0x%x\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_CSRRC);
        take_exception = access_csr(imm12, reg_rs1, false, (rs1 != 0), reg_rd);
        if (take_exception)
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc);
        else
            pc += 4;
    }
    else if ((opcode & INST_CSRRWI_MASK) == INST_CSRRWI)
    {
        DPRINTF(LOG_INST,("%08x: csrwi r%d, %d, 0x%x\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_CSRRWI);
        take_exception = access_csr(imm12, rs1, true, true, reg_rd);
        if (take_exception)
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc);
        else
            pc += 4;
    }
    else if ((opcode & INST_CSRRSI_MASK) == INST_CSRRSI)
    {
        DPRINTF(LOG_INST,("%08x: csrsi r%d, %d, 0x%x\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_CSRRSI);
        take_exception = access_csr(imm12, rs1, (rs1 != 0), false, reg_rd);
        if (take_exception)
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc);
        else
            pc += 4;
    }
    else if ((opcode & INST_CSRRCI_MASK) == INST_CSRRCI)
    {
        DPRINTF(LOG_INST,("%08x: csrci r%d, %d, 0x%x\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_CSRRCI);
        take_exception = access_csr(imm12, rs1, false, (rs1 != 0), reg_rd);
        if (take_exception)
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc);
        else
            pc += 4;
    }
    else if ((opcode & INST_WFI_MASK) == INST_WFI)
    {
        DPRINTF(LOG_INST,("%08x: wfi\n", pc));
        INST_STAT(ENUM_INST_WFI);
        pc += 4;
    }
    else if ((opcode & INST_SD_MASK) == INST_SD)
    {
        // ['imm12hi', 'rs1', 'rs2', 'imm12lo']
        DPRINTF(LOG_INST,("%08x: sd %d(r%d), r%d\n", pc, storeimm, rs1, rs2));
        if (store(pc, reg_rs1 + storeimm, reg_rs2, 8))
            pc += 4;
        else
            return ;

        INST_STAT(ENUM_INST_SD);

        // No writeback
        rd = 0;
    }
    else if ((opcode & INST_LD_MASK) == INST_LD)
    {        
        // ['rd', 'rs1', 'imm12']
        DPRINTF(LOG_INST,("%08x: ld r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        if (load(pc, reg_rs1 + imm12, &reg_rd, 8, true))
            pc += 4;
        else
            return;

        INST_STAT(ENUM_INST_LD);
    }
    else if ((opcode & INST_ADDIW_MASK) == INST_ADDIW)
    {
        // ['rd', 'rs1', 'imm12']
        INST_STAT(ENUM_INST_ADDIW);
        reg_rd = SEXT32(reg_rs1 + imm12);
        pc += 4;
        DPRINTF(LOG_INST,("%08x: addiw r%d, r%d, %d\n", pc, rd, rs1, imm12));
    }
    else if ((opcode & INST_ADDW_MASK) == INST_ADDW)
    {
        // ['rd', 'rs1', 'rs2']
        INST_STAT(ENUM_INST_ADDW);
        reg_rd = SEXT32(reg_rs1 + reg_rs2);
        pc += 4;
        DPRINTF(LOG_INST,("%08x: addw r%d, r%d, r%d\n", pc, rd, rs1, rs2));
    }
    else if ((opcode & INST_SUBW_MASK) == INST_SUBW)
    {
        // ['rd', 'rs1', 'rs2']
        INST_STAT(ENUM_INST_SUBW);
        reg_rd = SEXT32(reg_rs1 - reg_rs2);
        pc += 4;
        DPRINTF(LOG_INST,("%08x: subw r%d, r%d, r%d\n", pc, rd, rs1, rs2));
    }
    else if ((opcode & INST_SLLIW_MASK) == INST_SLLIW)
    {
        reg_rs1 &= 0xFFFFFFFF;
        shamt &= SHIFT_MASK32;

        // ['rd', 'rs1']        
        INST_STAT(ENUM_INST_SLLIW);
        reg_rd = SEXT32(reg_rs1 << shamt);
        pc += 4;
        DPRINTF(LOG_INST,("%08x: slliw r%d, r%d, %d\n", pc, rd, rs1, shamt));
    }
    else if ((opcode & INST_SRLIW_MASK) == INST_SRLIW)
    {
        shamt &= SHIFT_MASK32;
        reg_rs1 &= 0xFFFFFFFF;

        // ['rd', 'rs1', 'shamt']        
        INST_STAT(ENUM_INST_SRLIW);
        reg_rd = SEXT32((uint64_t)reg_rs1 >> shamt);
        pc += 4;
        DPRINTF(LOG_INST,("%08x: srliw r%d, r%d, %d\n", pc, rd, rs1, shamt));
    }
    else if ((opcode & INST_SRAIW_MASK) == INST_SRAIW)
    {
        reg_rs1 &= 0xFFFFFFFF;
        shamt &= SHIFT_MASK32;

        // ['rd', 'rs1', 'shamt']        
        INST_STAT(ENUM_INST_SRAIW);
        reg_rd = SEXT32((int32_t)reg_rs1 >> shamt);
        pc += 4;
        DPRINTF(LOG_INST,("%08x: sraiw r%d, r%d, %d\n", pc, rd, rs1, shamt));
    }
    else if ((opcode & INST_SLLW_MASK) == INST_SLLW)
    {
        reg_rs2 &= SHIFT_MASK32;

        // ['rd', 'rs1', 'rs2']        
        INST_STAT(ENUM_INST_SLLW);
        reg_rd = SEXT32(reg_rs1 << reg_rs2);
        pc += 4;
        DPRINTF(LOG_INST,("%08x: sllw r%d, r%d, r%d\n", pc, rd, rs1, rs2));
    }
    else if ((opcode & INST_SRLW_MASK) == INST_SRLW)
    {
        reg_rs1 &= 0xFFFFFFFF;
        reg_rs2 &= SHIFT_MASK32;

        // ['rd', 'rs1', 'rs2']
        INST_STAT(ENUM_INST_SRLW);
        reg_rd = SEXT32((uint64_t)reg_rs1 >> reg_rs2);
        pc += 4;
        DPRINTF(LOG_INST,("%08x: srlw r%d, r%d, r%d\n", pc, rd, rs1, rs2));
    }
    else if ((opcode & INST_SRAW_MASK) == INST_SRAW)
    {
        reg_rs2 &= SHIFT_MASK32;

        // ['rd', 'rs1', 'rs2']        
        INST_STAT(ENUM_INST_SRAW);
        reg_rd = SEXT32((int64_t)reg_rs1 >> reg_rs2);
        pc += 4;
        DPRINTF(LOG_INST,("%08x: sraw r%d, r%d, r%d\n", pc, rd, rs1, rs2));
    }
    else
    {
        error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
        exception(MCAUSE_ILLEGAL_INSTRUCTION, pc);
        m_fault        = true;
        take_exception = true;
    }

    if (rd != 0 && !take_exception)
        m_gpr[rd] = reg_rd;

    // Pending interrupt
    if (!take_exception && (m_csr_mip & m_csr_mie))
    {
        uint32_t pending_interrupts = (m_csr_mip & m_csr_mie);
        uint32_t m_enabled          = m_csr_mpriv < PRIV_MACHINE || (m_csr_mpriv == PRIV_MACHINE && (m_csr_msr & SR_MIE));
        uint32_t s_enabled          = m_csr_mpriv < PRIV_SUPER   || (m_csr_mpriv == PRIV_SUPER   && (m_csr_msr & SR_SIE));
        uint32_t m_interrupts       = pending_interrupts & ~m_csr_mideleg & -m_enabled;
        uint32_t s_interrupts       = pending_interrupts & m_csr_mideleg & -s_enabled;
        uint32_t interrupts         = m_interrupts ? m_interrupts : s_interrupts;

        // Interrupt pending and mask enabled
        if (interrupts)
        {
            //printf("Take Interrupt...: %08x\n", interrupts);
            int i;

            for (i=IRQ_MIN;i<IRQ_MAX;i++)
            {
                if (interrupts & (1 << i))
                {
                    // Only service one interrupt per cycle
                    DPRINTF(LOG_INST,( "Interrupt%d taken...\n", i));
                    exception(MCAUSE_INTERRUPT + i, pc);
                    take_exception = true;
                    break;
                }
            }
        }
    }

    if (!take_exception)
        m_pc = pc;
}
//-----------------------------------------------------------------
// step: Step through one instruction
//-----------------------------------------------------------------
void rv64::step(void)
{
    m_stats[STATS_INSTRUCTIONS]++;

    // Execute instruction at current PC
    execute();

    // Increment timer counter
    m_csr_mtime++;

    // Timer should generate a interrupt?
    if (!m_compliant_csr)
    {
        // Limited internal timer, truncate to 32-bits
        m_csr_mtime &= 0xFFFFFFFF;
        if (m_csr_mtime == m_csr_mtimecmp)
            m_csr_mip |= (SR_IP_STIP | SR_IP_MTIP);
    }

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
void rv64::set_interrupt(int irq)
{
    assert(irq == 0);
    m_csr_mip |= (SR_IP_MEIP | SR_IP_SEIP);
}
//-----------------------------------------------------------------
// stats_reset: Reset runtime stats
//-----------------------------------------------------------------
void rv64::stats_reset(void)
{
    // Clear stats
    for (int i=STATS_MIN;i<STATS_MAX;i++)
        m_stats[i] = 0;
}
//-----------------------------------------------------------------
// stats_dump: Show execution stats
//-----------------------------------------------------------------
void rv64::stats_dump(void)
{  
    printf( "Runtime Stats:\n");
    printf( "- Total Instructions %d\n", m_stats[STATS_INSTRUCTIONS]);
    if (m_stats[STATS_INSTRUCTIONS] > 0)
    {
        printf( "- Loads %d (%d%%)\n",  m_stats[STATS_LOADS],  (m_stats[STATS_LOADS] * 100)  / m_stats[STATS_INSTRUCTIONS]);
        printf( "- Stores %d (%d%%)\n", m_stats[STATS_STORES], (m_stats[STATS_STORES] * 100) / m_stats[STATS_INSTRUCTIONS]);
        printf( "- Branches Operations %d (%d%%)\n", m_stats[STATS_BRANCHES], (m_stats[STATS_BRANCHES] * 100)  / m_stats[STATS_INSTRUCTIONS]);
    }

    stats_reset();
}
