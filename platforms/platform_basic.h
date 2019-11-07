//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __PLATFORM_BASIC_H__
#define __PLATFORM_BASIC_H__

#include "platform.h"
#include "platform_cpu.h"

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

class platform_basic: public platform_cpu
{
public:
    platform_basic(const char *misa, uint32_t membase, uint32_t memsize, console_io *con_io = NULL): platform_cpu(misa, false, membase, memsize)
    {
        cpu * pcpu = get_cpu();
        if (!pcpu)
            return ;

        // Create IRQ controller
        device *irq_ctrl = new device_irq_ctrl(CONFIG_IRQCTRL_BASE, 11);
        pcpu->attach_device(irq_ctrl);

        // Timer
        pcpu->attach_device(new device_timer_owl(CONFIG_TIMER_BASE, irq_ctrl, CONFIG_TIMER_IRQ));

        // Uart
        pcpu->attach_device(new device_uart_lite(CONFIG_UARTLITE_BASE, irq_ctrl, CONFIG_UARTLITE_IRQ, con_io));

        // SPI
        pcpu->attach_device(new device_spi_lite(CONFIG_SPILITE_BASE, irq_ctrl, CONFIG_SPILITE_IRQ));
    }
};

#endif