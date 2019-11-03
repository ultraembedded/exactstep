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
    m_enable_rvc         = true;
    m_enable_rva         = true;

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
    m_load_res  = 0;

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
    // 2 byte aligned addresses are supported if RVC
    if (m_enable_rvc && (address & 2))
    {
        uint32_t opcode = read32(address & ~3);
        opcode >>= 16;
        return (read32((address+4) & ~3) << 16) | opcode;
    }
    else
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

    uint64_t misa_val = MISA_VALUE;
    misa_val |= m_enable_rvc ? MISA_RVC : 0;
    misa_val |= m_enable_rva ? MISA_RVA : 0;

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
                    // Abnormal exit
                    if (data & 0xFF)
                        exit(data & 0xFF);
                    else
                        m_stopped = true;
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
        CSR_CONST(MISA,  misa_val)
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
    uint64_t deleg;
    uint64_t bit;

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
        uint64_t s = m_csr_msr;

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
        uint64_t s = m_csr_msr;

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

        uint64_t s        = m_csr_msr;
        uint64_t prev_prv = SR_GET_MPP(m_csr_msr);

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

        uint64_t s        = m_csr_msr;
        uint64_t prev_prv = (m_csr_msr & SR_SPP) ? PRIV_SUPER : PRIV_USER;

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
    else if ((opcode & INST_MULW_MASK) == INST_MULW)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: mulw r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_MULW);
        reg_rd = SEXT32((int64_t)reg_rs1 * (int64_t)reg_rs2);
        pc += 4;        
    }
    else if ((opcode & INST_DIVW_MASK) == INST_DIVW)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: divw r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_DIVW);
        if ((int64_t)(int32_t)reg_rs2 != 0)
            reg_rd = SEXT32((int64_t)(int32_t)reg_rs1 / (int64_t)(int32_t)reg_rs2);
        else
            reg_rd = (uint64_t)-1;
        pc += 4;        
    }
    else if ((opcode & INST_DIVUW_MASK) == INST_DIVUW)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: divuw r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_DIVUW);
        if ((uint32_t)reg_rs2 != 0)
            reg_rd = SEXT32((uint32_t)reg_rs1 / (uint32_t)reg_rs2);
        else
            reg_rd = (uint64_t)-1;
        pc += 4;        
    }
    else if ((opcode & INST_REMW_MASK) == INST_REMW)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: remw r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_REMW);

        if ((int64_t)(int32_t)reg_rs2 != 0)
            reg_rd = SEXT32((int64_t)(int32_t)reg_rs1 % (int64_t)(int32_t)reg_rs2);
        else
            reg_rd = reg_rs1;
        pc += 4;
    }
    else if ((opcode & INST_REMUW_MASK) == INST_REMUW)
    {
        // ['rd', 'rs1', 'rs2']
        DPRINTF(LOG_INST,("%08x: remuw r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_REMUW);

        if ((uint32_t)reg_rs2 != 0)
            reg_rd = SEXT32((uint32_t)reg_rs1 % (uint32_t)reg_rs2);
        else
            reg_rd = reg_rs1;
        pc += 4;
    }
    //-----------------------------------------------------------------
    // A Extension
    //-----------------------------------------------------------------
    else if (m_enable_rva && (opcode & INST_AMOADD_W_MASK) == INST_AMOADD_W)
    {
        DPRINTF(LOG_INST,("%08x: amoadd.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return;

        // Modify
        uint32_t val = reg_rd + reg_rs2;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return ;

        INST_STAT(ENUM_INST_ADD);
        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOXOR_W_MASK) == INST_AMOXOR_W)
    {
        DPRINTF(LOG_INST,("%08x: amoxor.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return;

        // Modify
        uint32_t val = reg_rd ^ reg_rs2;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return ;

        INST_STAT(ENUM_INST_XOR);
        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOOR_W_MASK) == INST_AMOOR_W)
    {
        DPRINTF(LOG_INST,("%08x: amoor.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return;

        // Modify
        uint32_t val = reg_rd | reg_rs2;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return ;

        INST_STAT(ENUM_INST_OR);
        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOAND_W_MASK) == INST_AMOAND_W)
    {
        DPRINTF(LOG_INST,("%08x: amoand.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return;

        // Modify
        uint32_t val = reg_rd & reg_rs2;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return ;

        INST_STAT(ENUM_INST_AND);
        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOMIN_W_MASK) == INST_AMOMIN_W)
    {
        DPRINTF(LOG_INST,("%08x: amomin.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return;

        // Modify
        uint32_t val = reg_rs2;
        if ((int32_t)reg_rd < (int32_t)reg_rs2)
            val = reg_rd;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return ;

        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOMAX_W_MASK) == INST_AMOMAX_W)
    {
        DPRINTF(LOG_INST,("%08x: amomax.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return;

        // Modify
        uint32_t val = reg_rs2;
        if ((int32_t)reg_rd > (int32_t)reg_rs2)
            val = reg_rd;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return ;

        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOMINU_W_MASK) == INST_AMOMINU_W)
    {
        DPRINTF(LOG_INST,("%08x: amominu.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return;

        // Modify
        uint32_t val = reg_rs2;
        if ((uint32_t)reg_rd < (uint32_t)reg_rs2)
            val = reg_rd;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return ;

        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOMAXU_W_MASK) == INST_AMOMAXU_W)
    {
        DPRINTF(LOG_INST,("%08x: amomaxu.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return;

        // Modify
        uint32_t val = reg_rs2;
        if ((uint32_t)reg_rd > (uint32_t)reg_rs2)
            val = reg_rd;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return ;

        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOSWAP_W_MASK) == INST_AMOSWAP_W)
    {
        DPRINTF(LOG_INST,("%08x: amoswap.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return;

        // Write
        if (!store(pc, reg_rs1, reg_rs2, 4))
            return ;

        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_LR_W_MASK) == INST_LR_W)
    {
        DPRINTF(LOG_INST,("%08x: lr.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return;

        // Record load address
        m_load_res = reg_rs1;

        INST_STAT(ENUM_INST_LW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_SC_W_MASK) == INST_SC_W)
    {
        DPRINTF(LOG_INST,("%08x: sc.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        if (m_load_res == reg_rs1)
        {
            // Write
            if (!store(pc, reg_rs1, reg_rs2, 4))
                return ;

            reg_rd = 0;
        }
        else
            reg_rd = 1;

        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOADD_D_MASK) == INST_AMOADD_D)
    {
        DPRINTF(LOG_INST,("%08x: amoadd.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 8, true))
            return;

        // Modify
        uint64_t val = reg_rd + reg_rs2;

        // Write
        if (!store(pc, reg_rs1, val, 8))
            return ;

        INST_STAT(ENUM_INST_ADD);
        INST_STAT(ENUM_INST_LD);
        INST_STAT(ENUM_INST_SD);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOXOR_D_MASK) == INST_AMOXOR_D)
    {
        DPRINTF(LOG_INST,("%08x: amoxor.d r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 8, true))
            return;

        // Modify
        uint64_t val = reg_rd ^ reg_rs2;

        // Write
        if (!store(pc, reg_rs1, val, 8))
            return ;

        INST_STAT(ENUM_INST_XOR);
        INST_STAT(ENUM_INST_LD);
        INST_STAT(ENUM_INST_SD);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOOR_D_MASK) == INST_AMOOR_D)
    {
        DPRINTF(LOG_INST,("%08x: amoor.d r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 8, true))
            return;

        // Modify
        uint64_t val = reg_rd | reg_rs2;

        // Write
        if (!store(pc, reg_rs1, val, 8))
            return ;

        INST_STAT(ENUM_INST_OR);
        INST_STAT(ENUM_INST_LD);
        INST_STAT(ENUM_INST_SD);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOAND_D_MASK) == INST_AMOAND_D)
    {
        DPRINTF(LOG_INST,("%08x: amoand.d r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 8, true))
            return;

        // Modify
        uint64_t val = reg_rd & reg_rs2;

        // Write
        if (!store(pc, reg_rs1, val, 8))
            return ;

        INST_STAT(ENUM_INST_AND);
        INST_STAT(ENUM_INST_LD);
        INST_STAT(ENUM_INST_SD);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOMIN_D_MASK) == INST_AMOMIN_D)
    {
        DPRINTF(LOG_INST,("%08x: amomin.d r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 8, true))
            return;

        // Modify
        uint64_t val = reg_rs2;
        if ((int64_t)reg_rd < (int64_t)reg_rs2)
            val = reg_rd;

        // Write
        if (!store(pc, reg_rs1, val, 8))
            return ;

        INST_STAT(ENUM_INST_LD);
        INST_STAT(ENUM_INST_SD);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOMAX_D_MASK) == INST_AMOMAX_D)
    {
        DPRINTF(LOG_INST,("%08x: amomax.d r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 8, true))
            return;

        // Modify
        uint64_t val = reg_rs2;
        if ((int64_t)reg_rd > (int64_t)reg_rs2)
            val = reg_rd;

        // Write
        if (!store(pc, reg_rs1, val, 8))
            return ;

        INST_STAT(ENUM_INST_LD);
        INST_STAT(ENUM_INST_SD);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOMINU_D_MASK) == INST_AMOMINU_D)
    {
        DPRINTF(LOG_INST,("%08x: amominu.d r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 8, true))
            return;

        // Modify
        uint64_t val = reg_rs2;
        if ((uint64_t)reg_rd < (uint64_t)reg_rs2)
            val = reg_rd;

        // Write
        if (!store(pc, reg_rs1, val, 8))
            return ;

        INST_STAT(ENUM_INST_LD);
        INST_STAT(ENUM_INST_SD);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOMAXU_D_MASK) == INST_AMOMAXU_D)
    {
        DPRINTF(LOG_INST,("%08x: amomaxu.d r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 8, true))
            return;

        // Modify
        uint64_t val = reg_rs2;
        if ((uint64_t)reg_rd > (uint64_t)reg_rs2)
            val = reg_rd;

        // Write
        if (!store(pc, reg_rs1, val, 8))
            return ;

        INST_STAT(ENUM_INST_LD);
        INST_STAT(ENUM_INST_SD);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOSWAP_D_MASK) == INST_AMOSWAP_D)
    {
        DPRINTF(LOG_INST,("%08x: amoswap.d r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 8, true))
            return;

        // Write
        if (!store(pc, reg_rs1, reg_rs2, 8))
            return ;

        INST_STAT(ENUM_INST_LD);
        INST_STAT(ENUM_INST_SD);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_LR_D_MASK) == INST_LR_D)
    {
        DPRINTF(LOG_INST,("%08x: lr.d r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        if (!load(pc, reg_rs1, &reg_rd, 8, true))
            return;

        // Record load address
        m_load_res = reg_rs1;

        INST_STAT(ENUM_INST_LD);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_SC_D_MASK) == INST_SC_D)
    {
        DPRINTF(LOG_INST,("%08x: sc.d r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        if (m_load_res == reg_rs1)
        {
            // Write
            if (!store(pc, reg_rs1, reg_rs2, 8))
                return ;

            reg_rd = 0;
        }
        else
            reg_rd = 1;

        INST_STAT(ENUM_INST_SD);
        pc += 4;
    }
    //-----------------------------------------------------------------
    // C Extension
    //-----------------------------------------------------------------
    // RVC - Quadrant 0
    else if (m_enable_rvc && ((opcode & 3) == 0))
    {
        opcode &= 0xFFFF;
        rvc_decode rvc(opcode);

        rs1     = rvc.rs1s();
        rs2     = rvc.rs2s();
        rd      = rs2;
        reg_rs1 = m_gpr[rs1];
        reg_rs2 = m_gpr[rs2];        

        // C.ADDI4SPN
        if ((opcode >> 13) == 0 && opcode != 0)
        {
            rs1     = RISCV_REG_SP;
            reg_rs1 = m_gpr[rs1];

            uint64_t imm = rvc.addi4spn_imm();
            DPRINTF(LOG_INST,("%08x: c.addi4spn r%d,r%d,%d\n", pc, rd, rs1, imm));
            INST_STAT(ENUM_INST_ADDI);
            reg_rd = reg_rs1 + imm;
            pc += 2;
        }
        // C.LW
        else if ((opcode >> 13) == 2)
        {
            uint64_t imm = rvc.lw_imm();
            DPRINTF(LOG_INST,("%08x: c.lw r%d, %d(r%d)\n", pc, rd, imm, rs1));
            INST_STAT(ENUM_INST_LW);
            if (load(pc, reg_rs1 + imm, &reg_rd, 4, true))
                pc += 2;
            else
                return;
        }
        // C.LD
        else if ((opcode >> 13) == 3)
        {
            uint64_t imm = rvc.ld_imm();
            DPRINTF(LOG_INST,("%08x: c.ld r%d, %d(r%d)\n", pc, rd, imm, rs1));
            INST_STAT(ENUM_INST_LD);
            if (load(pc, reg_rs1 + imm, &reg_rd, 8, true))
                pc += 2;
            else
                return;
        }
        // C.SW
        else if ((opcode >> 13) == 6)
        {
            uint64_t imm = rvc.lw_imm();
            DPRINTF(LOG_INST,("%08x: c.sw %d(r%d), r%d\n", pc, imm, rs1, rd));
            INST_STAT(ENUM_INST_SW);
            if (store(pc, reg_rs1 + imm, reg_rs2, 4))
                pc += 2;
            else
                return ;

            // No writeback
            rd = 0;
        }
        // C.SD
        else if ((opcode >> 13) == 7)
        {
            uint64_t imm = rvc.ld_imm();
            DPRINTF(LOG_INST,("%08x: c.sd %d(r%d), r%d\n", pc, imm, rs1, rd));
            INST_STAT(ENUM_INST_SD);
            if (store(pc, reg_rs1 + imm, reg_rs2, 8))
                pc += 2;
            else
                return ;

            // No writeback
            rd = 0;
        }
        // Illegal instruction
        else
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc);
            m_fault        = true;
            take_exception = true;
        }
    }
    // RVC - Quadrant 1 (top half - c.nop - c.lui)
    else if (m_enable_rvc && ((opcode & 3) == 1) && (((opcode & 0xFFFF) >> 13) < 4))
    {
        opcode &= 0xFFFF;
        rvc_decode rvc(opcode);

        rs1     = rvc.rs1();
        rs2     = rvc.rs2();
        rd      = rs1;
        reg_rs1 = m_gpr[rs1];
        reg_rs2 = m_gpr[rs2];

        // C.NOP
        if ((opcode >> 13) == 0 && opcode == 0)
        {
            DPRINTF(LOG_INST,("%08x: c.nop\n", pc));
            INST_STAT(ENUM_INST_ADDI);
            pc += 2;
        }
        // C.ADDI
        else if ((opcode >> 13) == 0)
        {
            int64_t imm = rvc.imm();
            DPRINTF(LOG_INST,("%08x: c.addi r%d, %d\n", pc, rs1, imm));
            INST_STAT(ENUM_INST_ADDI);
            reg_rd = reg_rs1 + imm;
            pc += 2;
        }
        // C.ADDIW
        else if ((opcode >> 13) == 1)
        {
            int64_t imm = rvc.imm();
            DPRINTF(LOG_INST,("%08x: c.addiw r%d, %d\n", pc, rs1, imm));
            INST_STAT(ENUM_INST_ADDIW);
            reg_rd = SEXT32(reg_rs1 + imm);
            pc += 2;
        }
        // C.LI
        else if ((opcode >> 13) == 2)
        {
            uint64_t imm = rvc.imm();
            DPRINTF(LOG_INST,("%08x: c.li r%d, %d\n", pc, rd, imm));
            INST_STAT(ENUM_INST_LUI);
            reg_rd = imm;
            pc += 2;
        }
        // C.ADDI16SP
        else if ((opcode >> 13) == 3 && ((opcode >> 7) & 0x1f) == 2)
        {
            uint64_t imm = rvc.addi16sp_imm();
            rd      = RISCV_REG_SP;
            rs1     = RISCV_REG_SP;
            reg_rs1 = m_gpr[rs1];
            DPRINTF(LOG_INST,("%08x: c.addi16sp r%d, r%d, %d\n", pc, rd, rs1, imm));
            INST_STAT(ENUM_INST_ADDI);
            reg_rd  = reg_rs1 + imm;
            pc += 2;
        }
        // C.LUI
        else if ((opcode >> 13) == 3)
        {
            uint64_t imm = rvc.imm() << 12;
            DPRINTF(LOG_INST,("%08x: c.lui r%d, 0x%x\n", pc, rd, imm));
            INST_STAT(ENUM_INST_LUI);
            reg_rd  = imm;
            pc += 2;
        }
        // Illegal instruction
        else
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc);
            m_fault        = true;
            take_exception = true;
        }
    }
    // RVC - Quadrant 1 (bottom half - c.srli -)
    else if (m_enable_rvc && ((opcode & 3) == 1))
    {
        opcode &= 0xFFFF;
        rvc_decode rvc(opcode);

        rs1     = rvc.rs1s();
        rs2     = rvc.rs2s();
        rd      = rs1;
        reg_rs1 = m_gpr[rs1];
        reg_rs2 = m_gpr[rs2];

        // C.SRLI
        if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x3) == 0)
        {
            uint64_t imm = rvc.zimm();
            DPRINTF(LOG_INST,("%08x: c.srli r%d, %d\n", pc, rd, imm));
            INST_STAT(ENUM_INST_SRLI);
            reg_rd = (uint64_t)reg_rs1 >> imm;
            pc += 2;
        }
        // C.SRAI
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x3) == 1)
        {
            uint64_t imm = rvc.zimm();
            DPRINTF(LOG_INST,("%08x: c.srai r%d, %d\n", pc, rd, imm));
            INST_STAT(ENUM_INST_SRAI);
            reg_rd = (int64_t)reg_rs1 >> imm;
            pc += 2;
        }
        // C.ANDI
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x3) == 2)
        {
            uint64_t imm = rvc.imm();
            DPRINTF(LOG_INST,("%08x: c.andi r%d, 0x%08x\n", pc, rd, imm));
            INST_STAT(ENUM_INST_ANDI);
            reg_rd = reg_rs1 & imm;
            pc += 2;
        }
        // C.SUB
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x7) == 3 && ((opcode >> 5) & 0x3) == 0)
        {
            DPRINTF(LOG_INST,("%08x: c.sub r%d, r%d\n", pc, rs1, rs2));
            INST_STAT(ENUM_INST_SUB);
            reg_rd = (int64_t)(reg_rs1 - reg_rs2);
            pc += 2;
        }
        // C.XOR
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x7) == 3 && ((opcode >> 5) & 0x3) == 1)
        {
            DPRINTF(LOG_INST,("%08x: c.xor r%d, r%d\n", pc, rs1, rs2));
            INST_STAT(ENUM_INST_XOR);
            reg_rd = reg_rs1 ^ reg_rs2;
            pc += 2;
        }
        // C.OR
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x7) == 3 && ((opcode >> 5) & 0x3) == 2)
        {
            DPRINTF(LOG_INST,("%08x: c.or r%d, r%d\n", pc, rs1, rs2));
            INST_STAT(ENUM_INST_OR);
            reg_rd = reg_rs1 | reg_rs2;
            pc += 2;
        }
        // C.AND
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x7) == 3 && ((opcode >> 5) & 0x3) == 3)
        {
            DPRINTF(LOG_INST,("%08x: c.and r%d, r%d\n", pc, rs1, rs2));
            INST_STAT(ENUM_INST_AND);
            reg_rd = reg_rs1 & reg_rs2;
            pc += 2;
        }
        // C.SUBW
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x7) == 7 && ((opcode >> 5) & 0x3) == 0)
        {
            DPRINTF(LOG_INST,("%08x: c.subw r%d, r%d\n", pc, rs1, rs2));
            INST_STAT(ENUM_INST_SUBW);
            reg_rd = SEXT32(reg_rs1 - reg_rs2);
            pc += 2;
        }
        // C.ADDW
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x7) == 7 && ((opcode >> 5) & 0x3) == 1)
        {
            DPRINTF(LOG_INST,("%08x: c.addw r%d, r%d, r%d\n", pc, rd, rs1, rs2));
            INST_STAT(ENUM_INST_ADDW);
            reg_rd = SEXT32(reg_rs1 + reg_rs2);
            pc += 2;
        }
        // C.J
        else if ((opcode >> 13) == 5)
        {
            uint64_t imm = rvc.j_imm();
            DPRINTF(LOG_INST,("%08x: c.j 0x%08x\n", pc, pc + imm));
            INST_STAT(ENUM_INST_J);
            pc += imm;
            rd = 0;
        }
        // C.BEQZ
        else if ((opcode >> 13) == 6)
        {
            uint64_t imm = rvc.b_imm();

            DPRINTF(LOG_INST,("%08x: c.beqz r%d, %d\n", pc, rs1, imm));
            INST_STAT(ENUM_INST_BEQ);
            if (reg_rs1 == 0)
                pc += imm;
            else
                pc += 2;
            rd = 0;
        }
        // C.BNEZ
        else if ((opcode >> 13) == 7)
        {
            uint64_t imm = rvc.b_imm();
            DPRINTF(LOG_INST,("%08x: c.bnez r%d, %d\n", pc, rs1, imm));
            INST_STAT(ENUM_INST_BNE);
            if (reg_rs1 != 0)
                pc += imm;
            else
                pc += 2;
            rd = 0;
        }
        // Illegal instruction
        else
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc);
            m_fault        = true;
            take_exception = true;
        }
    }
    // RVC - Quadrant 2
    else if (m_enable_rvc && ((opcode & 3) == 2))
    {
        opcode &= 0xFFFF;

        rvc_decode rvc(opcode);
        rs1     = rvc.rs1();
        rs2     = rvc.rs2();
        rd      = rs1;
        reg_rs1 = m_gpr[rs1];
        reg_rs2 = m_gpr[rs2];

        // C.SLLI
        if ((opcode >> 13) == 0)
        {
            uint64_t imm = rvc.zimm();
            DPRINTF(LOG_INST,("%08x: c.slli r%d, %d\n", pc, rs1, imm));
            INST_STAT(ENUM_INST_SLLI);
            reg_rd = reg_rs1 << imm;
            pc += 2;
        }
        // C.LWSP
        else if ((opcode >> 13) == 2)
        {
            uint64_t imm = rvc.lwsp_imm();
            rs1     = RISCV_REG_SP;
            reg_rs1 = m_gpr[rs1];

            DPRINTF(LOG_INST,("%08x: c.lwsp r%d, %d(r%d)\n", pc, rd, imm, rs1));
            INST_STAT(ENUM_INST_LW);
            if (load(pc, reg_rs1 + imm, &reg_rd, 4, true))
                pc += 2;
            else
                return;
        }
        // C.LDSP
        else if ((opcode >> 13) == 3)
        {
            uint64_t imm = rvc.ldsp_imm();
            rs1     = RISCV_REG_SP;
            reg_rs1 = m_gpr[rs1];

            DPRINTF(LOG_INST,("%08x: c.ldsp r%d, %d(r%d)\n", pc, rd, imm, rs1));
            INST_STAT(ENUM_INST_LW);
            if (load(pc, reg_rs1 + imm, &reg_rd, 8, true))
                pc += 2;
            else
                return;
        }
        // C.JR
        // C.MV
        // C.EBREAK
        // C.JALR
        // C.ADD
        else if ((opcode >> 13) == 4)
        {
            if (!(opcode & (1 << 12)))
            {
                // C.JR
                if (((opcode >> 2) & 0x1F) == 0)
                {
                    rd = 0;
                    DPRINTF(LOG_INST,("%08x: c.jr r%d\n", pc, rs1));
                    INST_STAT(ENUM_INST_J);
                    pc = reg_rs1 & ~1;
                }
                // C.MV
                else
                {
                    DPRINTF(LOG_INST,("%08x: c.mv r%d, r%d\n", pc, rd, rs2));
                    INST_STAT(ENUM_INST_ADD);
                    pc += 2;
                    reg_rd = reg_rs2;
                }
            }
            else
            {
                // C.EBREAK
                if (((opcode >> 7) & 0x1F) == 0 && ((opcode >> 2) & 0x1F) == 0)
                {
                    rd = 0;
                    DPRINTF(LOG_INST,("%08x: c.ebreak\n", pc));
                    INST_STAT(ENUM_INST_EBREAK);

                    exception(MCAUSE_BREAKPOINT, pc);
                    take_exception   = true;
                    m_break          = true;
                }
                // C.JALR
                else if (((opcode >> 2) & 0x1F) == 0)
                {
                    rd = RISCV_REG_RA;
                    DPRINTF(LOG_INST,("%08x: c.jalr r%d, r%d\n", pc, rd, rs1));
                    INST_STAT(ENUM_INST_JALR);
                    reg_rd = pc + 2;
                    pc     = reg_rs1 & ~1;
                }
                // C.ADD
                else
                {
                    DPRINTF(LOG_INST,("%08x: c.add r%d, r%d, r%d\n", pc, rd, rs1, rs2));
                    INST_STAT(ENUM_INST_ADD);
                    reg_rd = reg_rs1 + reg_rs2;
                    pc += 2;
                }
            }
        }
        // C.SWSP
        else if ((opcode >> 13) == 6)
        {
            uint64_t uimm = rvc.swsp_imm();
            rs1     = RISCV_REG_SP;
            reg_rs1 = m_gpr[rs1];
            DPRINTF(LOG_INST,("%08x: c.swsp r%d, %d(r%d)\n", pc, rs2, uimm, rs1));
            INST_STAT(ENUM_INST_SW);

            if (store(pc, reg_rs1 + uimm, reg_rs2, 4))
                pc += 2;
            else
                return ;

            // No writeback
            rd = 0;            
        }
        // C.SDSP
        else if ((opcode >> 13) == 7)
        {
            uint64_t uimm = rvc.sdsp_imm();
            rs1     = RISCV_REG_SP;
            reg_rs1 = m_gpr[rs1];
            DPRINTF(LOG_INST,("%08x: c.sdsp r%d, %d(r%d)\n", pc, rs2, uimm, rs1));
            INST_STAT(ENUM_INST_SW);

            if (store(pc, reg_rs1 + uimm, reg_rs2, 8))
                pc += 2;
            else
                return ;

            // No writeback
            rd = 0; 
        }
        // Illegal instruction
        else
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc);
            m_fault        = true;
            take_exception = true;
        }
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
        uint64_t pending_interrupts = (m_csr_mip & m_csr_mie);
        uint64_t m_enabled          = m_csr_mpriv < PRIV_MACHINE || (m_csr_mpriv == PRIV_MACHINE && (m_csr_msr & SR_MIE));
        uint64_t s_enabled          = m_csr_mpriv < PRIV_SUPER   || (m_csr_mpriv == PRIV_SUPER   && (m_csr_msr & SR_SIE));
        uint64_t m_interrupts       = pending_interrupts & ~m_csr_mideleg & -m_enabled;
        uint64_t s_interrupts       = pending_interrupts & m_csr_mideleg & -s_enabled;
        uint64_t interrupts         = m_interrupts ? m_interrupts : s_interrupts;

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
            DPRINTF(LOG_REGISTERS,( " %016lx %016lx %016lx %016lx\n", m_gpr[i+0], m_gpr[i+1], m_gpr[i+2], m_gpr[i+3]));
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
