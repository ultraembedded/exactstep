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

// Forward decl
class cpu;

//--------------------------------------------------------------------
// ELF loader
//--------------------------------------------------------------------
class device_tree
{
public:
    device_tree(const char *filename, console_io *con_io = NULL);

    bool         load(cpu *cpu);

protected:
    bool         open_fdt(void);
    int          process_node(int offset);

protected:
    std::string  m_filename;
    uint8_t *    m_fdt;
    console_io  *m_console;
};

#endif