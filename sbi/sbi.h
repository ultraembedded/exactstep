//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __SBI_H__
#define __SBI_H__

#include <string>
#include <stdint.h>
#include <vector>
#include "cpu.h"
#include "syscall_if.h"

//-----------------------------------------------------------------
// sbi: SBI hosting
//--------------------------------------------------------------------
class sbi: public syscall_if
{
public:
    sbi(console_io *conio);
    bool syscall_handler(cpu *instance);

    static bool setup(cpu *cpu, console_io *conio, uint32_t kernel_addr, uint32_t dtb_addr);

protected:
    console_io *m_conio;
};

#endif