//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>
#include <vector>
#include "memory.h"
#include "device.h"
#include "mem_api.h"
#include "console_io.h"

//--------------------------------------------------------------------
// CPU model base class
//--------------------------------------------------------------------
class cpu: public mem_api
{
public:
    cpu();

    // mem_api
    virtual bool      create_memory(uint32_t addr, uint32_t size, uint8_t *mem = NULL);
    virtual bool      valid_addr(uint32_t addr);
    virtual void      write(uint32_t addr, uint8_t data);
    virtual uint8_t   read(uint32_t addr);

    // Memory access helpers
    virtual bool      attach_memory(memory_base *memory);
    virtual void      write16(uint32_t address, uint16_t data);
    virtual uint16_t  read16(uint32_t address);    
    virtual void      write32(uint32_t address, uint32_t data);
    virtual uint32_t  read32(uint32_t address);

    // Attach peripherals
    virtual bool      attach_device(device * device);

    // Reset core to execute from specified PC
    virtual void      reset(uint32_t pc) = 0;

    // Status    
    virtual bool      get_fault(void)   { return m_fault; }
    virtual bool      get_stopped(void) { return m_stopped; }

    // Execute one instruction
    virtual void      step(void);

    // Breakpoints
    virtual bool      get_break(void);
    virtual bool      set_breakpoint(uint32_t pc);
    virtual bool      clr_breakpoint(uint32_t pc);
    virtual bool      check_breakpoint(uint32_t pc);

    // State after execution
    virtual uint32_t  get_opcode(void) = 0;
    virtual uint32_t  get_pc(void) = 0;
    virtual uint64_t  get_pc64(void) { return get_pc(); }
    virtual uint32_t  get_register(int r) = 0;
    virtual uint64_t  get_register64(int r) { return 0; }
    virtual int       get_reg_width(void) { return 32; } // Default
    virtual int       get_num_reg(void) = 0;

    virtual void      set_register(int r, uint32_t val) = 0;
    virtual void      set_pc(uint32_t val) = 0;

    virtual void      set_register(int r, uint64_t val) { }
    virtual void      set_pc(uint64_t val) { }

    // First register for args in ABI
    virtual int       get_abi_reg_arg0(void) = 0;

    // Number of registers (max) used for args
    virtual int       get_abi_reg_num(void) = 0;

    // Get return register
    virtual int       get_abi_reg_ret(void) { return get_abi_reg_arg0(); }

    // Trigger interrupt
    virtual void      set_interrupt(int irq) = 0;
    virtual void      clr_interrupt(int irq) = 0;

    // Instruction trace
    virtual void      enable_trace(uint32_t mask) { m_trace = mask; }

    // Monitor executed instructions
    virtual void      log_exception(uint64_t src, uint64_t dst, uint64_t cause) { }
    virtual void      log_branch(uint64_t src, uint64_t dst, bool taken) { }
    virtual void      log_branch_jump(uint64_t src, uint64_t dst) { }
    virtual void      log_branch_call(uint64_t src, uint64_t dst) { }
    virtual void      log_branch_ret(uint64_t src, uint64_t dst) { }
    virtual void      log_commit_pc(uint64_t pc) { }

    // Stats
    virtual void      stats_reset(void) = 0;
    virtual void      stats_dump(void) = 0;

    // Console
    void              set_console(console_io *cio)  { m_console = cio; }

    // Error message
    bool              error(bool is_fatal, const char *fmt, ...);

protected:
    // Memory
    memory_base        *m_memories;
    device             *m_devices;

    // Status
    bool                m_stopped;
    bool                m_fault;
    bool                m_break;
    int                 m_trace;

    // Breakpoints
    bool                m_has_breakpoints;
    std::vector <uint32_t > m_breakpoints;

    // Console
    console_io         *m_console;
};

#endif
