//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __PLATFORM_CPU_H__
#define __PLATFORM_CPU_H__

#include "platform.h"
#include "rv32.h"
#include "rv64.h"
#include "armv6m.h"

#include "device_systick.h"
#include "device_sysuart.h"
#include "device_dummy.h"

class platform_cpu: public platform
{
public:
    platform_cpu(const char *misa, bool support_s = false, uint32_t membase = 0, uint32_t memsize = 0)
    {
        m_cpu = NULL;

        if (!strncmp(misa, "RV32", 4) || !strncmp(misa, "rv32", 4))
        {
            printf("Platform: Select %s\n", misa);
            rv32 * cpu = new rv32(membase, memsize);

            // Simplified CSR handling for now...
            cpu->enable_compliant_csr(support_s);

            // Optional instruction sets
            cpu->enable_rvm((strchr(misa, 'M') || strchr(misa, 'm')));
            cpu->enable_rvc((strchr(misa, 'C') || strchr(misa, 'c')));
            cpu->enable_rva((strchr(misa, 'A') || strchr(misa, 'a')));

            m_cpu = cpu;
        }
        else if (!strncmp(misa, "RV64", 4) || !strncmp(misa, "rv64", 4))
        {
            printf("Platform: Select %s\n", misa);
            rv64 *cpu = new rv64(membase, memsize);

            // Simplified CSR handling for now...
            cpu->enable_compliant_csr(support_s);

            // Optional instruction sets
            cpu->enable_rvm((strchr(misa, 'M') || strchr(misa, 'm')));
            cpu->enable_rvc((strchr(misa, 'C') || strchr(misa, 'c')));
            cpu->enable_rva((strchr(misa, 'A') || strchr(misa, 'a')));

            m_cpu = cpu;
        }
        else if (!strncmp(misa, "armv6", 5))
        {
            if (membase == 0)
                membase = 0x20000000;

            printf("Platform: Select ARMv6m\n");
            armv6m *cpu = new armv6m(membase, memsize);

            // Simple UART - writes to 0xE0000000 are output on the console
            cpu->attach_device(new device_sysuart(0xE0000000, 0x1000, NULL, -1));

            // Dummy System Control Block - writes have no effect, reads return 0
            cpu->attach_device(new device_dummy(0xE000ED00, 36));

            m_cpu = cpu;
        }
    }

    virtual cpu* get_cpu(void) { return m_cpu; }
    cpu * m_cpu;
};

#endif