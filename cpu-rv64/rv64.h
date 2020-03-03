//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __RV64_H__
#define __RV64_H__

#include <stdint.h>
#include <vector>
#include "memory.h"
#include "cpu.h"

//--------------------------------------------------------------------
// rv64: RV64IM model
//--------------------------------------------------------------------
class rv64: public cpu
{
public:
                        rv64(uint32_t baseAddr = 0, uint32_t len = 0);

    void                reset(uint32_t start_addr);
    uint32_t            get_opcode(uint64_t pc);
    void                step(void);

    void                set_interrupt(int irq);
    void                clr_interrupt(int irq);

    uint32_t            get_register(int r);
    uint64_t            get_register64(int r);

    uint32_t            get_pc(void)        { return m_pc_x; }
    uint64_t            get_pc64(void)      { return m_pc_x; }
    uint32_t            get_opcode(void)    { return get_opcode(m_pc_x); }
    int                 get_num_reg(void)   { return 32; }
    int                 get_reg_width(void) { return 64; }

    void                set_register(int r, uint32_t val);
    void                set_register(int r, uint64_t val);
    void                set_pc(uint32_t val);
    void                set_pc(uint64_t val);

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

    enum eStats
    { 
        STATS_MIN,
        STATS_INSTRUCTIONS = STATS_MIN,
        STATS_LOADS,
        STATS_STORES,
        STATS_BRANCHES,
        STATS_MAX
    };    

protected:  
    void                execute(void);
    int                 load(uint64_t pc, uint64_t address, uint64_t *result, int width, bool signedLoad);
    int                 store(uint64_t pc, uint64_t address, uint64_t data, int width);
    virtual bool        access_csr(uint64_t address, uint64_t data, bool set, bool clr, uint64_t &result);
    void                exception(uint64_t cause, uint64_t pc, uint64_t badaddr = 0);

// MMU
private:
    int                 mmu_read_word(uint64_t address, uint64_t *val);
    uint64_t            mmu_walk(uint64_t addr);
    int                 mmu_i_translate(uint64_t addr, uint64_t *physical);
    int                 mmu_d_translate(uint64_t pc, uint64_t addr, uint64_t *physical, int writeNotRead);

private:

    // CPU Registers
    uint64_t            m_gpr[32];
    uint64_t            m_pc;
    uint64_t            m_pc_x;
    uint64_t            m_load_res;

    // CSR - Machine
    uint64_t            m_csr_mepc;
    uint64_t            m_csr_mcause;
    uint64_t            m_csr_msr;
    uint64_t            m_csr_mpriv;
    uint64_t            m_csr_mevec;
    uint64_t            m_csr_mtval;
    uint64_t            m_csr_mie;
    uint64_t            m_csr_mip;
    uint64_t            m_csr_mtime;
    uint64_t            m_csr_mtimecmp;
    bool                m_csr_mtime_ie;
    uint64_t            m_csr_mscratch;
    uint64_t            m_csr_mideleg;
    uint64_t            m_csr_medeleg;

    // CSR - Supervisor
    uint64_t            m_csr_sepc;
    uint64_t            m_csr_sevec;
    uint64_t            m_csr_scause;
    uint64_t            m_csr_stval;
    uint64_t            m_csr_satp;
    uint64_t            m_csr_sscratch;

    // TLB cache
    static const int MMU_TLB_ENTRIES = 64;
    uint64_t            m_mmu_addr[MMU_TLB_ENTRIES];
    uint64_t            m_mmu_pte[MMU_TLB_ENTRIES];

    // Settings
    bool                m_enable_mem_errors;
    bool                m_compliant_csr;
    bool                m_enable_rvm;
    bool                m_enable_rvc;
    bool                m_enable_rva;
    bool                m_enable_mtimecmp;

    // Stats
    uint32_t            m_stats[STATS_MAX];
};

#endif
