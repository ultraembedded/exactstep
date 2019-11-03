//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __PLATFORM_RV32_VIRT_H__
#define __PLATFORM_RV32_VIRT_H__

#include "platform.h"
#include "rv32.h"

#include "device_uart_8250.h"
#include "device_timer_clint.h"
#include "device_irq_plic.h"
#include "device_tree.h"

// OpenSBI peripherals
#define CONFIG_PLIC_BASE              0x0c000000
#define CONFIG_CLINT_BASE             0x02000000
#define CONFIG_UART8250_BASE          0x10000000

class platform_rv32_virt: public platform
{
public:
    platform_rv32_virt(const char *filename, console_io *con_io = NULL)
    {
        m_cpu      = NULL;
        m_filename = std::string(filename);
        m_console  = con_io;
    }

    cpu* get_cpu(void)
    {
        if (!m_cpu)
        {
            device_tree fdt(m_filename.c_str(), m_console);
            if (fdt.load())
            {
                printf("Platform: Select RV32\n");
                m_cpu = new rv32();

                // Attach memories to CPU
                m_cpu->create_memory(0x80000000, (64 << 20));
                //memory_base *mem_next = NULL;
                //for (memory_base *mem = fdt.get_memory(); mem != NULL; mem = mem_next)
                //{
                //    mem_next  = mem->next;
                //    mem->next = NULL;
                //    m_cpu->attach_memory(mem);
                //}

                // OpenSBI peripherals
                m_cpu->attach_device(new device_irq_plic(CONFIG_PLIC_BASE, 11));
                m_cpu->attach_device(new device_timer_clint(CONFIG_CLINT_BASE, m_cpu));
                m_cpu->attach_device(new device_uart_8250(CONFIG_UART8250_BASE, NULL, -1, m_console));

                // Attach devices to CPU
                device *dev_next = NULL;
                for (device *dev = fdt.get_device(); dev != NULL; dev = dev_next)
                {
                    dev_next  = dev->device_next;
                    dev->device_next = NULL;

                    m_cpu->attach_device(dev);
                }

                // Standard CSR behaviour
                m_cpu->enable_compliant_csr(true);
                m_cpu->enable_rvc(true);
                m_cpu->enable_rva(true);
                //m_cpu->enable_mem_errors(true);
            }
            else
                fprintf(stderr, "ERROR: Failed to open device tree\n");
        }

        return m_cpu;
    }

    rv32 *      m_cpu;
    std::string m_filename;
    console_io *m_console;
};

#endif