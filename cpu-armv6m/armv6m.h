//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __ARMV6M_H__
#define __ARMV6M_H__

#include <stdint.h>
#include <vector>
#include "memory.h"
#include "cpu.h"
#include "device_systick.h"

//--------------------------------------------------------------------
// armv6m: Simple ARM v6m model
//--------------------------------------------------------------------
class armv6m: public cpu
{
public:
                        armv6m(uint32_t baseAddr = 0, uint32_t len = 0);

    void                reset(uint32_t start_addr);
    uint32_t            get_opcode(uint32_t pc);
    void                step(void);

    void                set_interrupt(int irq);

    bool                get_reg_valid(int r) { return true; }
    uint32_t            get_register(int r);

    uint32_t            get_pc(void);
    uint32_t            get_opcode(void)  { return get_opcode(get_pc()); }
    int                 get_num_reg(void) { return 16; }

    void                set_register(int r, uint32_t val);
    void                set_pc(uint32_t val);

    void                stats_reset(void) { }
    void                stats_dump(void)  { }

    // First register for args in ABI
    int                 get_abi_reg_arg0(void) { return 0; }

    // Number of registers (max) used for args
    int                 get_abi_reg_num(void) { return 4; }

    void                set_flag(uint32_t flag, bool val);
    bool                get_flag(uint32_t flag);

protected:
    uint16_t            armv6m_read_inst(uint32_t addr);
    void                armv6m_update_sp(uint32_t sp);
    void                armv6m_update_n_z_flags(uint32_t rd);
    uint32_t            armv6m_add_with_carry(uint32_t rn, uint32_t rm, uint32_t carry_in, uint32_t mask);
    uint32_t            armv6m_shift_left(uint32_t val, uint32_t shift, uint32_t mask);
    uint32_t            armv6m_shift_right(uint32_t val, uint32_t shift, uint32_t mask);
    uint32_t            armv6m_arith_shift_right(uint32_t val, uint32_t shift, uint32_t mask);
    uint32_t            armv6m_rotate_right(uint32_t val, uint32_t shift, uint32_t mask);
    uint32_t            armv6m_sign_extend(uint32_t val, int offset);
    void                armv6m_dump_inst(uint16_t inst);
    uint32_t            armv6m_exception(uint32_t pc, uint32_t exception);
    void                armv6m_exc_return(uint32_t pc);

public:
    int                 armv6m_decode(uint16_t inst);
    void                armv6m_execute(uint16_t inst, uint16_t inst2);

protected:  

    // Registers
    uint32_t            m_regfile[16];
    uint32_t            m_psp; // Process stack pointer
    uint32_t            m_msp; // Main stack pointer
    uint32_t            m_apsr;
    uint32_t            m_ipsr;
    uint32_t            m_epsr;

    uint32_t            m_primask;
    uint32_t            m_control;

    typedef enum { MODE_THREAD = 0, MODE_HANDLER } tMode;
    tMode               m_current_mode;

    uint32_t            m_entry_point;

    // Decode
    int                 m_inst_group;
    uint32_t            m_rd;
    uint32_t            m_rt;
    uint32_t            m_rm;
    uint32_t            m_rn;
    uint32_t            m_imm;
    uint32_t            m_cond;
    uint32_t            m_reglist;

    // Built in peripherals
    device_systick    * m_systick;
    bool                m_systick_irq;
};

#endif
