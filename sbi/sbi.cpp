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
#include <assert.h>

#include "sbi.h"
#include "rv32.h"
#include "rv64.h"
#include "bin_load.h"

//-----------------------------------------------------------------
// SYSCALLs
//-----------------------------------------------------------------
#define SBI_SET_TIMER 0
#define SBI_CONSOLE_PUTCHAR 1
#define SBI_CONSOLE_GETCHAR 2
#define SBI_CLEAR_IPI 3
#define SBI_SEND_IPI 4
#define SBI_REMOTE_FENCE_I 5
#define SBI_REMOTE_SFENCE_VMA 6
#define SBI_REMOTE_SFENCE_VMA_ASID 7
#define SBI_SHUTDOWN 8

//-----------------------------------------------------------------
// setup: Setup SBI handler (and load some binaries)
//-----------------------------------------------------------------
bool sbi::setup(cpu *cpu, console_io *conio, uint32_t kernel_addr, uint32_t dtb_addr)
{
    if (cpu->get_reg_width() == 64)
        ((rv64*)cpu)->sbi_boot(kernel_addr, dtb_addr);
    else
        ((rv32*)cpu)->sbi_boot(kernel_addr, dtb_addr);

    // Register SBI syscall handler
    cpu->set_syscall_handler(new sbi(conio));

    return true;
}
//-----------------------------------------------------------------
// Construction
//-----------------------------------------------------------------
sbi::sbi(console_io *conio)
{
    m_conio = conio;
}   
//-----------------------------------------------------------------
// syscall_handler: Try and execute a hosted system call
//-----------------------------------------------------------------
bool sbi::syscall_handler(cpu *cpu)
{
    int reg_id  = 10 + 7;
    int reg_a0  = 10;
    int reg_ret = 10;
    uint64_t a0    = (cpu->get_reg_width() == 64) ? cpu->get_register64(reg_a0 + 0) : cpu->get_register(reg_a0 + 0);
    uint64_t a1    = (cpu->get_reg_width() == 64) ? cpu->get_register64(reg_a0 + 1) : cpu->get_register(reg_a0 + 1);
    uint64_t a2    = (cpu->get_reg_width() == 64) ? cpu->get_register64(reg_a0 + 2) : cpu->get_register(reg_a0 + 2);
    uint64_t which = (cpu->get_reg_width() == 64) ? cpu->get_register64(reg_a0 + 7) : cpu->get_register(reg_a0 + 7);

    #define SET_RET(x) cpu->set_register(reg_ret, (uint32_t)(x))

    // Only take over syscalls in SUPER mode
    if (cpu->get_reg_width() == 64)
    {
        if (!((rv64*)cpu)->in_super_mode())
            return false;
    }
    else
    {
        if (!((rv32*)cpu)->in_super_mode())
            return false;
    }

    switch (which)
    {
        case SBI_SHUTDOWN:
            printf("Shutdown...\n");
            exit(0);
            return true;
        case SBI_CONSOLE_PUTCHAR:
            if (m_conio)
                m_conio->putchar(a0);
            return true;
        case SBI_CONSOLE_GETCHAR:
            if (m_conio)
            {
                if (cpu->get_reg_width() == 64)
                    ((rv64*)cpu)->set_register(reg_ret, (uint64_t)m_conio->getchar());
                else
                    ((rv32*)cpu)->set_register(reg_ret, (uint32_t)m_conio->getchar());
            }
            return true;
        case SBI_SET_TIMER:
            if (cpu->get_reg_width() == 64)
                ((rv64*)cpu)->set_timer(a0);
            else
                ((rv32*)cpu)->set_timer(a0);
            return true;
        case SBI_REMOTE_FENCE_I:
        case SBI_REMOTE_SFENCE_VMA:
            return true;
        default:
            printf("SBI: Unhandled SYSCALL, stopping... (id=%d)\n", which);
            exit(-1);
            break;
    }

    // Not handled
    printf("ECALL: Not handled %d\n", which);
    return false;
}
