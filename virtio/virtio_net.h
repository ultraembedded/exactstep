//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __VIRTIO_NET_H__
#define __VIRTIO_NET_H__

#include "virtio.h"

class net_tap;

//-----------------------------------------------------------------
// virtio_net: Net VirtIO device
//-----------------------------------------------------------------
class virtio_net: public virtio_device
{
public:
    virtio_net(virtio *virtio);

    bool open(const char *tap_device, uint8_t *mac_addr);

    bool has_rx_space(void);

    int  clock(void);

protected:
    net_tap *m_net;
    virtio  *m_virtio;
    int      m_clk_div;

};

#endif