//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_TIMER_CLINT_H__
#define __DEVICE_TIMER_CLINT_H__

#include "device.h"
#include "cpu.h"

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define CLINT_REG_MSIP          0x0000
#define CLINT_REG_TIMER_CMP_LO  0x4000
#define CLINT_REG_TIMER_CMP_HI  0x4004
#define CLINT_REG_TIMER_VAL_LO  0xbff8
#define CLINT_REG_TIMER_VAL_HI  0xbffc
#define CLINT_REG_SIZE          0xc000

#define IRQ_M_TIMER             7

//-----------------------------------------------------------------
// device_timer_clint: Model of 'Core-Local Interruptor'
//-----------------------------------------------------------------
class device_timer_clint: public device
{
public:
    device_timer_clint(uint32_t base_addr, cpu *cpu): device("clint", base_addr, CLINT_REG_SIZE, NULL, 0)
    {
        m_cpu = cpu;
        reset();
    }
    
    void reset(void)
    {
        m_reg_cmp  = (uint64_t)-1;
        m_reg_val  = 0;
    }

    bool write32(uint32_t address, uint32_t data)
    {
        address -= m_base;
        switch (address)
        {
            case CLINT_REG_MSIP:
                if (data != 0)
                {
                    fprintf(stderr, "CLINT: Error - trying to raise MSIP - not supported yet\n");
                    return false;
                }
                break;
            case CLINT_REG_TIMER_CMP_LO:
                m_reg_cmp &= ~0xffffffffull;
                m_reg_cmp |= data;
            break;
            case CLINT_REG_TIMER_CMP_HI:
                m_reg_cmp &= ~0xffffffff00000000ull;
                m_reg_cmp |= ((uint64_t)data) << 32;
            break;
            default:
                fprintf(stderr, "CLINT: Bad write @ %08x\n", address);
                return false;
            break;
        }
        return true;
    }
    bool read32(uint32_t address, uint32_t &data)
    {
        data = 0;
        address -= m_base;

        switch (address)
        {
            case CLINT_REG_TIMER_CMP_LO:
                data = m_reg_cmp >> 0;
            break;
            case CLINT_REG_TIMER_CMP_HI:
                data = m_reg_cmp >> 32;
            break;
            case CLINT_REG_TIMER_VAL_LO:
                data = m_reg_val >> 0;
            break;
            case CLINT_REG_TIMER_VAL_HI:
                data = m_reg_val >> 32;
            break;
            default:
                fprintf(stderr, "CLINT: Bad read @ %08x\n", address);
                return false;
            break;
        }
        return true;
    }

    int clock(void)
    {
        m_reg_val += 1;

        // Timer match - should set MIP_MTIP
        if (m_reg_val >= m_reg_cmp)
            m_cpu->set_interrupt(IRQ_M_TIMER);
        else
            m_cpu->clr_interrupt(IRQ_M_TIMER);
        
        return 0;
    }

private:
    cpu     *m_cpu;
    uint64_t m_reg_cmp;
    uint64_t m_reg_val;
};

#endif
