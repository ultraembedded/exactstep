//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __RV32_H__
#define __RV32_H__

#include <stdint.h>
#include <vector>
#include "memory.h"
#include "cpu.h"

//--------------------------------------------------------------------
// rv32: RV32IM model
//--------------------------------------------------------------------
class rv32: public cpu
{
public:
                        rv32(uint32_t baseAddr = 0, uint32_t len = 0);

    void                reset(uint32_t start_addr);
    uint32_t            get_opcode(uint32_t pc);
    void                step(void);

    void                set_interrupt(int irq);
    void                clr_interrupt(int irq);

    uint32_t            get_register(int r);

    uint32_t            get_pc(void)      { return m_pc_x; }
    uint32_t            get_opcode(void)  { return get_opcode(m_pc_x); }
    int                 get_num_reg(void) { return 32; }

    void                set_register(int r, uint32_t val);
    void                set_pc(uint32_t val);

    void                stats_reset(void);
    void                stats_dump(void);

    void                enable_mem_errors(bool en) { m_enable_mem_errors = en; }
    void                enable_compliant_csr(bool en) { m_compliant_csr = en; }

    // First register for args in ABI
    int                 get_abi_reg_arg0(void) { return 10; }

    // Number of registers (max) used for args
    int                 get_abi_reg_num(void) { return 8; }

    // Enable / Disable ISA extensions
    void                enable_rvm(bool en) { m_enable_rvm = en; }
    void                enable_rvc(bool en) { m_enable_rvc = en; }
    void                enable_rva(bool en) { m_enable_rva = en; }

protected:  
    bool                execute(void);
    int                 load(uint32_t pc, uint32_t address, uint32_t *result, int width, bool signedLoad);
    int                 store(uint32_t pc, uint32_t address, uint32_t data, int width);
    virtual bool        access_csr(uint32_t address, uint32_t data, bool set, bool clr, uint32_t &result);
    void                exception(uint32_t cause, uint32_t pc, uint32_t badaddr = 0);

// MMU
private:
    int                 mmu_read_word(uint32_t address, uint32_t *val);
    uint32_t            mmu_walk(uint32_t addr);
    int                 mmu_i_translate(uint32_t addr, uint32_t *physical);
    int                 mmu_d_translate(uint32_t pc, uint32_t addr, uint32_t *physical, int writeNotRead);

private:

    // CPU Registers
    uint32_t            m_gpr[32];
    uint32_t            m_pc;
    uint32_t            m_pc_x;
    uint32_t            m_load_res;

    // CSR - Machine
    uint32_t            m_csr_mepc;
    uint32_t            m_csr_mcause;
    uint32_t            m_csr_msr;
    uint32_t            m_csr_mpriv;
    uint32_t            m_csr_mevec;
    uint32_t            m_csr_mtval;
    uint32_t            m_csr_mie;
    uint32_t            m_csr_mip;
    uint64_t            m_csr_mtime;
    uint32_t            m_csr_mtimecmp;
    bool                m_csr_mtime_ie;
    uint32_t            m_csr_mscratch;
    uint32_t            m_csr_mideleg;
    uint32_t            m_csr_medeleg;

    // CSR - Supervisor
    uint32_t            m_csr_sepc;
    uint32_t            m_csr_sevec;
    uint32_t            m_csr_scause;
    uint32_t            m_csr_stval;
    uint32_t            m_csr_satp;
    uint32_t            m_csr_sscratch;

    // TLB cache
    static const int MMU_TLB_ENTRIES = 64;
    uint32_t            m_mmu_addr[MMU_TLB_ENTRIES];
    uint32_t            m_mmu_pte[MMU_TLB_ENTRIES];

    // Settings
    bool                m_enable_mem_errors;
    bool                m_compliant_csr;
    bool                m_enable_rvm;
    bool                m_enable_rvc;
    bool                m_enable_rva;
    bool                m_enable_mtimecmp;

    // Stats
    enum eStats
    { 
        STATS_MIN,
        STATS_INSTRUCTIONS = STATS_MIN,
        STATS_LOADS,
        STATS_STORES,
        STATS_BRANCHES,
        STATS_MUL,
        STATS_DIV,
        STATS_MAX
    };
    uint32_t            m_stats[STATS_MAX];
};

#endif
