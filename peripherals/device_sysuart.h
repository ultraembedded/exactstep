//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_SYSUART_H__
#define __DEVICE_SYSUART_H__

#include "device.h"

//-----------------------------------------------------------------
// Model of ARM System UART
//-----------------------------------------------------------------
class device_sysuart: public device
{
public:
    device_sysuart(uint32_t base_addr, uint32_t size, device *irq_ctrl, int irq_num): device("sysuart", base_addr, size, irq_ctrl, irq_num)
    {
        reset();
    }
    
    void reset(void)
    {

    }

    bool write32(uint32_t address, uint32_t data)
    {
        printf("%c",(data >> 0) & 0xFF);
        fflush(stdout);
        return true;
    }
    bool read32(uint32_t address, uint32_t &data)
    {
        data = 0;
        return true;
    }

    int clock(void)
    {
        return 0;
    }
};

#endif
