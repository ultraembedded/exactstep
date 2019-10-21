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
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>

extern "C"
{
    #include <libfdt.h>
}

#include "device_tree.h"

#include "device_uart_lite.h"
#include "device_spi_lite.h"
#include "device_timer_r5.h"
#include "device_timer_owl.h"
#include "device_irq_ctrl.h"
#include "device_dummy.h"

//-----------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------
device_tree::device_tree(const char *filename, console_io *con_io /*= NULL*/)
{
    m_fdt      = NULL;
    m_filename = std::string(filename);
    m_memory   = NULL;
    m_devices  = NULL;
    m_console  = con_io;
}
//-----------------------------------------------------------------
// create_memory: Create a memory
//-----------------------------------------------------------------
bool device_tree::create_memory(uint32_t base, uint32_t size)
{
    memory_base *mem = new memory("mem", base, size);

    mem->next = m_memory;
    m_memory = mem;

    return true;
}
//-----------------------------------------------------------------
// attach_device: Attach a memory device to a particular region
//-----------------------------------------------------------------
bool device_tree::attach_device(device *dev)
{
    assert(dev->device_next == NULL);

    dev->device_next = m_devices;
    m_devices = dev;

    return true;
}
//-----------------------------------------------------------------
// open_fdt: Open FDT file (binary)
//-----------------------------------------------------------------
bool device_tree::open_fdt(void)
{
    uint8_t *fdt = NULL;
    FILE *f = fopen(m_filename.c_str(), "rb");
    if (f)
    {
        int size;

        // Get size
        fseek(f, 0, SEEK_END);
        size = (int)ftell(f);
        rewind(f);

        // Read file into buffer
        fdt = new uint8_t[size];
        if (fdt)
        {
            int res = fread(fdt, 1, size, f);
            fclose(f);

            if (res != size)
            {
                delete fdt;
                return false;
            }
        }
    }
    else
        return false;

    if (fdt_check_header(fdt) < 0)
    {
        fprintf(stderr, "ERROR: Bad DTS format\n");
        delete fdt;
        return false;
    }

    m_fdt = fdt;

    return true;
}
//-----------------------------------------------------------------
// load: Process device tree
//-----------------------------------------------------------------
bool device_tree::load(void)
{
    device *irq_ctrl = NULL;

    printf("Parsing device tree:\n");
    if (open_fdt())
    {
        for (int offset = 0 ; offset >= 0; offset = fdt_next_node(m_fdt, offset, NULL))
        {
            //const char *name;
            //if ((name = fdt_get_name(m_fdt, offset, NULL)) != NULL)
            //    ;

            int size;

            // Find memory definitions
            const char *device_type;
            if (device_type = (const char *)fdt_getprop(m_fdt, offset, "device_type", &size))
            {
                if (!strcmp(device_type, "memory"))
                {
                    const uint32_t *reg = (const uint32_t*)fdt_getprop(m_fdt, offset, "reg", &size);
                    if (!reg)
                        continue;

                    assert(size == 8);

                    uint32_t reg_addr = ntohl(reg[0]);
                    uint32_t reg_size = ntohl(reg[1]);

                    printf("|- Attach memory: Addr %08x - %08x\n", reg_addr, reg_addr + reg_size - 1);
                    create_memory(reg_addr, reg_size);
                }
                else if (!strcmp(device_type, "cpu"))
                {
                    continue;
                }
            }

            // Match device
            const char * compat;
            if (compat = (const char *)fdt_getprop(m_fdt, offset, "compatible", &size))
            {
                const uint32_t *reg = (const uint32_t*)fdt_getprop(m_fdt, offset, "reg", &size);
                if (!reg)
                    continue;

                uint32_t reg_addr = ntohl(reg[0]);
                uint32_t reg_size = (size > 4) ? ntohl(reg[1]) : 0;

                int irq_num = -1;
                const uint32_t *irq = (const uint32_t*)fdt_getprop(m_fdt, offset, "interrupts", &size);
                if (irq)
                    irq_num = ntohl(*irq);

                if (!strcmp(compat, "xlnx,xps-intc-1.00.a"))
                {
                    // Create IRQ controller
                    // TODO: Support multiple controllers
                    irq_ctrl = new device_irq_ctrl(reg_addr, 0);
                    printf("|- Create interrupt controller: Addr %08x\n", reg_addr);
                    attach_device(irq_ctrl);
                }
                else if (!strcmp(compat, "xlnx,xps-uartlite-1.00.a"))
                {
                    printf("|- Create UART: Addr %08x IRQ %d\n", reg_addr, irq_num);
                    attach_device(new device_uart_lite(reg_addr, irq_ctrl, irq_num, m_console));
                }
                else if (!strcmp(compat, "actions,s500-timer"))
                {
                    printf("|- Create Timer: Addr %08x IRQ %d\n", reg_addr, irq_num);
                    attach_device(new device_timer_owl(reg_addr, irq_ctrl, irq_num));
                }
                else if (!strcmp(compat, "riscv,openr5-timer"))
                {
                    printf("|- Create Timer: Addr %08x IRQ %d\n", reg_addr, irq_num);
                    attach_device(new device_timer_r5(reg_addr, irq_ctrl, irq_num));
                }
                else if (!strcmp(compat, "xlnx,xps-spi-2.00.b"))
                {
                    printf("|- Create SPI: Addr %08x IRQ %d\n", reg_addr, irq_num);
                    attach_device(new device_spi_lite(reg_addr, irq_ctrl, irq_num));
                }
                else
                {
                    printf("|- Create dummy device (%s): Addr %08x - %08x\n", compat, reg_addr, reg_addr + reg_size-1);
                    attach_device(new device_dummy(reg_addr, reg_size));
                }
            }
        }

        return true;
    }

    return false;
}