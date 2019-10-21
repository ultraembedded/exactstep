//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_DUMMY_H__
#define __DEVICE_DUMMY_H__

#include "device.h"

//-----------------------------------------------------------------
// device_dummy:
//-----------------------------------------------------------------
class device_dummy: public device
{
public:
    device_dummy(uint32_t base_addr, uint32_t size): device("dummy", base_addr, size, NULL, -1)
    {
        reset();
    }
    
    bool write32(uint32_t address, uint32_t data)
    {
        printf("DUMMY (WR): %08x=%08x\n", address, data);
        return true;
    }
    bool read32(uint32_t address, uint32_t &data)
    {
        data = 0;
        printf("DUMMY (RD): %08x=%08x\n", address, data);
        return true;
    }

    int clock(void)
    {
        return 0;
    }

private:
};

#endif
