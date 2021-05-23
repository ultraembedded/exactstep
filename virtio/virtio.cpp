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

#include "virtio.h"
#include "cpu.h"

//--------------------------------------------------------------------
// Defines:
//--------------------------------------------------------------------
#define VIRTIO_MMIO_MAGIC_VALUE         0x000
#define VIRTIO_MMIO_VERSION             0x004
#define VIRTIO_MMIO_DEVICE_ID           0x008
#define VIRTIO_MMIO_VENDOR_ID           0x00c
#define VIRTIO_MMIO_DEVICE_FEATURES     0x010
#define VIRTIO_MMIO_DEVICE_FEATURES_SEL 0x014
#define VIRTIO_MMIO_DRIVER_FEATURES     0x020
#define VIRTIO_MMIO_DRIVER_FEATURES_SEL 0x024
#define VIRTIO_MMIO_QUEUE_SEL           0x030
#define VIRTIO_MMIO_QUEUE_NUM_MAX       0x034
#define VIRTIO_MMIO_QUEUE_NUM           0x038
#define VIRTIO_MMIO_QUEUE_READY         0x044
#define VIRTIO_MMIO_QUEUE_NOTIFY        0x050
#define VIRTIO_MMIO_INTERRUPT_STATUS    0x060
#define VIRTIO_MMIO_INTERRUPT_ACK       0x064
#define VIRTIO_MMIO_STATUS              0x070
#define VIRTIO_MMIO_QUEUE_DESC_LOW      0x080
#define VIRTIO_MMIO_QUEUE_DESC_HIGH     0x084
#define VIRTIO_MMIO_QUEUE_AVAIL_LOW     0x090
#define VIRTIO_MMIO_QUEUE_AVAIL_HIGH    0x094
#define VIRTIO_MMIO_QUEUE_USED_LOW      0x0a0
#define VIRTIO_MMIO_QUEUE_USED_HIGH     0x0a4
#define VIRTIO_MMIO_CONFIG_GENERATION   0x0fc
#define VIRTIO_MMIO_CONFIG              0x100

#define dprintf(a) // printf a

//--------------------------------------------------------------------
// reset:
//--------------------------------------------------------------------
void virtio::reset(void)
{
    m_status     = 0;
    m_sel_q      = 0;
    m_sel_feat   = 0;
    m_int_status = 0;

    for (int i=0;i<VIRTIO_QUEUES;i++)
    {
        m_queue[i].notify          = 0;
        m_queue[i].ready           = 0;
        m_queue[i].num             = 0;
        m_queue[i].last_avail_idx  = 0;
        m_queue[i].desc_addr       = 0;
        m_queue[i].avail_addr      = 0;
        m_queue[i].used_addr       = 0;
    }

    assert(sizeof(t_virtio_desc) == 16);
}
//--------------------------------------------------------------------
// write8:
//--------------------------------------------------------------------
bool virtio::write8(uint32_t address, uint8_t data)
{
    return false;
}
//--------------------------------------------------------------------
// read8:
//--------------------------------------------------------------------
bool virtio::read8(uint32_t address, uint8_t &data)
{
    address -= m_base;
    if (address >= VIRTIO_MMIO_CONFIG)
    {
        address -= VIRTIO_MMIO_CONFIG;

        uint8_t *p = (uint8_t*)&m_cfg_space[0];
        data = p[address];
        return true;
    }
    return false;
}
//--------------------------------------------------------------------
// write32:
//--------------------------------------------------------------------
bool virtio::write32(uint32_t address, uint32_t data)
{
    address -= m_base;

    if (address >= VIRTIO_MMIO_CONFIG)
    {
        dprintf(("[VIRTIO] Config write %08x=%08x\n", address, data));
        if ((address - VIRTIO_MMIO_CONFIG) < 256)
            m_cfg_space[(address - VIRTIO_MMIO_CONFIG) / 4] = data;
    }
    else switch (address)
    {
    case VIRTIO_MMIO_STATUS:
        m_status = data;
        dprintf(("[VIRTIO] Set status %08x\n", data));

        // Reset
        if (data == 0)
        {
            m_irq_raised = false;
            reset();
        }
        break;
    case VIRTIO_MMIO_INTERRUPT_ACK:
        m_int_status &= ~data;
        if (m_int_status ==0)
            m_irq_raised = false;
        // TODO: Clear IRQ on m_int_status == 0
        break;
    case VIRTIO_MMIO_DEVICE_FEATURES_SEL:
        dprintf(("[VIRTIO] Select feature %d\n", data));
        m_sel_feat = data;
        break;
    case VIRTIO_MMIO_QUEUE_SEL:
        dprintf(("[VIRTIO] Select queue %d\n", data));
        m_sel_q = data;
        break;
    case VIRTIO_MMIO_QUEUE_NUM:
        assert((data & (data - 1)) == 0); // Must be a power of 2
        assert(data != 0);
        dprintf(("[VIRTIO] Select queue %d\n", m_sel_q));
        m_queue[m_sel_q].num = data;
        break;
    case VIRTIO_MMIO_QUEUE_DESC_LOW:
        dprintf(("[VIRTIO] Queue %d desc_addr %08x\n", m_sel_q, data));
        m_queue[m_sel_q].desc_addr &= ~0xFFFFFFFFULL;
        m_queue[m_sel_q].desc_addr |= data;
        break;
    case VIRTIO_MMIO_QUEUE_AVAIL_LOW:
        dprintf(("[VIRTIO] Queue %d avail_addr %08x\n", m_sel_q, data));
        m_queue[m_sel_q].avail_addr &= ~0xFFFFFFFFULL;
        m_queue[m_sel_q].avail_addr |= data;
        break;
    case VIRTIO_MMIO_QUEUE_USED_LOW:
        dprintf(("[VIRTIO] Queue %d used_addr %08x\n", m_sel_q, data));
        m_queue[m_sel_q].used_addr &= ~0xFFFFFFFFULL;
        m_queue[m_sel_q].used_addr |= data;
        break;
    case VIRTIO_MMIO_QUEUE_DESC_HIGH:
        m_queue[m_sel_q].desc_addr &= ~0xFFFFFFFF00000000ULL;
        m_queue[m_sel_q].desc_addr |= ((uint64_t)data) << 32;
        break;
    case VIRTIO_MMIO_QUEUE_AVAIL_HIGH:
        m_queue[m_sel_q].avail_addr &= ~0xFFFFFFFF00000000ULL;
        m_queue[m_sel_q].avail_addr |= ((uint64_t)data) << 32;
        break;
    case VIRTIO_MMIO_QUEUE_USED_HIGH:
        m_queue[m_sel_q].used_addr &= ~0xFFFFFFFF00000000ULL;
        m_queue[m_sel_q].used_addr |= ((uint64_t)data) << 32;
        break;
    case VIRTIO_MMIO_QUEUE_READY:
        dprintf(("[VIRTIO] Queue %d ready %d\n", m_sel_q, data));
        m_queue[m_sel_q].ready = data & 1;
        break;
    case VIRTIO_MMIO_QUEUE_NOTIFY:
        if (data < VIRTIO_QUEUES)
            m_queue[data].notify++;
        break;
    }

    return false;
}
//--------------------------------------------------------------------
// read32:
//--------------------------------------------------------------------
bool virtio::read32(uint32_t address, uint32_t &data)
{
    address -= m_base;
    data     = 0;

    if (address >= VIRTIO_MMIO_CONFIG)
    {
        dprintf(("[VIRTIO] Config read %08x\n", address));
        if ((address - VIRTIO_MMIO_CONFIG) < 256)
            data = m_cfg_space[(address - VIRTIO_MMIO_CONFIG) / 4];
    }
    else switch (address)
    {
    case VIRTIO_MMIO_MAGIC_VALUE:
        data = 0x74726976;
        break;
    case VIRTIO_MMIO_VERSION:
        data = 2;
        break;
    case VIRTIO_MMIO_DEVICE_ID:
        data = m_device_id;
        break;
    case VIRTIO_MMIO_VENDOR_ID:
        data = m_vendor_id;
        break;
    case VIRTIO_MMIO_STATUS:
        data = m_status;
        break;
    case VIRTIO_MMIO_INTERRUPT_STATUS:
        data = m_int_status;
        break;
    case VIRTIO_MMIO_DEVICE_FEATURES_SEL:
        data = m_sel_feat;
        break;
    case VIRTIO_MMIO_DEVICE_FEATURES:
        switch(m_sel_feat)
        {
        case 0:
            data = m_features;
            break;
        case 1:
            data = 1; // Version 1
            break;
        }
        break;        
    case VIRTIO_MMIO_QUEUE_SEL:
        data = m_sel_q;
        break;
    case VIRTIO_MMIO_QUEUE_NUM_MAX:
        data = VIRTIO_Q_SIZE;
        break;
    case VIRTIO_MMIO_QUEUE_NUM:
        data = m_queue[m_sel_q].num;
        break;
    case VIRTIO_MMIO_QUEUE_DESC_LOW:
        data = m_queue[m_sel_q].desc_addr;
        break;
    case VIRTIO_MMIO_QUEUE_AVAIL_LOW:
        data = m_queue[m_sel_q].avail_addr;
        break;
    case VIRTIO_MMIO_QUEUE_USED_LOW:
        data = m_queue[m_sel_q].used_addr;
        break;
    case VIRTIO_MMIO_QUEUE_DESC_HIGH:
        data = m_queue[m_sel_q].desc_addr >> 32;
        break;
    case VIRTIO_MMIO_QUEUE_AVAIL_HIGH:
        data = m_queue[m_sel_q].avail_addr >> 32;
        break;
    case VIRTIO_MMIO_QUEUE_USED_HIGH:
        data = m_queue[m_sel_q].used_addr >> 32;
        break;
    case VIRTIO_MMIO_QUEUE_READY:
        data = m_queue[m_sel_q].ready;
        break;
    case VIRTIO_MMIO_CONFIG_GENERATION:
        data = 0;
        break;
    default:
        printf("ERROR: VIRTIO %08x not supported\n", address);
        return false;
        break;
    }

    return true;
}
//--------------------------------------------------------------------
// get_desc: Get descriptor from memory
//--------------------------------------------------------------------
t_virtio_desc virtio::get_desc(int q, int idx)
{
    uint64_t addr = m_queue[q].desc_addr + (idx * sizeof(t_virtio_desc));

    uint8_t buf[16];
    for (int i=0;i<sizeof(buf);i++)
        buf[i] = m_mem->read(addr + i);

    t_virtio_desc *d_ptr = (t_virtio_desc *)buf;
    return *d_ptr;
}
//--------------------------------------------------------------------
// get_avail_idx:
//--------------------------------------------------------------------
uint16_t virtio::get_avail_idx(int queue_idx)
{
    return m_mem->read16(m_queue[queue_idx].avail_addr + 2);
}
//--------------------------------------------------------------------
// get_avail_value:
//--------------------------------------------------------------------
uint16_t virtio::get_avail_value(int queue_idx, uint16_t avail_idx)
{
    return m_mem->read16(m_queue[queue_idx].avail_addr + 4 + (avail_idx & (m_queue[queue_idx].num - 1)) * 2);
}
//--------------------------------------------------------------------
// get_used_idx:
//--------------------------------------------------------------------
uint16_t virtio::get_used_idx(int queue_idx)
{
    return m_mem->read16(m_queue[queue_idx].used_addr + 2);
}
//--------------------------------------------------------------------
// set_used_idx:
//--------------------------------------------------------------------
void virtio::set_used_idx(int queue_idx, uint16_t value)
{
    m_mem->write16(m_queue[queue_idx].used_addr + 2, value);
}
//--------------------------------------------------------------------
// queue_access:
//--------------------------------------------------------------------
bool virtio::queue_access(uint8_t *buf, int queue_idx, int desc_idx, int offset, int count, bool to_queue)
{
    int l, f_write_flag;

    if (count == 0)
        return true;

    t_virtio_desc desc = get_desc(queue_idx, desc_idx);

    if (to_queue)
    {
        f_write_flag = VIRTQ_DESC_F_WRITE;

        // Write descriptors
        while (true)
        {
            if ((desc.flags & VIRTQ_DESC_F_WRITE) == f_write_flag)
                break;
            if (!(desc.flags & VIRTQ_DESC_F_NEXT))
                return false;
            desc_idx = desc.next;
            desc = get_desc(queue_idx, desc_idx);
        }
    }
    else
        f_write_flag = 0;

    // Find the descriptor that matches the data offset
    while (true)
    {
        if ((desc.flags & VIRTQ_DESC_F_WRITE) != f_write_flag)
            return false;
        if (offset < desc.len)
            break;
        if (!(desc.flags & VIRTQ_DESC_F_NEXT))
            return false;
        desc_idx = desc.next;
        offset -= desc.len;
        desc = get_desc(queue_idx, desc_idx);
    }

    while (true)
    {
        if (count < (desc.len - offset))
            l = count;
        else
            l = (desc.len - offset);

        if (to_queue)
        {
            for (int i=0;i<l;i++)
                m_mem->write(desc.addr + offset + i, buf[i]);
        }
        else
        {
            for (int i=0;i<l;i++)
                buf[i] = m_mem->read(desc.addr + offset + i);
        }
        count -= l;
        if (count == 0)
            break;
        offset += l;
        buf += l;
        if (offset == desc.len)
        {
            if (!(desc.flags & VIRTQ_DESC_F_NEXT))
                return false;
            desc_idx = desc.next;
            desc = get_desc(queue_idx, desc_idx);
            if ((desc.flags & VIRTQ_DESC_F_WRITE) != f_write_flag)
                return false;
            offset = 0;
        }
    }

    return true;
}
//--------------------------------------------------------------------
// consume_desc: Write to the used ring to indicate a completion
//--------------------------------------------------------------------
void virtio::consume_desc(int queue_idx, int desc_idx, int desc_len)
{
    uint64_t addr;
    uint32_t index;

    // The Used Ring looks like;

    //struct virtq_used
    //{
    //    le16 flags;
    //    le16 idx;
    //    struct virtq_used_elem ring[QUEUE_SIZE];
    //};
 
    //struct virtq_used_elem
    //{
    //    le32 id;   // Index of start of used descriptor chain
    //    le32 len;  // Total length of the descriptor chain
    //};

    // Get current used pointer
    index = get_used_idx(queue_idx);

    // Write to appropriate virtq_used_elem
    addr = m_queue[queue_idx].used_addr + 4 + (index & (m_queue[queue_idx].num - 1)) * 8;
    m_mem->write32(addr,     desc_idx); // Index of start of used descriptor chain
    m_mem->write32(addr + 4, desc_len); // Total length of the descriptor chain

    // Increment used pointer
    set_used_idx(queue_idx, index + 1);

    // Raise interrupts (TODO: Look at VIRTQ_USED_F_NO_NOTIFY)
    m_int_status |= 1;
    raise_interrupt();
}
//--------------------------------------------------------------------
// get_desc_size: Find the total read write size of the transfer
//--------------------------------------------------------------------
bool virtio::get_desc_size(int *pread_size, int *pwrite_size, int queue_idx, int desc_idx)
{
    t_virtio_desc desc;
    int read_size, write_size;

    read_size = 0;
    write_size = 0;
    desc = get_desc(queue_idx, desc_idx);
    
    // Find total read length
    while (true)
    {
        // Write desc - end of read
        if (desc.flags & VIRTQ_DESC_F_WRITE)
            break;

        read_size += desc.len;

        // End of read list
        if (!(desc.flags & VIRTQ_DESC_F_NEXT))
            goto done;

        // Chained descriptors
        desc_idx = desc.next;
        desc = get_desc(queue_idx, desc_idx);
    }
    
    // Find total write length
    while (true)
    {
        if (!(desc.flags & VIRTQ_DESC_F_WRITE))
        {
            printf("ERROR: Badly formed descriptors\n");
            return false;
        }

        write_size += desc.len;

        // End of write list
        if (!(desc.flags & VIRTQ_DESC_F_NEXT))
            break;

        // Chained descriptors
        desc_idx = desc.next;
        desc = get_desc(queue_idx, desc_idx);
    }

 done:
    *pread_size = read_size;
    *pwrite_size = write_size;
    return true;
}
//--------------------------------------------------------------------
// clock:
//--------------------------------------------------------------------
int virtio::clock(void) 
{ 
    // Not ready
    if (!(m_status & (1 << VIRTIO_CONFIG_S_DRIVER)))
        return 0;

    return m_dev->clock();
}
