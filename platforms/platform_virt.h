//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __PLATFORM_VIRT_H__
#define __PLATFORM_VIRT_H__

#include "platform.h"
#include "platform_cpu.h"

#include "device_uart_8250.h"
#include "device_timer_clint.h"
#include "device_irq_plic.h"

// OpenSBI peripherals
#define CONFIG_PLIC_BASE              0x0c000000
#define CONFIG_CLINT_BASE             0x02000000
#define CONFIG_UART8250_BASE          0x10000000

class platform_virt: public platform_cpu
{
public:
    platform_virt(const char *misa, uint32_t membase, uint32_t memsize, console_io *con_io = NULL): platform_cpu(misa, true, membase, memsize)
    {
        if (!m_cpu)
            return;

        // OpenSBI peripherals
        m_cpu->attach_device(new device_irq_plic(CONFIG_PLIC_BASE, 11));
        m_cpu->attach_device(new device_timer_clint(CONFIG_CLINT_BASE, m_cpu));
        m_cpu->attach_device(new device_uart_8250(CONFIG_UART8250_BASE, NULL, -1, con_io));
    }
};

#endif