//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_TIMER_OWL_H__
#define __DEVICE_TIMER_OWL_H__

#include "device.h"

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define TIMER_CTRL        0x0
    #define TIMER_CTRL_PD_SHIFT           0
    #define TIMER_CTRL_PD_MASK            0x1
    #define TIMER_CTRL_INTEN_SHIFT        1
    #define TIMER_CTRL_INTEN_MASK         0x1
    #define TIMER_CTRL_EN_SHIFT           2
    #define TIMER_CTRL_EN_MASK            0x1

#define TIMER_CMP         0x4
    #define TIMER_CMP_VALUE_SHIFT                0
    #define TIMER_CMP_VALUE_MASK                 0xffffffff

#define TIMER_VAL         0x8
    #define TIMER_VAL_CURRENT_SHIFT              0
    #define TIMER_VAL_CURRENT_MASK               0xffffffff

#define NUM_TIMERS            2

//-----------------------------------------------------------------
// device_timer_owl: Model of Owl Timer IP
//-----------------------------------------------------------------
class device_timer_owl: public device
{
public:
    device_timer_owl(uint32_t base_addr, device *irq_ctrl, int irq_num): device("owl_timer", base_addr, 256, irq_ctrl, irq_num)
    {
        reset();
    }
    
    void reset(void)
    {
        for (int i=0;i<NUM_TIMERS;i++)
        {
            m_reg_ctrl[i] = 0;
            m_reg_cmp[i]  = 0;
            m_reg_val[i]  = 0;
        }
    }

    bool write32(uint32_t address, uint32_t data)
    {
        address -= m_base;

        switch (address)
        {
            case 0x08 + TIMER_CTRL:
                m_reg_ctrl[0] = data;
            break;
            case 0x08 + TIMER_CMP:
                m_reg_cmp[0] = data;
            break;
            case 0x08 + TIMER_VAL:
                m_reg_val[0] = data;
            break;
            case 0x14 + TIMER_CTRL:
                m_reg_ctrl[1] = data;
            break;
            case 0x14 + TIMER_CMP:
                m_reg_cmp[1] = data;
            break;
            case 0x14 + TIMER_VAL:
                m_reg_val[1] = data;
            break;
            default:
                fprintf(stderr, "TimerOwl: Bad write @ %08x\n", address);
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
            case 0x08 + TIMER_CTRL:
                data = m_reg_ctrl[0];
            break;
            case 0x08 + TIMER_CMP:
                data = m_reg_cmp[0];
            break;
            case 0x08 + TIMER_VAL:
                data = m_reg_val[0];
            break;
            case 0x14 + TIMER_CTRL:
                data = m_reg_ctrl[1];
            break;
            case 0x14 + TIMER_CMP:
                data = m_reg_cmp[1];
            break;
            case 0x14 + TIMER_VAL:
                data = m_reg_val[1];
            break;
            default:
                fprintf(stderr, "TimerOwl: Bad read @ %08x\n", address);
                return false;
            break;
        }

        return true;
    }

    int clock(void)
    {
        bool irq[NUM_TIMERS] = {false, false};

        for (int i=0;i<NUM_TIMERS;i++)
        {
            uint32_t enable  = (m_reg_ctrl[i] & (1 << TIMER_CTRL_EN_SHIFT))    != 0;
            uint32_t int_en  = (m_reg_ctrl[i] & (1 << TIMER_CTRL_INTEN_SHIFT)) != 0;

            // Timer disabled
            if (enable)
                m_reg_val[i] += 1;

            // Timer expired
            if (m_reg_val[i] == m_reg_cmp[i])
                irq[i] = enable && int_en;
        }

        // Interrupt generated
        if (irq[1] || irq[0])
            raise_interrupt();

        return 0;
    }

private:
    uint32_t m_reg_ctrl[NUM_TIMERS];
    uint32_t m_reg_cmp[NUM_TIMERS];
    uint32_t m_reg_val[NUM_TIMERS];
};

#endif
