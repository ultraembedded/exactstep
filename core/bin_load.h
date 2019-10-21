//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __BIN_LOAD_H__
#define __BIN_LOAD_H__

#include "mem_api.h"
#include <string>

//--------------------------------------------------------------------
// Binary loader
//--------------------------------------------------------------------
class bin_load
{
public:
    bin_load(const char *filename, mem_api *target);

    bool     load(uint32_t mem_base, uint32_t mem_size);

protected:
    std::string m_filename;
    mem_api *   m_target;
};

#endif