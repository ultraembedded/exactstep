//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __PLATFORM_RV64_BASIC_H__
#define __PLATFORM_RV64_BASIC_H__

#include "platform.h"
#include "rv64.h"

#include "device_uart_lite.h"
#include "device_spi_lite.h"
#include "device_timer_owl.h"
#include "device_irq_ctrl.h"
#include "device_dummy.h"

#define CONFIG_IRQCTRL_BASE           0x90000000
#define CONFIG_TIMER_BASE             0x91000000
#define CONFIG_TIMER_IRQ              0
#define CONFIG_UARTLITE_BASE          0x92000000
#define CONFIG_UARTLITE_IRQ           1
#define CONFIG_SPILITE_BASE           0x93000000
#define CONFIG_SPILITE_IRQ            2

class platform_rv64_basic: public platform
{
public:
    platform_rv64_basic(uint32_t membase, uint32_t memsize, console_io *con_io = NULL)
    {
        printf("Platform: Select RV64\n");
        m_cpu = new rv64(membase, memsize);

        // Create IRQ controller
        device *irq_ctrl = new device_irq_ctrl(CONFIG_IRQCTRL_BASE, 11);
        m_cpu->attach_device(irq_ctrl);

        // Timer
        m_cpu->attach_device(new device_timer_owl(CONFIG_TIMER_BASE, irq_ctrl, CONFIG_TIMER_IRQ));

        // Uart
        m_cpu->attach_device(new device_uart_lite(CONFIG_UARTLITE_BASE, irq_ctrl, CONFIG_UARTLITE_IRQ, con_io));

        // SPI
        m_cpu->attach_device(new device_spi_lite(CONFIG_SPILITE_BASE, irq_ctrl, CONFIG_SPILITE_IRQ));

        // Simplified CSR handling for now...
        m_cpu->enable_compliant_csr(false);
    }

    cpu* get_cpu(void) { return m_cpu; }

    rv64 * m_cpu;
};

#endif