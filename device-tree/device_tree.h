//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_TREE_H__
#define __DEVICE_TREE_H__

#include <stdint.h>
#include <string>

#include "memory.h"
#include "device.h"
#include "console_io.h"

//--------------------------------------------------------------------
// ELF loader
//--------------------------------------------------------------------
class device_tree
{
public:
    device_tree(const char *filename, console_io *con_io = NULL);

    bool         load(void);

    bool         create_memory(uint32_t base, uint32_t size);
    memory_base* get_memory(void) { return m_memory; }

    bool         attach_device(device *dev);
    device     * get_device(void) { return m_devices; }

protected:
    bool         open_fdt(void);
    int          process_node(int offset);

protected:
    std::string  m_filename;
    uint8_t *    m_fdt;
    memory_base *m_memory;
    device      *m_devices;
    console_io  *m_console;
};

#endif