//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <string>
#include <string.h>

//--------------------------------------------------------------------
// Base interface for memories / devices
//--------------------------------------------------------------------
class memory_base
{
public:
    memory_base(std::string name, uint32_t base, uint32_t size)
    {
        m_base      = base;
        m_size      = size;
        m_name      = name;
        m_trace     = false;
        next        = NULL;        
    }

    std::string get_name(void)     { return m_name; }
    void enable_trace(bool en)     { m_trace = en; }

    // Reset / Init
    virtual void reset(void) { }

    // Address range check
    virtual bool valid_addr(uint32_t addr) { return (addr >= m_base) && (addr < (m_base + m_size)); }

    // Write Access
    virtual bool write8(uint32_t addr, uint8_t data) = 0;
    virtual bool write16(uint32_t addr, uint32_t data)
    {
        bool res = true;
        if (m_trace)
            printf("%s: write16 0x%08x=0x%08x\n", m_name.c_str(), addr, data);
        for (int i=0;i<2;i++)
            res &= write8(addr + i, data >> (8*i));
        return res;
    }
    virtual bool write32(uint32_t addr, uint32_t data)
    {
        bool res = true;
        if (m_trace)
            printf("%s: write32 0x%08x=0x%08x\n", m_name.c_str(), addr, data);
        for (int i=0;i<4;i++)
            res &= write8(addr + i, data >> (8*i));
        return res;
    }
    virtual bool write_block(uint32_t addr,  uint8_t *data, int length)
    {
        bool res = true;
        if (m_trace)
            printf("%s: write 0x%08x length %d\n", m_name.c_str(), addr, length);
        for (int i=0;i<length;i++)
            res &= write8(addr + i, *data++);
        return res;
    }

    // Read Access
    virtual bool read8(uint32_t addr, uint8_t &data) = 0;
    virtual bool read16(uint32_t addr, uint16_t &data)
    {
        bool res = true;
        data = 0;
        for (int i=0;i<2;i++)
        {
            uint8_t b = 0;
            res  &= read8(addr + i, b);
            data |= ((uint32_t)b) << (i * 8);
        }
        if (m_trace)
            printf("%s: read16 0x%08x=0x%08x\n", m_name.c_str(), addr, data);        
        return res;
    }
    virtual bool read32(uint32_t addr, uint32_t &data)
    {
        bool res = true;
        data = 0;
        for (int i=0;i<4;i++)
        {
            uint8_t b = 0;
            res  &= read8(addr + i, b);
            data |= ((uint32_t)b) << (i * 8);
        }
        if (m_trace)
            printf("%s: read32 0x%08x=0x%08x\n", m_name.c_str(), addr, data);        
        return res;
    }
    virtual bool read_block(uint32_t addr,  uint8_t *data, int length)
    {
        bool res = true;
        if (m_trace)
            printf("%s: read 0x%08x length %d\n", m_name.c_str(), addr, length);
        for (int i=0;i<length;i++)
            res &= read8(addr + i, data[i]);
        return res;
    }

    // Min access width
    virtual int min_access_size(void) { return 1; }

    // Clock: Clock peripheral (returns next call cycle delta)
    virtual int clock(void) { return 0; }

public:
    memory_base *next;

protected:
    uint32_t    m_base;
    uint32_t    m_size;
    std::string m_name;
    bool        m_trace;
};

//-----------------------------------------------------------------
// Basic memory
//-----------------------------------------------------------------
class memory: public memory_base
{
public:
    memory(std::string name, uint32_t base, uint32_t size, uint8_t * buf = NULL): memory_base(name, base, size)
    {
        if (buf)
            m_mem = buf;
        else
            m_mem  = new uint8_t[size];
    }

    virtual void reset(void)
    {
        memset(m_mem, 0, m_size);
    }

    bool write8(uint32_t addr, uint8_t data)
    {
        if (valid_addr(addr))
        {
            m_mem[addr - m_base] = data;
            return true;
        }
        return false;
    }
    bool read8(uint32_t addr, uint8_t &data)
    {
        if (valid_addr(addr))
        {
            data = m_mem[addr - m_base];
            return true;
        }
        return false;
    }

protected:
    uint8_t  *m_mem;
};

#endif
