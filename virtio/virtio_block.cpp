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

#include "cpu.h"
#include "virtio_block.h"

//--------------------------------------------------------------------
// Defines:
//--------------------------------------------------------------------
#define VIRTIO_BLK_T_IN          0
#define VIRTIO_BLK_T_OUT         1
#define VIRTIO_BLK_T_FLUSH       4
#define VIRTIO_BLK_T_FLUSH_OUT   5

#define VIRTIO_BLK_S_OK          0
#define VIRTIO_BLK_S_IOERR       1
#define VIRTIO_BLK_S_UNSUPP      2

#define SECTOR_SIZE              512

//-----------------------------------------------------------------
// Structures
//-----------------------------------------------------------------
typedef struct
{
    uint32_t type;
    uint32_t ioprio;
    uint64_t sector_num;
} t_virtio_block_hdr;

//--------------------------------------------------------------------
// Construction:
//--------------------------------------------------------------------
virtio_block::virtio_block(virtio *virtio)
{
    m_fp      = NULL;
    m_virtio  = virtio;
    m_clk_div = 0;
}
//--------------------------------------------------------------------
// open:
//--------------------------------------------------------------------
bool virtio_block::open(const char *filename)
{
    m_fp = fopen(filename, "rb+");
    if (!m_fp)
        return false;

    fseek(m_fp, 0, SEEK_END);
    int64_t file_size = ftello(m_fp);
    fseek(m_fp, 0, SEEK_SET);

    uint64_t num_sectors = (file_size + 511) / 512;
    m_virtio->m_cfg_space[0] = num_sectors >> 0;
    m_virtio->m_cfg_space[1] = num_sectors >> 32;

    m_virtio->set_device(this, 2, 0xFFFF, 0);

    return true;
}
//--------------------------------------------------------------------
// read_block:
//--------------------------------------------------------------------
bool virtio_block::read_block(uint64_t sector_num, uint8_t *buf, int num_sectors)
{
    if (!m_fp)
        return false;

    fseek(m_fp, sector_num * SECTOR_SIZE, SEEK_SET);
    int res = fread(buf, 1, num_sectors * SECTOR_SIZE, m_fp);
    return res == (num_sectors * SECTOR_SIZE);
}
//--------------------------------------------------------------------
// write_block:
//--------------------------------------------------------------------
bool virtio_block::write_block(uint64_t sector_num, uint8_t *buf, int num_sectors)
{
    if (!m_fp)
        return false;

    fseek(m_fp, sector_num * SECTOR_SIZE, SEEK_SET);
    int res = fwrite(buf, 1, num_sectors * SECTOR_SIZE, m_fp);
    return res == (num_sectors * SECTOR_SIZE);
}
//--------------------------------------------------------------------
// request:
//--------------------------------------------------------------------
bool virtio_block::request(int queue_idx, int desc_idx, int read_size, int write_size)
{
    t_virtio_block_hdr h;
    bool ok;

    // Get request header
    if (!m_virtio->copy_from_queue((uint8_t*)&h, queue_idx, desc_idx, 0, sizeof(h)))
        return false;

    switch(h.type)
    {
    // Storage read
    case VIRTIO_BLK_T_IN:
    {
        uint8_t *buf = (uint8_t*)malloc(write_size);
        
        ok = read_block(h.sector_num, buf, (write_size - 1) / SECTOR_SIZE);
        
        if (!ok)
            buf[write_size - 1] = VIRTIO_BLK_S_IOERR;
        else
            buf[write_size - 1] = VIRTIO_BLK_S_OK;

        m_virtio->copy_to_queue(queue_idx, desc_idx, 0, buf, write_size);
        free(buf);
        buf = NULL;

        m_virtio->consume_desc(queue_idx, desc_idx, write_size);
    }
    break;
    // Storage write
    case VIRTIO_BLK_T_OUT:
    {
        assert(write_size >= 1);
        int len = read_size - sizeof(h);
        uint8_t *buf = (uint8_t*)malloc(len);

        m_virtio->copy_from_queue(buf, queue_idx, desc_idx, sizeof(h), len);
        
        ok = write_block(h.sector_num, buf, len / SECTOR_SIZE);

        free(buf);
        buf = NULL;

        uint8_t resp_buf[1];
        if (!ok)
            resp_buf[0] = VIRTIO_BLK_S_IOERR;
        else
            resp_buf[0] = VIRTIO_BLK_S_OK;
        m_virtio->copy_to_queue(queue_idx, desc_idx, 0, resp_buf, sizeof(resp_buf));
        m_virtio->consume_desc(queue_idx, desc_idx, 1);
    }
    break;
    default:
        break;
    }
    return true;
}
//--------------------------------------------------------------------
// clock:
//--------------------------------------------------------------------
int virtio_block::clock(void) 
{ 
    if (m_clk_div++ < 100)
        return 0;
    m_clk_div = 0;

    int queue_idx = 0;
    int read_size, write_size;

    if (m_virtio->m_queue[queue_idx].last_avail_idx != m_virtio->get_avail_idx(queue_idx))
    {
        int desc_idx = m_virtio->get_avail_value(queue_idx, m_virtio->m_queue[queue_idx].last_avail_idx);

        if (m_virtio->get_desc_size(&read_size, &write_size, queue_idx, desc_idx))
            request(queue_idx, desc_idx, read_size, write_size);

        m_virtio->m_queue[queue_idx].last_avail_idx++;
    }

    return 0; 
}
