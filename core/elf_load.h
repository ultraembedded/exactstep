//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __ELF_LOAD_H__
#define __ELF_LOAD_H__

#include "mem_api.h"
#include <string>

//--------------------------------------------------------------------
// ELF loader
//--------------------------------------------------------------------
class elf_load
{
public:
    elf_load(const char *filename, mem_api *target, bool load_to_paddr = false, int64_t load_offset = 0);

    bool     load(void);
    uint32_t get_entry_point(void) { return m_entry_point; }
    bool     get_symbol(const char *symname, uint32_t &value);

protected:
    std::string m_filename;
    mem_api *   m_target;
    uint32_t    m_entry_point;
    bool        m_load_to_paddr;
    int64_t     m_load_offset;
};

#endif
