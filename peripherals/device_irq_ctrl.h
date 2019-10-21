//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_IRQ_CTRL_H__
#define __DEVICE_IRQ_CTRL_H__

#include "device.h"

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define IRQ_ISR           0x0
    #define IRQ_ISR_STATUS_SHIFT                 0
    #define IRQ_ISR_STATUS_MASK                  0x3

#define IRQ_IPR           0x4
    #define IRQ_IPR_PENDING_SHIFT                0
    #define IRQ_IPR_PENDING_MASK                 0x3

#define IRQ_IER           0x8
    #define IRQ_IER_ENABLE_SHIFT                 0
    #define IRQ_IER_ENABLE_MASK                  0x3

#define IRQ_IAR           0xc
    #define IRQ_IAR_ACK_SHIFT                    0
    #define IRQ_IAR_ACK_MASK                     0x3

#define IRQ_SIE           0x10
    #define IRQ_SIE_SET_SHIFT                    0
    #define IRQ_SIE_SET_MASK                     0x3

#define IRQ_CIE           0x14
    #define IRQ_CIE_CLR_SHIFT                    0
    #define IRQ_CIE_CLR_MASK                     0x3

#define IRQ_IVR           0x18
    #define IRQ_IVR_VECTOR_SHIFT                 0
    #define IRQ_IVR_VECTOR_MASK                  0xffffffff

#define IRQ_MER           0x1c
    #define IRQ_MER_ME                           0
    #define IRQ_MER_ME_SHIFT                     0
    #define IRQ_MER_ME_MASK                      0x1

//-----------------------------------------------------------------
// device_irq_ctrl: Basic IRQ controller somewhat compatible with Xilinx IP
//-----------------------------------------------------------------
class device_irq_ctrl: public device
{
public:
    device_irq_ctrl(uint32_t base_addr, int irq): device("irq_ctrl", base_addr, 256, NULL, irq)
    {
        reset();
    }
    
    void reset(void)
    {
        m_isr = 0;
        m_ier = 0;
        m_mer = 0;        
    }

    void set_irq(int irq)
    {
        if (irq != -1)
            m_isr |= (1 << irq);
    }

    int get_irq(void)
    {
        if ((m_isr & m_ier) != 0 && m_mer)
            return m_irq_number;
        else
            return -1;
    }

    bool write32(uint32_t address, uint32_t data)
    {
        address -= m_base;
        switch (address)
        {
            case IRQ_ISR:
                m_isr |= data;
            break;
            case IRQ_IER:
                m_ier = data;
            break;
            case IRQ_SIE:
                m_ier|= data;
            break;
            case IRQ_CIE:
                m_ier&= ~data;
            break;
            case IRQ_IAR:
                m_isr&= ~data;
            break;
            case IRQ_MER:
                m_mer = data & (IRQ_MER_ME_MASK << IRQ_MER_ME_SHIFT);
            break;
            default:
                fprintf(stderr, "IRQ-CTRL: Bad write @ %08x\n", address);
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
            case IRQ_ISR:
                data = m_isr;
            break;
            case IRQ_IPR:
                data = m_isr & m_ier;
            break;
            case IRQ_IER:
                data = m_ier;
            break;
            case IRQ_IVR:
                data = (uint32_t)-1;
                for (int i=0;i<32;i++)
                    if ((m_isr & m_ier) & (1 << i))
                    {
                        data = i;
                        break;
                    }
            break;
            case IRQ_MER:
                data = m_mer;
            break;
            default:
                fprintf(stderr, "IRQ-CTRL: Bad read @ %08x\n", address);
                return false;
            break;
        }
        return true;
    }

    int clock(void)
    {
        return 0;
    }

private:
    uint32_t m_base_addr;
    uint32_t m_isr;
    uint32_t m_ier;
    uint32_t m_mer;
};

#endif
