//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __CONSOLE_IO_H__
#define __CONSOLE_IO_H__

//--------------------------------------------------------------------
// Abstract interface for conio
//--------------------------------------------------------------------
class console_io
{
public:  
    virtual int putchar(int ch) = 0;
    virtual int getchar(void) = 0;
};

#endif