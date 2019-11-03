//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_IRQ_PLIC_H__
#define __DEVICE_IRQ_PLIC_H__

#include "device.h"

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define PLIC_REG_SRC_PRIO0           0x0000
#define PLIC_REG_SRC_PRIO127         0x01fc
#define PLIC_REG_PENDING0            0x1000
#define PLIC_REG_PENDING3            0x100c
#define PLIC_REG_ENABLE0             0x2000
#define PLIC_REG_ENABLE3             0x200c
#define PLIC_REG_PRIO_THRESH         0x00200000
#define PLIC_REG_CLAIM               0x00200004

#define PLIC_NUM_IRQS                128
#define PLIC_REG_SIZE                (0x10000000 - 0x0c000000)
#define PLIC_IRQ_GROUPS              4

//-----------------------------------------------------------------
// device_irq_plic: 'Platform Interrupt Controller' model
//-----------------------------------------------------------------
class device_irq_plic: public device
{
public:
    device_irq_plic(uint32_t base_addr, int irq): device("plic", base_addr, PLIC_REG_SIZE, NULL, irq)
    {
        reset();
    }
    
    void reset(void)
    {
        for (int x=0;x<PLIC_IRQ_GROUPS;x++)
        {
            m_pending[x] = 0;
            m_enable[x] = 0;
        }
        for (int i=0;i<PLIC_NUM_IRQS;i++)
            m_prio[i] = 0;

        m_prio_thresh = 0;
        m_irq = false;
    }

    int eval_irq(void)
    {
        uint32_t masked[PLIC_IRQ_GROUPS];

        for (int x=0;x<PLIC_IRQ_GROUPS;x++)
            masked[x] = m_pending[x] & m_enable[x];

        // Mask interrupts less than or equal to threshold
        for (int i=0;i<PLIC_NUM_IRQS;i++)
            if (m_prio[i] <= m_prio_thresh)
            {
                int grp = i / 32;
                int bit = i % 32;
                masked[grp] &= ~(1  << bit);
            }

        m_irq = false;
        for (int x=0;x<PLIC_IRQ_GROUPS;x++)
            for (int i=0;i<32;i++)
                if (masked[x] & (1 << i))
                {
                    m_irq = true;
                    return (x*32) + i;
                }
        return 0; // Interrupt 0 not valid
    }

    void set_irq(int irq)
    {
        if (irq != -1)
        {
            int grp = irq / 32;
            int bit = irq % 32;
            if (grp < PLIC_IRQ_GROUPS)
                m_pending[grp] |= (1 << bit);
        }

        // Interrupt 0 is hard-wired to zero
        m_pending[0] &= ~(1 << 0);

        eval_irq();
    }

    int get_irq(void)
    {
        if (m_irq)
            return m_irq_number;
        else
            return -1;
    }

    bool write32(uint32_t address, uint32_t data)
    {
        address -= m_base;

        if (address >= PLIC_REG_SRC_PRIO0 && address <= PLIC_REG_SRC_PRIO127)
            m_prio[(address-PLIC_REG_SRC_PRIO0)/4] = data;
        else if (address >= PLIC_REG_ENABLE0 && address <= PLIC_REG_ENABLE3)
            m_enable[(address-PLIC_REG_ENABLE0)/4] = data;
        else if (address == PLIC_REG_PRIO_THRESH)
            m_prio_thresh = data;
        else
            fprintf(stderr, "PLIC: Bad write @ %08x\n", address);

        eval_irq();
        return true;
    }

    bool read32(uint32_t address, uint32_t &data)
    {
        data = 0;
        address -= m_base;

        if (address >= PLIC_REG_SRC_PRIO0 && address <= PLIC_REG_SRC_PRIO127)
        {
            data = m_prio[(address-PLIC_REG_SRC_PRIO0)/4];
            return true;
        }
        else if (address >= PLIC_REG_PENDING0 && address <= PLIC_REG_PENDING3)
        {
            data = m_pending[(address-PLIC_REG_PENDING0)/4];
            return true;
        }
        else if (address >= PLIC_REG_ENABLE0 && address <= PLIC_REG_ENABLE3)
        {
            data = m_enable[(address-PLIC_REG_ENABLE0)/4];
            return true;
        }
        else if (address == PLIC_REG_PRIO_THRESH)
        {
            data = m_prio_thresh;
            return true;
        }
        else if (address == PLIC_REG_CLAIM)
        {
            int irq = eval_irq();

            // Interrupt claimed - clear
            if (irq >= 0)
            {
                int grp = irq / 32;
                int bit = irq % 32;
                m_pending[grp] &= ~(1 << bit);
            }

            data = (uint32_t)irq;
        }

        fprintf(stderr, "PLIC: Bad read @ %08x\n", address);
        return false;
    }

    int clock(void)
    {
        return 0;
    }

private:
    uint8_t  m_prio[PLIC_NUM_IRQS];
    uint32_t m_pending[PLIC_IRQ_GROUPS];
    uint32_t m_enable[PLIC_IRQ_GROUPS];
    uint8_t  m_prio_thresh;
    bool     m_irq;
};

#endif
