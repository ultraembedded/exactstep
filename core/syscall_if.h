//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __SYSCALL_IF_H__
#define __SYSCALL_IF_H__

//--------------------------------------------------------------------
// Abstract interface for syscall_if
//--------------------------------------------------------------------
class cpu;
class syscall_if
{
public:  
    virtual bool syscall_handler(cpu *instance) = 0;
};

#endif
