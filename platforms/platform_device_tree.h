//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __PLATFORM_DEVICE_TREE_H__
#define __PLATFORM_DEVICE_TREE_H__

#include "platform.h"
#include "platform_cpu.h"

#include "device_tree.h"

class platform_device_tree: public platform_cpu
{
public:
    platform_device_tree(const char *misa, const char *filename, console_io *con_io = NULL): platform_cpu(misa, true, 0, 0)
    {
        m_filename    = std::string(filename);
        m_console     = con_io;
        m_this_cpu    = NULL;
        m_mem_base    = 0;
        m_mem_size    = 0;
        m_initrd_base = 0;
        m_initrd_size = 0;
    }

    virtual cpu* get_cpu(void)
    {
        if (!m_this_cpu)
        {
            m_this_cpu = platform_cpu::get_cpu();
            if (!m_this_cpu)
                return NULL;

            device_tree fdt(m_filename.c_str(), m_console);
            if (!fdt.load(m_this_cpu))
                fprintf(stderr, "ERROR: Failed to open device tree\n");

            m_mem_base = fdt.get_mem_base();
            m_mem_size = fdt.get_mem_size();

            m_initrd_base = fdt.get_initrd_base();
            m_initrd_size = fdt.get_initrd_size();
        }

        return m_this_cpu;
    }

    // This API only makes sense for systems with a single linear memory
    uint32_t     get_mem_base(void) { return m_mem_base; }
    uint32_t     get_mem_size(void) { return m_mem_size; }

    uint32_t     get_initrd_base(void) { return m_initrd_base; }
    uint32_t     get_initrd_size(void) { return m_initrd_size; }

    cpu *        m_this_cpu;
    std::string  m_filename;
    console_io * m_console;
    uint32_t     m_mem_base;
    uint32_t     m_mem_size;
    uint32_t     m_initrd_base;
    uint32_t     m_initrd_size;
};

#endif