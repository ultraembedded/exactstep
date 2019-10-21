//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __DEVICE_SPI_LITE_H__
#define __DEVICE_SPI_LITE_H__

#include <queue>
#include "device.h"

#define dprintf    //printf

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define SPI_FIFO_DEPTH  4
#define SPI_CLOCK_DIV   32

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define SPI_DGIER         0x1c
    #define SPI_DGIER_GIE                        31
    #define SPI_DGIER_GIE_SHIFT                  31
    #define SPI_DGIER_GIE_MASK                   0x1

#define SPI_IPISR         0x20
    #define SPI_IPISR_TX_EMPTY                   2
    #define SPI_IPISR_TX_EMPTY_SHIFT             2
    #define SPI_IPISR_TX_EMPTY_MASK              0x1

#define SPI_IPIER         0x28
    #define SPI_IPIER_TX_EMPTY                   2
    #define SPI_IPIER_TX_EMPTY_SHIFT             2
    #define SPI_IPIER_TX_EMPTY_MASK              0x1

#define SPI_SRR           0x40
    #define SPI_SRR_RESET_SHIFT                  0
    #define SPI_SRR_RESET_MASK                   0xffffffff

#define SPI_CR            0x60
    #define SPI_CR_LOOP                          0
    #define SPI_CR_LOOP_SHIFT                    0
    #define SPI_CR_LOOP_MASK                     0x1

    #define SPI_CR_SPE                           1
    #define SPI_CR_SPE_SHIFT                     1
    #define SPI_CR_SPE_MASK                      0x1

    #define SPI_CR_MASTER                        2
    #define SPI_CR_MASTER_SHIFT                  2
    #define SPI_CR_MASTER_MASK                   0x1

    #define SPI_CR_CPOL                          3
    #define SPI_CR_CPOL_SHIFT                    3
    #define SPI_CR_CPOL_MASK                     0x1

    #define SPI_CR_CPHA                          4
    #define SPI_CR_CPHA_SHIFT                    4
    #define SPI_CR_CPHA_MASK                     0x1

    #define SPI_CR_TXFIFO_RST                    5
    #define SPI_CR_TXFIFO_RST_SHIFT              5
    #define SPI_CR_TXFIFO_RST_MASK               0x1

    #define SPI_CR_RXFIFO_RST                    6
    #define SPI_CR_RXFIFO_RST_SHIFT              6
    #define SPI_CR_RXFIFO_RST_MASK               0x1

    #define SPI_CR_MANUAL_SS                     7
    #define SPI_CR_MANUAL_SS_SHIFT               7
    #define SPI_CR_MANUAL_SS_MASK                0x1

    #define SPI_CR_TRANS_INHIBIT                 8
    #define SPI_CR_TRANS_INHIBIT_SHIFT           8
    #define SPI_CR_TRANS_INHIBIT_MASK            0x1

    #define SPI_CR_LSB_FIRST                     9
    #define SPI_CR_LSB_FIRST_SHIFT               9
    #define SPI_CR_LSB_FIRST_MASK                0x1

#define SPI_SR            0x64
    #define SPI_SR_RX_EMPTY                      0
    #define SPI_SR_RX_EMPTY_SHIFT                0
    #define SPI_SR_RX_EMPTY_MASK                 0x1

    #define SPI_SR_RX_FULL                       1
    #define SPI_SR_RX_FULL_SHIFT                 1
    #define SPI_SR_RX_FULL_MASK                  0x1

    #define SPI_SR_TX_EMPTY                      2
    #define SPI_SR_TX_EMPTY_SHIFT                2
    #define SPI_SR_TX_EMPTY_MASK                 0x1

    #define SPI_SR_TX_FULL                       3
    #define SPI_SR_TX_FULL_SHIFT                 3
    #define SPI_SR_TX_FULL_MASK                  0x1

#define SPI_DTR           0x68
    #define SPI_DTR_DATA_SHIFT                   0
    #define SPI_DTR_DATA_MASK                    0xff

#define SPI_DRR           0x6c
    #define SPI_DRR_DATA_SHIFT                   0
    #define SPI_DRR_DATA_MASK                    0xff

#define SPI_SSR           0x70
    #define SPI_SSR_VALUE                        0
    #define SPI_SSR_VALUE_SHIFT                  0
    #define SPI_SSR_VALUE_MASK                   0x1

//-----------------------------------------------------------------
// SpiLite: Model of Xilinx SPI-Lite IP
//-----------------------------------------------------------------
class device_spi_lite: public device
{
public:
    device_spi_lite(uint32_t base_addr, device *irq_ctrl, int irq_num): device("spi_lite", base_addr, 256, irq_ctrl, irq_num)
    {
        reset();
    }
    
    void reset(void)
    {
        memset(&m_reg, 0, 256 * 4);
        m_spi_delay   = 0;
        m_irq_inhibit = false;
    }

    void tx_push(uint32_t data)
    {
        if (m_tx.size() < SPI_FIFO_DEPTH)
            m_tx.push(data);
    }

    void rx_push(uint32_t data)
    {
        if (m_rx.size() < SPI_FIFO_DEPTH)
            m_rx.push(data);
    }    

    void tx_flush(void)
    {
        while (!m_tx.empty())
            m_tx.pop();
    }

    void rx_flush(void)
    {
        while (!m_rx.empty())
            m_rx.pop();
    }

    bool write32(uint32_t address, uint32_t data)
    {
        dprintf("SPI: Write %08x=%08x\n", address, data);
        address -= m_base;
        switch (address)
        {
            case SPI_DTR:
                tx_push(data);
            break;
            case SPI_CR:
                // Tx FIFO flush
                if (data & (1 << SPI_CR_TXFIFO_RST_SHIFT))
                {
                    tx_flush();
                    data &= ~(1 << SPI_CR_TXFIFO_RST_SHIFT);
                }
                // Rx FIFO flush
                if (data & (1 << SPI_CR_RXFIFO_RST_SHIFT))
                {
                    rx_flush();
                    data &= ~(1 << SPI_CR_RXFIFO_RST_SHIFT);
                }
            break;
            case SPI_SRR:
                // Soft reset
                if (data == 0x0000000A)
                {
                    tx_flush();
                    rx_flush();
                    data = 0;
                }
            break;
            case SPI_IPISR:
                data = m_reg[address/4] & ~data;
                m_irq_inhibit = false;
            break;
        }

        m_reg[address/4] = data;
        return true;
    }
    bool read32(uint32_t address, uint32_t &data)
    {
        address -= m_base;
        switch (address)
        {
            case SPI_DRR:
                if (m_rx.size() != 0)
                {
                    m_reg[address/4] = m_rx.front();
                    m_rx.pop();
                }
            break;
            case SPI_SR:
            {
                bool tx_full  = m_tx.size() == SPI_FIFO_DEPTH;
                bool tx_empty = m_tx.size() == 0;
                bool rx_empty = m_rx.size() == 0;
                bool rx_full  = m_rx.size() == SPI_FIFO_DEPTH;

                m_reg[address/4] = (tx_full  << SPI_SR_TX_FULL_SHIFT)  | 
                                  (tx_empty << SPI_SR_TX_EMPTY_SHIFT) | 
                                  (rx_empty << SPI_SR_RX_EMPTY_SHIFT) |
                                  (rx_full  << SPI_SR_RX_FULL_SHIFT);
            }
            break;
        }
        dprintf("SPI: Read %08x=%08x\n", address, m_reg[address/4]);
        data = m_reg[address/4];
        return true;
    }

    int clock(void)
    {
        bool enable          = (m_reg[SPI_CR/4] & (1 << SPI_CR_SPE_SHIFT)) != 0;
        bool loopback        = (m_reg[SPI_CR/4] & (1 << SPI_CR_LOOP_SHIFT)) != 0;
        bool trans_inhibit   = (m_reg[SPI_CR/4] & (1 << SPI_CR_TRANS_INHIBIT_SHIFT)) != 0;
        bool global_int_en   = (m_reg[SPI_DGIER/4] & (1 << SPI_DGIER_GIE_SHIFT)) != 0;
        bool tx_empty_irq_en = (m_reg[SPI_IPIER/4] & (1 << SPI_IPIER_TX_EMPTY_SHIFT)) != 0;
        bool tx_empty_event  = false;

        if (m_spi_delay == 0)
        {
            if (enable && !trans_inhibit && m_tx.size() != 0)
            {
                if (loopback)
                    rx_push(m_tx.front());
                else 
                    rx_push(0xFF);
                m_tx.pop();

                if (m_tx.size() == 0)
                    tx_empty_event = true;
            }
            m_spi_delay = SPI_CLOCK_DIV;
        }
        else if (enable && !trans_inhibit)
            m_spi_delay -= 1;

        // Tx empty event, set interrupt
        if (tx_empty_event)
            m_reg[SPI_IPISR/4] |= (1 << SPI_IPISR_TX_EMPTY_SHIFT);

        if (!m_irq_inhibit && global_int_en && tx_empty_irq_en && (m_reg[SPI_IPISR/4] & (1 << SPI_IPISR_TX_EMPTY_SHIFT)))
        {
            m_irq_inhibit = true;
            raise_interrupt();
        }

        return 0;
    }

private:
    uint32_t m_reg[256];
    std::queue <uint32_t> m_tx;
    std::queue <uint32_t> m_rx;
    uint32_t m_spi_delay;
    bool     m_irq_inhibit;
};

#endif
