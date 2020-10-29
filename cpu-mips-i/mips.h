//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __MIPS_H__
#define __MIPS_H__

#include <stdint.h>
#include <vector>
#include "memory.h"
#include "cpu.h"

//--------------------------------------------------------------------
// Class
//--------------------------------------------------------------------
class mips_i: public cpu
{
public:
                        mips_i(uint32_t baseAddr = 0, uint32_t len = 0);

    void                reset(uint32_t start_addr);
    uint32_t            get_opcode(uint32_t pc);
    void                step(void);

    void                set_interrupt(int irq);
    void                clr_interrupt(int irq) { }

    uint32_t            get_register(int reg);
    uint32_t            get_pc() { return m_pc_x; }
    uint32_t            get_opcode(void)  { return get_opcode(m_pc_x); }
    int                 get_num_reg(void) { return 32; }

    // First register for args in ABI
    int                 get_abi_reg_arg0(void) { return 4; }

    // Get return register
    int                 get_abi_reg_ret(void) { return 2; }

    // Number of registers (max) used for args
    int                 get_abi_reg_num(void) { return 4; }

    void                set_register(int reg, uint32_t val);
    void                set_pc(uint32_t val) { m_pc = val; m_pc_x = m_pc; }

    void                stats_reset(void);
    void                stats_dump(void);

    void                enable_mem_errors(bool en) { m_enable_mem_errors = en; }
    void                set_big_endian(bool be) { m_big_endian = be; }

protected:  
    bool                execute(void);
    int                 load(uint32_t pc, uint32_t address, uint32_t *result, int width, bool signedLoad);
    int                 store(uint32_t pc, uint32_t address, uint32_t data, uint8_t mask);
    void                exception(uint32_t cause, uint32_t pc, uint32_t badaddr = 0);

    virtual int         copro0_inst(uint32_t pc, uint32_t opc, uint32_t reg_rs, uint32_t reg_rt, int &wb_reg, uint32_t &result);
    virtual int         copro_inst(int cop,  uint32_t pc, uint32_t opc, uint32_t reg_rs, uint32_t reg_rt, int &wb_reg, uint32_t &result);

private:

    // Other registers
    uint32_t           m_pc;
    uint32_t           m_pc_next;
    uint32_t           m_pc_x;
    uint32_t           m_epc;
    uint32_t           m_status;
    uint32_t           m_cause;
    uint32_t           m_badaddr;
    uint32_t           m_hi;
    uint32_t           m_lo;
    uint32_t           m_isr_vector;
    bool               m_big_endian;
    bool               m_branch_ds;
    bool               m_enable_mem_errors;
    uint32_t           m_cycles;
    uint32_t           m_gpr[32];

    enum eStatsMips
    { 
        STATS_MIN,
        STATS_INSTRUCTIONS = STATS_MIN,
        STATS_LOADS,
        STATS_STORES,
        STATS_BRANCHES,
        STATS_COPRO,
        STATS_NOP,
        STATS_EXCEPTIONS,
        STATS_MAX
    };

    // Stats
    uint32_t            m_stats[STATS_MAX];
};

#endif
