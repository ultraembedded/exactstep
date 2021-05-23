//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __VIRTIO_H__
#define __VIRTIO_H__

#include "device.h"

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define VIRTIO_QUEUES   8
#define VIRTIO_Q_SIZE   16

#define VIRTIO_CONFIG_S_ACKNOWLEDGE 1
#define VIRTIO_CONFIG_S_DRIVER      2
#define VIRTIO_CONFIG_S_DRIVER_OK   4
#define VIRTIO_CONFIG_S_FAILED      0x80

//-----------------------------------------------------------------
// Structures
//-----------------------------------------------------------------
typedef struct
{ 
    /* Address (guest-physical). */ 
    uint64_t addr; 
    /* Length. */ 
    uint32_t len; 

/* This marks a buffer as continuing via the next field. */ 
#define VIRTQ_DESC_F_NEXT   1 
/* This marks a buffer as device write-only (otherwise device read-only). */ 
#define VIRTQ_DESC_F_WRITE     2 
/* This means the buffer contains a list of buffer descriptors. */ 
#define VIRTQ_DESC_F_INDIRECT   4 
    /* The flags as indicated above. */ 
    uint16_t flags; 
    /* Next field if flags & NEXT */ 
    uint16_t next; 
} t_virtio_desc;

typedef struct
{ 
    uint32_t            ready;
    uint32_t            num;
    uint16_t            last_avail_idx;
    uint64_t            desc_addr;
    uint64_t            avail_addr;
    uint64_t            used_addr;
    uint32_t            notify;

    // Desc 
    t_virtio_desc       desc[VIRTIO_Q_SIZE]; 
} t_virtio_q;

class cpu;

//-----------------------------------------------------------------
// virtio_device: Interface class
//-----------------------------------------------------------------
class virtio_device
{
public:
    virtual int  clock(void) { return 0; }
};

//-----------------------------------------------------------------
// virtio: MMIO virtio device
//-----------------------------------------------------------------
class virtio: public device
{
public:
    virtio(cpu *pcpu, uint32_t base_addr, device *irq_ctrl, int irq_num): device("virtio", base_addr, 4096, irq_ctrl, irq_num)
    {
        m_mem = pcpu;
        m_device_id = 0;
        m_vendor_id = 0;
        m_features  = 0;

        memset(m_cfg_space, 0, sizeof(m_cfg_space));
        reset();
    }

    void set_device(virtio_device *dev, uint32_t device_id, uint32_t vendor_id, uint32_t features)
    {
        printf("Set device %08x %08x %08x\n", device_id, vendor_id, features);
        m_dev       = dev;
        m_device_id = device_id;
        m_vendor_id = vendor_id;
        m_features  = features;
    }

    void         reset(void);
    int          clock(void);
    virtual int  min_access_size(void) { return 1; }

    virtual bool write32(uint32_t address, uint32_t data);
    virtual bool read32(uint32_t address, uint32_t &data);

    virtual bool write8(uint32_t addr, uint8_t data);
    virtual bool read8(uint32_t addr, uint8_t &data);

    t_virtio_desc get_desc(int q, int idx);
    void          consume_desc(int queue_idx, int desc_idx, int desc_len);
    bool          get_desc_size(int *pread_size, int *pwrite_size, int queue_idx, int desc_idx);

    bool          queue_access(uint8_t *buf, int queue_idx, int desc_idx, int offset, int count, bool to_queue);

    bool          copy_to_queue(int queue_idx, int desc_idx, int offset, uint8_t *buf, int count)
    {
        return queue_access(buf, queue_idx, desc_idx, offset, count, true);
    }
    bool          copy_from_queue(uint8_t *buf, int queue_idx, int desc_idx, int offset, int count)
    {
        return queue_access(buf, queue_idx, desc_idx, offset, count, false);
    }

    uint16_t get_avail_idx(int queue_idx);
    uint16_t get_avail_value(int queue_idx, uint16_t avail_idx);
    uint16_t get_used_idx(int queue_idx);
    void     set_used_idx(int queue_idx, uint16_t value);

public:
    uint32_t m_device_id;
    uint32_t m_vendor_id;
    uint32_t m_features;

    uint32_t m_status;
    uint32_t m_sel_q;
    uint32_t m_sel_feat;
    uint32_t m_int_status;

    t_virtio_q m_queue[VIRTIO_QUEUES];

    uint32_t m_cfg_space[256/4];
    cpu     *m_mem;

    virtio_device * m_dev;
};

#endif