//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_SYSTICK_H__
#define __DEVICE_SYSTICK_H__

#include "device.h"

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define TIMER_CSR         0x0
    #define TIMER_CSR_ENABLE                     0
    #define TIMER_CSR_ENABLE_SHIFT               0
    #define TIMER_CSR_ENABLE_MASK                0x1

    #define TIMER_CSR_INTERRUPT                  1
    #define TIMER_CSR_INTERRUPT_SHIFT            1
    #define TIMER_CSR_INTERRUPT_MASK             0x1

#define TIMER_RVR         0x4
    #define TIMER_RVR_RELOAD_SHIFT               0
    #define TIMER_RVR_RELOAD_MASK                0xffffffff

#define TIMER_CVR         0x8
    #define TIMER_CVR_CURRENT_SHIFT              0
    #define TIMER_CVR_CURRENT_MASK               0xffffffff

//-----------------------------------------------------------------
// TimerSystick: Model of Systick Timer IP
//-----------------------------------------------------------------
class device_systick: public device
{
public:
    device_systick(uint32_t base_addr, device *irq_ctrl, int irq_num): device("systick", base_addr, 32, irq_ctrl, irq_num)
    {
        reset();
    }
    
    void reset(void)
    {
        m_irq         = false;
        m_reg_csr     = 0;
        m_reg_reload  = 0;
        m_reg_current = 0;
    }

    bool write32(uint32_t address, uint32_t data)
    {
        address -= m_base;
        switch (address)
        {
            case TIMER_CSR:
                m_reg_csr = data;                
            break;
            case TIMER_RVR:
                m_reg_reload = data;
            break;
            default:
                fprintf(stderr, "TimerSystick: Bad write @ %08x\n", address);
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
            case TIMER_CSR:
                data = m_reg_csr;
            break;
            case TIMER_RVR:
                data = m_reg_reload;
            break;
            case TIMER_CVR:
                data = m_reg_current;
            break;
            default:
                fprintf(stderr, "TimerSystick: Bad read @ %08x\n", address);
                return false;
        }
        return true;
    }

    int clock(void)
    {
        // Timer disabled
        if (!(m_reg_csr & (1 << TIMER_CSR_ENABLE_SHIFT)))
        {
            m_reg_current = m_reg_reload;
            m_irq         = false;
        }
        // Timer expired
        else if (m_reg_current == 0)
        {
            m_reg_current = m_reg_reload;
            m_irq         = true;
        }
        else
            m_reg_current-= 1;

        // Interrupts enabled
        if (m_irq && (m_reg_csr & (1 << TIMER_CSR_INTERRUPT_SHIFT)))
        {
            raise_interrupt();
            m_irq = false;
        }
        return 0;
    }

private:
    uint32_t m_base_addr;
    bool     m_irq;
    uint32_t m_reg_csr;
    uint32_t m_reg_reload;
    uint32_t m_reg_current;
};

#endif
