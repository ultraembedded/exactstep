//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "cpu.h"

class platform
{
public:
    virtual cpu* get_cpu(void) = 0;
};

#endif