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
#define TIMER_CTRL        0x0
    #define TIMER_CTRL_INTERRUPT                 0
    #define TIMER_CTRL_INTERRUPT_SHIFT           0
    #define TIMER_CTRL_INTERRUPT_MASK            0x1

#define TIMER_CMP         0x4
    #define TIMER_CMP_VALUE_SHIFT                0
    #define TIMER_CMP_VALUE_MASK                 0xffffffff

#define TIMER_VAL         0x8
    #define TIMER_VAL_CURRENT_SHIFT              0
    #define TIMER_VAL_CURRENT_MASK               0xffffffff

//-----------------------------------------------------------------
// device_timer_r5: Model of OpenR5 Timer IP
//-----------------------------------------------------------------
class device_timer_r5: public device
{
public:
    device_timer_r5(uint32_t base_addr, device *irq_ctrl, int irq_num): device("r5_timer", base_addr, 256, irq_ctrl, irq_num)
    {
        reset();
    }
    
    void reset(void)
    {
        m_reg_ctrl = 0;
        m_reg_cmp  = 0;
        m_reg_val  = 0;
    }

    bool write32(uint32_t address, uint32_t data)
    {
        address -= m_base;
        switch (address)
        {
            case TIMER_CTRL:
                m_reg_ctrl = data;                
            break;
            case TIMER_CMP:
                m_reg_cmp = data;
            break;
            default:
                fprintf(stderr, "TimerOpenR5: Bad write @ %08x\n", address);
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
            case TIMER_CTRL:
                data = m_reg_ctrl;
            break;
            case TIMER_CMP:
                data = m_reg_cmp;
            break;
            case TIMER_VAL:
                data = m_reg_val;
            break;
            default:
                fprintf(stderr, "TimerOpenR5: Bad read @ %08x\n", address);
                return false;
            break;
        }
        return true;
    }

    int clock(void)
    {
        m_reg_val += 1;

        // Interrupts enabled and timer match
        if (m_reg_val == m_reg_cmp && (m_reg_ctrl & (1 << TIMER_CTRL_INTERRUPT_SHIFT)))
            raise_interrupt();
        
        return 0;
    }

private:
    uint32_t m_reg_ctrl;
    uint32_t m_reg_cmp;
    uint32_t m_reg_val;
};

#endif
