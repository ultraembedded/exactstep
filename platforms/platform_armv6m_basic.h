//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __PLATFORM_ARMV6M_BASIC_H__
#define __PLATFORM_ARMV6M_BASIC_H__

#include "platform.h"
#include "armv6m.h"

#include "device_systick.h"
#include "device_sysuart.h"
#include "device_dummy.h"
#include "device_uart_lite.h"

class platform_armv6m_basic: public platform
{
public:
    platform_armv6m_basic(uint32_t membase, uint32_t memsize, console_io *con_io = NULL)
    {
        printf("Platform: Select ARMv6m\n");
        m_cpu = new armv6m(membase, memsize);

        // Simple UART - writes to 0xE0000000 are output on the console
        m_cpu->attach_device(new device_sysuart(0xE0000000, 0x1000, NULL, -1));

        // Another instance of the UART @ 0x42000000
        m_cpu->attach_device(new device_uart_lite(0x42000000, NULL, -1, con_io));

        // Dummy System Control Block - writes have no effect, reads return 0
        m_cpu->attach_device(new device_dummy(0xE000ED00, 36));

    }

    cpu* get_cpu(void) { return m_cpu; }

    armv6m * m_cpu;
};

#endif