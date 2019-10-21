//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                     License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "memory.h"

//--------------------------------------------------------------------
// Device base class
//--------------------------------------------------------------------
class device: public memory_base
{
public:
    device(std::string name, uint32_t base, uint32_t size, device *irq_ctrl = NULL, int irq = -1):
    memory_base(name, base, size)
    {
        m_irq_ctrl   = irq_ctrl;
        m_irq_number = irq;
        m_irq_raised = false;
        device_next  = NULL;
    }

    virtual void set_irq(int irq) { }
    virtual int  get_irq(void)
    {
        bool irq = m_irq_raised;
        m_irq_raised = false;
        return irq ? m_irq_number : -1; 
    } 

    virtual void raise_interrupt(void)
    {
        if (m_irq_ctrl)
            m_irq_ctrl->set_irq(m_irq_number);
        else
            m_irq_raised = true;
    }

    virtual int  min_access_size(void) { return 4; }

    virtual bool write8(uint32_t addr, uint8_t data)
    {
        printf("ERROR: write8 not supported @ 0x%08x\n", addr);
        return false;
    }

    virtual bool write_block(uint32_t addr,  uint8_t *data, int length)
    {
        printf("ERROR: write_block not supported @ 0x%08x\n", addr);
        return false;
    }

    virtual bool read8(uint32_t addr, uint8_t &data)
    {
        printf("ERROR: read8 not supported @ 0x%08x\n", addr);
        return false;
    }

    virtual bool read_block(uint32_t addr,  uint8_t *data, int length)
    {
        printf("ERROR: read_block not supported @ 0x%08x\n", addr);
        return false;
    }

public:
    device*          device_next;

protected:
    int              m_irq_number;
    device         * m_irq_ctrl;
    bool             m_irq_raised;
};

#endif
