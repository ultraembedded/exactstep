//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdint.h>
#include <stdlib.h>

//-----------------------------------------------------------------
// SDL based display widget
//-----------------------------------------------------------------
class display
{
public:
    display()
    {
        m_screen = NULL;
        m_width  = 0;
        m_height = 0;
    }
    bool init(int width, int height);
    bool update(uint8_t *memory);

private:
    int   m_width;
    int   m_height;
    void *m_screen;
};

#endif
