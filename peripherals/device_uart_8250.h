//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_UART_8250_H__
#define __DEVICE_UART_8250_H__

#include "device.h"
#include "console_io.h"

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define UART8250_RBR_OFFSET     0   // READ:  Recieve Buffer Register
#define UART8250_THR_OFFSET     0   // WRITE: Transmitter Holding Register
#define UART8250_IER_OFFSET     1   // READ/WRITE: Interrupt Enable Register
#define UART8250_FCR_OFFSET     2   // WRITE: FIFO Control Register
#define UART8250_IIR_OFFSET     2   // READ/WRITE: Interrupt Identification Register
#define UART8250_LCR_OFFSET     3   // WRITE: Line Control Register
#define UART8250_MCR_OFFSET     4   // WRITE: Modem Control Register
#define UART8250_LSR_OFFSET     5   // READ:  Line Status Register
#define UART8250_MSR_OFFSET     6   // READ:  Modem Status Register
#define UART8250_SCR_OFFSET     7   // READ/WRITE: Scratch Register
#define UART8250_MDR1_OFFSET    8   // READ/WRITE:  Mode Register

// DLAB = 1
#define UART8250_DLAB_OFFSET    8
#define UART8250_DLL_OFFSET     (UART8250_DLAB_OFFSET+0)   // WRITE: Divisor Latch Low
#define UART8250_DLM_OFFSET     (UART8250_DLAB_OFFSET+1)   // WRITE: Divisor Latch High

#define UART8250_REG_SIZE       16

#define UART8250_LSR_FIFOE      0x80    // Fifo error
#define UART8250_LSR_TEMT       0x40    // Transmitter empty
#define UART8250_LSR_THRE       0x20    // Transmit-hold-register empty
#define UART8250_LSR_BI         0x10    // Break interrupt indicator
#define UART8250_LSR_FE         0x08    // Frame error indicator
#define UART8250_LSR_PE         0x04    // Parity error indicator
#define UART8250_LSR_OE         0x02    // Overrun error indicator
#define UART8250_LSR_DR         0x01    // Receiver data ready

#define UART8250_LCR_DLAB       0x80

//-----------------------------------------------------------------
// Simplified model of 8250 UART
//-----------------------------------------------------------------
class device_uart_8250: public device
{
public:
    device_uart_8250(uint32_t base_addr, device *irq_ctrl, int irq_num, console_io *con_io): device("uart_8250", base_addr, UART8250_REG_SIZE, irq_ctrl, irq_num)
    {
        m_console    = con_io;
        assert(m_console != NULL);

        reset();
    }
    
    void reset(void)
    {
        memset(m_reg, 0, UART8250_REG_SIZE);
        m_reg[UART8250_LSR_OFFSET] = UART8250_LSR_TEMT | UART8250_LSR_THRE;
        m_rx  = -1;
        m_poll_count = 0;
    }

    bool write8(uint32_t address, uint8_t data)
    {
        address -= m_base;

        // Divisor Latch Access Bit
        if (m_reg[UART8250_LCR_OFFSET] & UART8250_LCR_DLAB)
        {
            switch (address)
            {
                case UART8250_DLL_OFFSET-UART8250_DLAB_OFFSET:
                    m_reg[address] = data;
                    return true;
                case UART8250_DLM_OFFSET-UART8250_DLAB_OFFSET:
                    m_reg[address] = data;
                    return true;
            }
        }

        m_reg[address] = data;
        //printf("UART8250: Write8 %08x=%08x\n", address, data);

        switch (address)
        {
            case UART8250_THR_OFFSET:
                m_console->putchar(data);
                m_reg[UART8250_LSR_OFFSET] = UART8250_LSR_TEMT | UART8250_LSR_THRE;
                break;
        }

        return false;
    }

    bool read8(uint32_t address, uint8_t &data)
    {
        address -= m_base;        

        if (address == UART8250_RBR_OFFSET)
            m_rx = -1;

        if (m_rx != -1)
            m_reg[UART8250_LSR_OFFSET] |= UART8250_LSR_DR;
        else
            m_reg[UART8250_LSR_OFFSET] &= ~UART8250_LSR_DR;

        data = m_reg[address];

        return false;
    }

    // 8-bit device...
    bool write32(uint32_t address, uint32_t data) { return false; }
    bool read32(uint32_t address, uint32_t &data) { return false; }

    int clock(void)
    {
        // No rx char in the buffer, poll again...
        if (m_rx == -1)
        {
            if (m_poll_count++ == 1024)
            {
                m_poll_count = 0;
                m_rx = m_console->getchar();
                m_reg[UART8250_RBR_OFFSET] = (uint8_t)m_rx;
            }
        }

        return 0;
    }

private:
    console_io *m_console;
    uint8_t  m_reg[UART8250_REG_SIZE];
    int      m_rx;
    int      m_poll_count;
};

#endif
