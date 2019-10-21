//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_UART_LITE_H__
#define __DEVICE_UART_LITE_H__

#include "device.h"
#include "console_io.h"

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define ULITE_RX          0x0
    #define ULITE_RX_DATA_SHIFT                  0
    #define ULITE_RX_DATA_MASK                   0xff

#define ULITE_TX          0x4
    #define ULITE_TX_DATA_SHIFT                  0
    #define ULITE_TX_DATA_MASK                   0xff

#define ULITE_STATUS      0x8
    #define ULITE_STATUS_IE                      4
    #define ULITE_STATUS_IE_SHIFT                4
    #define ULITE_STATUS_IE_MASK                 0x1

    #define ULITE_STATUS_TXFULL                  3
    #define ULITE_STATUS_TXFULL_SHIFT            3
    #define ULITE_STATUS_TXFULL_MASK             0x1

    #define ULITE_STATUS_TXEMPTY                 2
    #define ULITE_STATUS_TXEMPTY_SHIFT           2
    #define ULITE_STATUS_TXEMPTY_MASK            0x1

    #define ULITE_STATUS_RXFULL                  1
    #define ULITE_STATUS_RXFULL_SHIFT            1
    #define ULITE_STATUS_RXFULL_MASK             0x1

    #define ULITE_STATUS_RXVALID                 0
    #define ULITE_STATUS_RXVALID_SHIFT           0
    #define ULITE_STATUS_RXVALID_MASK            0x1

#define ULITE_CONTROL     0xc
    #define ULITE_CONTROL_IE                     4
    #define ULITE_CONTROL_IE_SHIFT               4
    #define ULITE_CONTROL_IE_MASK                0x1

    #define ULITE_CONTROL_RST_RX                 1
    #define ULITE_CONTROL_RST_RX_SHIFT           1
    #define ULITE_CONTROL_RST_RX_MASK            0x1

    #define ULITE_CONTROL_RST_TX                 0
    #define ULITE_CONTROL_RST_TX_SHIFT           0
    #define ULITE_CONTROL_RST_TX_MASK            0x1

//-----------------------------------------------------------------
// UartLite: Model of Xilinx UART-Lite IP
//-----------------------------------------------------------------
class device_uart_lite: public device
{
public:
    device_uart_lite(uint32_t base_addr, device *irq_ctrl, int irq_num, console_io *con_io): device("uart_lite", base_addr, 256, irq_ctrl, irq_num)
    {
        m_console    = con_io;

        assert(m_console != NULL);

        reset();
    }
    
    void reset(void)
    {
        m_irq = false;
        m_rx  = -1;
        m_ctrl = 0;
    }

    bool write32(uint32_t address, uint32_t data)
    {
        address -= m_base;
        switch (address)
        {
            case ULITE_TX:
                m_console->putchar(data);

                // Transmit -> empty
                m_irq = true;                
            break;
            case ULITE_CONTROL:
                m_ctrl = data;
            break;
            default:
                fprintf(stderr, "UARTLITE: Bad write @ %08x\n", address);
                exit (-1);
            break;
        }
        return true;
    }
    bool read32(uint32_t address, uint32_t &data)
    {
        data = 0;
        address -= m_base;

        // Poll for new character
        if (m_rx == -1)
        {
            m_rx = m_console->getchar();
            if (m_rx != -1)
                m_irq = true;
        }

        switch (address)
        {
            case ULITE_RX:
                data = ((uint32_t)m_rx) & ULITE_RX_DATA_MASK;
                m_rx = -1;
            break;
            case ULITE_CONTROL:
                data = m_ctrl;
            break;
            case ULITE_STATUS:
                data |= ((m_ctrl >> ULITE_CONTROL_IE_SHIFT) & 0x1) << ULITE_STATUS_IE_SHIFT;
                data |= 0 << ULITE_STATUS_TXFULL_SHIFT;
                data |= 1 << ULITE_STATUS_TXEMPTY_SHIFT;
                data |= 0 << ULITE_STATUS_RXFULL_SHIFT;
                data |= (m_rx != -1) << ULITE_STATUS_RXVALID_SHIFT;
            break;
            default:
                fprintf(stderr, "UARTLITE: Bad read @ %08x\n", address);
                exit (-1);
            break;
        }
        return true;
    }

    int clock(void)
    {
        // No rx char in the buffer, poll again...
        if (m_rx == -1)
        {
            static int poll_count = 0;

            if (poll_count++ == 1024)
            {
                poll_count = 0;
                m_rx = m_console->getchar();
                if (m_rx != -1)
                    m_irq = true;
            }
        }

        // Interrupts enabled
        if (m_irq && (m_ctrl & (1 << ULITE_CONTROL_IE_SHIFT)))
        {
            raise_interrupt();
            m_irq = false;
        }

        return 0;
    }

private:
    bool     m_irq;
    console_io *m_console;
    uint32_t m_ctrl;
    int      m_rx;
};

#endif
