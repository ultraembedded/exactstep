//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_TIMER_R5_H__
#define __DEVICE_TIMER_R5_H__

#include "device.h"

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define MTIMER_BASE         0x02000000
#define MTIMER_MSIP         0x0000
#define MTIMER_MTIMECMP_LO  0x4000
#define MTIMER_MTIMECMP_HI  0x4004
#define MTIMER_MTIME_LO     0xbff8
#define MTIMER_MTIME_HI     0xbffc
#define MTIMER_SIZE         0x000c0000

//-----------------------------------------------------------------
// device_mtimer: Model of standard RISC-V Timer IP
//-----------------------------------------------------------------
class device_mtimer: public device
{
public:
    device_mtimer(uint32_t base_addr, device *irq_ctrl, int irq_num): device("r5_timer", base_addr, 256, irq_ctrl, irq_num)
    {
        reset();
    }
    
    void reset(void)
    {
        m_time     = 0;
        m_timecmp  = 0;
    }

    bool write32(uint32_t address, uint32_t data)
    {
        address -= m_base;

        switch (address)
        {
            case MTIMER_MSIP:
            break;
            case MTIMER_MTIMECMP_LO:
                m_timecmp &= 0xFFFFFFFF00000000ULL;
                m_timecmp |= data;
            break;
            case MTIMER_MTIMECMP_HI:
                m_timecmp &= 0x00000000FFFFFFFFULL;
                m_timecmp |= ((uint64_t)data)<<32;
            break;
            case MTIMER_MTIME_LO:
                m_time &= 0xFFFFFFFF00000000ULL;
                m_time |= data;
            break;
            case MTIMER_MTIME_HI:
                m_time &= 0x00000000FFFFFFFFULL;
                m_time |= ((uint64_t)data)<<32;
            break;
            default:
                fprintf(stderr, "MTIMER: Bad write @ %08x\n", address);
                return false;
        }

        return true;
    }
    bool read32(uint32_t address, uint32_t &data)
    {
        data = 0;
        address -= m_base;

        switch (address)
        {
            case MTIMER_MSIP:
            break;
            case MTIMER_MTIMECMP_LO:
                data = (uint32_t)(m_timecmp >> 0);
            break;
            case MTIMER_MTIMECMP_HI:
                data = (uint32_t)(m_timecmp >> 32);
            break;
            case MTIMER_MTIME_LO:
                data = (uint32_t)(m_time >> 0);
            break;
            case MTIMER_MTIME_HI:
                data = (uint32_t)(m_time >> 32);
            break;
            default:
                fprintf(stderr, "MTIMER: Bad read @ %08x\n", address);
                return false;
        }        

        return true;
    }

    int clock(void)
    {
        m_time += 1;

        // Interrupts enabled and timer match
        if (m_time >= m_timecmp)
            raise_interrupt();
        
        return 0;
    }

private:
    uint64_t m_time;
    uint64_t m_timecmp;
};

#endif
