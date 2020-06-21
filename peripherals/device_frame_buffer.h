//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_FRAME_BUFFER_H__
#define __DEVICE_FRAME_BUFFER_H__

#include "device.h"
#include "display.h"

//-----------------------------------------------------------------
// device_frame_buffer: Simplified frame buffer device
//-----------------------------------------------------------------
class device_frame_buffer: public device
{
public:
    device_frame_buffer(uint32_t base_addr, int width, int height): device("fb", base_addr, height * width * 2, NULL, -1)
    {
        m_fb      = new uint8_t[height * width * 2];
        m_ticks   = 0;

        m_display.init(width, height);
    }

    void reset(void)
    {

    }

    int  min_access_size(void) { return 1; }

    bool write8(uint32_t address, uint8_t data)
    {
        address -= m_base;
        m_fb[address] = data;
        return true;
    }

    bool write_block(uint32_t address, uint8_t *data, int length)
    {
        address -= m_base;
        for (int i=0;i<length;i++)
            m_fb[address+i] = data[i];
        return true;
    }

    bool write32(uint32_t address, uint32_t data)
    {
        address -= m_base;
        m_fb[address+0] = data >> 0;
        m_fb[address+1] = data >> 8;
        m_fb[address+2] = data >> 16;
        m_fb[address+3] = data >> 24;
        return true;
    }

    bool read32(uint32_t address, uint32_t &data)
    {
        return true;
    }

    int clock(void)
    {
        if (++m_ticks == 100000)
        {
            m_display.update(m_fb);
            m_ticks   = 0;
        }
        return 0;
    }

private:
    uint8_t *m_fb;
    int      m_ticks;
    display  m_display;
};

#endif
