//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <assert.h>

#ifdef INCLUDE_NET_DEVICE
#include "cpu.h"
#include "virtio_net.h"
#include "net_tap.h"

//--------------------------------------------------------------------
// Defines:
//--------------------------------------------------------------------
#define VIRTIO_MAX_MTU 1600

//--------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------
typedef struct
{
    uint8_t  flags;
    uint8_t  gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
    uint16_t num_buffers;
} t_virtio_net_hdr;

//--------------------------------------------------------------------
// Construction:
//--------------------------------------------------------------------
virtio_net::virtio_net(virtio *virtio)
{
    m_net     = NULL;
    m_virtio  = virtio;
    m_clk_div = 0;
}
//--------------------------------------------------------------------
// open:
//--------------------------------------------------------------------
bool virtio_net::open(const char *tap_device, uint8_t *mac_addr)
{
    m_net = new net_tap(tap_device);

    m_virtio->set_device(this, 1, 0xFFFF, mac_addr ? (1 << 5) : 0);
    if (mac_addr)
    {
        uint8_t *p = (uint8_t*)&m_virtio->m_cfg_space[0];
        memcpy(p, mac_addr, 6);
    }

    return true;
}
//--------------------------------------------------------------------
// has_rx_space: Space in the receive queue (0)
//--------------------------------------------------------------------
bool virtio_net::has_rx_space(void)
{
    if (!m_virtio->m_queue[0].ready)
        return false;

    return m_virtio->m_queue[0].last_avail_idx != m_virtio->get_avail_idx(0);
}
//--------------------------------------------------------------------
// clock:
//--------------------------------------------------------------------
int virtio_net::clock(void) 
{
    uint8_t packet[VIRTIO_MAX_MTU];

    if (m_clk_div++ < 100)
        return 0;
    m_clk_div = 0;

    // Process network receive
    if (has_rx_space())
    {
        int packet_len = m_net->receive(packet, sizeof(packet));
        if (packet_len > 0)
        {
            int queue_idx = 0;
            int read_size, write_size;

            int desc_idx = m_virtio->get_avail_value(queue_idx, m_virtio->m_queue[queue_idx].last_avail_idx);
            if (m_virtio->get_desc_size(&read_size, &write_size, queue_idx, desc_idx))
            {
                t_virtio_net_hdr h;
                int hdr_size = sizeof(t_virtio_net_hdr);
                memset(&h, 0, hdr_size);

                int len = hdr_size + packet_len; 
                if (len <= write_size)
                {
                    m_virtio->copy_to_queue(queue_idx, desc_idx, 0, (uint8_t*)&h, hdr_size);
                    m_virtio->copy_to_queue(queue_idx, desc_idx, hdr_size, packet, packet_len);
                    m_virtio->consume_desc(queue_idx, desc_idx, len);
                    m_virtio->m_queue[queue_idx].last_avail_idx++;
                }
            }
        }
    }

    // Process network transmit
    if (m_virtio->m_queue[1].notify)
    {
        int queue_idx = 1;
        int desc_idx, read_size, write_size;

        uint16_t avail_idx = m_virtio->get_avail_idx(queue_idx);
        if (m_virtio->m_queue[queue_idx].last_avail_idx != avail_idx)
        {
            desc_idx = m_virtio->get_avail_value(queue_idx, m_virtio->m_queue[queue_idx].last_avail_idx);
            if (m_virtio->get_desc_size(&read_size, &write_size, queue_idx, desc_idx))
            {
                if (read_size < VIRTIO_MAX_MTU)
                {
                    m_virtio->copy_from_queue(packet, queue_idx, desc_idx, 0, read_size);
                    m_net->send(&packet[12], read_size - 12);
                }

                m_virtio->consume_desc(queue_idx, desc_idx, 0);
            }
            m_virtio->m_queue[queue_idx].last_avail_idx++;
            m_virtio->m_queue[queue_idx].notify--;
        }

    }    

    return 0; 
}
#endif