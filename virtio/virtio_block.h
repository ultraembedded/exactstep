//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __VIRTIO_BLOCK_H__
#define __VIRTIO_BLOCK_H__

#include "virtio.h"

//-----------------------------------------------------------------
// virtio_block: Block VirtIO device
//-----------------------------------------------------------------
class virtio_block: public virtio_device
{
public:
    virtio_block(virtio *virtio);

    bool open(const char *filename);

    virtual bool read_block(uint64_t sector_num, uint8_t *buf, int num_sectors);
    virtual bool write_block(uint64_t sector_num, uint8_t *buf, int num_sectors);

    bool request(int queue_idx, int desc_idx, int read_size, int write_size);
    int  clock(void);

protected:
    FILE   *m_fp;
    virtio *m_virtio;
    int     m_clk_div;
};

#endif