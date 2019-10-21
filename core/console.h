//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "console_io.h"

//-----------------------------------------------------------------
// console: Simple polled console
//-----------------------------------------------------------------
class console: public console_io
{
public:
    console();

    int putchar(int ch);
    int getchar(void);
};

#endif