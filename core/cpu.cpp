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
#include <stdarg.h>
#include <assert.h>
#include "cpu.h"

//-----------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------
cpu::cpu()
{
    m_memories           = NULL;
    m_devices            = NULL;
    m_console            = NULL;
    m_has_breakpoints    = false;
    m_stopped            = false;
    m_fault              = false;
    m_break              = false;
    m_trace              = 0;
    m_syscall_if         = NULL;
}
//-----------------------------------------------------------------
// error: Handle an error
//-----------------------------------------------------------------
bool cpu::error(bool is_fatal, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    if (is_fatal)
        exit(-1);

    return true;
}
//-----------------------------------------------------------------
// create_memory: Create a memory region
//-----------------------------------------------------------------
bool cpu::create_memory(uint32_t baseAddr, uint32_t len, uint8_t *buf /*=NULL*/)
{
    // Avoid adding duplicate memories
    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(baseAddr) && mem->valid_addr(baseAddr + len -1))
            return true;

    return attach_memory(new memory("mem", baseAddr, len, buf));
}
//-----------------------------------------------------------------
// attach_memory: Attach a memory device to a particular region
//-----------------------------------------------------------------
bool cpu::attach_memory(memory_base *memory)
{
    assert(memory->next == NULL);

    memory->next = m_memories;
    m_memories = memory;
    memory->reset();

    return true;
}
//-----------------------------------------------------------------
// attach_memory: Attach a memory device to a particular region
//-----------------------------------------------------------------
bool cpu::attach_device(device *dev)
{
    // Devices are memory mapped
    attach_memory(dev);

    assert(dev->device_next == NULL);

    dev->device_next = m_devices;
    m_devices = dev;
    dev->reset();

    return true;
}
//-----------------------------------------------------------------
// get_break: Get breakpoint status (and clear)
//-----------------------------------------------------------------
bool cpu::get_break(void)
{
    bool brk = m_break;
    m_break = false;
    return brk;
}
//-----------------------------------------------------------------
// set_breakpoint: Set breakpoint on a given PC
//-----------------------------------------------------------------
bool cpu::set_breakpoint(uint32_t pc)
{
    m_breakpoints.push_back(pc);
    m_has_breakpoints = true;
    return true;
}
//-----------------------------------------------------------------
// set_breakpoint: Clear breakpoint on a given PC
//-----------------------------------------------------------------
bool cpu::clr_breakpoint(uint32_t pc)
{
    for (std::vector<uint32_t>::iterator it = m_breakpoints.begin() ; it != m_breakpoints.end(); ++it)
        if ((*it) == pc)
        {
            m_breakpoints.erase(it);
            m_has_breakpoints = !m_breakpoints.empty();
            return true;
        }

    return false;
}
//-----------------------------------------------------------------
// check_breakpoint: Check if breakpoint has been hit
//-----------------------------------------------------------------
bool cpu::check_breakpoint(uint32_t pc)
{
    for (std::vector<uint32_t>::iterator it = m_breakpoints.begin() ; it != m_breakpoints.end(); ++it)
        if ((*it) == pc)
            return true;

    return false;
}
//-----------------------------------------------------------------
// valid_addr: Check if the physical memory address is valid
//-----------------------------------------------------------------
bool cpu::valid_addr(uint32_t address)
{
    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(address))
            return true;

    return false;
}
//-----------------------------------------------------------------
// write: Write a byte to memory (physical address)
//-----------------------------------------------------------------
void cpu::write(uint32_t address, uint8_t data)
{
    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(address))
        {
            mem->write8(address, data);
            return ;
        }

    error(false, "Failed store @ 0x%08x\n", address);
}
//-----------------------------------------------------------------
// read: Read a byte from memory (physical address)
//-----------------------------------------------------------------
uint8_t cpu::read(uint32_t address)
{
    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(address))
        {
            uint8_t data = 0;
            mem->read8(address, data);
            return data;
        }

    return 0;
}
//-----------------------------------------------------------------
// write16: Write a word to memory (physical address)
//-----------------------------------------------------------------
void cpu::write16(uint32_t address, uint16_t data)
{
    address &= ~1;

    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(address))
        {
            mem->write16(address, data);
            return ;
        }

    error(false, "Failed store @ 0x%08x\n", address);
}
//-----------------------------------------------------------------
// read16: Read a word from memory (physical address)
//-----------------------------------------------------------------
uint16_t cpu::read16(uint32_t address)
{
    address &= ~1;

    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(address))
        {
            uint16_t data = 0;
            mem->read16(address, data);
            return data;
        }

    return 0;
}
//-----------------------------------------------------------------
// write32: Write a word to memory (physical address)
//-----------------------------------------------------------------
void cpu::write32(uint32_t address, uint32_t data)
{
    address &= ~3;

    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(address))
        {
            mem->write32(address, data);
            return ;
        }

    error(false, "Failed store @ 0x%08x\n", address);
}
//-----------------------------------------------------------------
// read32: Read a word from memory (physical address)
//-----------------------------------------------------------------
uint32_t cpu::read32(uint32_t address)
{
    address &= ~3;

    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(address))
        {
            uint32_t data = 0;
            mem->read32(address, data);
            return data;
        }

    return 0;
}
//-----------------------------------------------------------------
// ifetch32: Read a instruction from memory (physical address)
//-----------------------------------------------------------------
uint32_t cpu::ifetch32(uint32_t address)
{
    address &= ~3;

    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(address))
        {
            uint32_t data = 0;
            mem->ifetch32(address, data);
            return data;
        }

    return 0;
}
//-----------------------------------------------------------------
// ifetch16: Read a instruction from memory (physical address)
//-----------------------------------------------------------------
uint16_t cpu::ifetch16(uint32_t address)
{
    address &= ~1;

    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(address))
        {
            uint16_t data = 0;
            mem->ifetch16(address, data);
            return data;
        }

    return 0;
}
//-----------------------------------------------------------------
// step: Step through one instruction
//-----------------------------------------------------------------
void cpu::step(void)
{
    // Breakpoint hit?
    if (m_has_breakpoints && check_breakpoint(get_pc()))
        m_break = true;

    // Clock peripherals
    for (device *dev = m_devices; dev != NULL; dev = dev->device_next)
    {
        dev->clock();

        if (dev->event_irq_raised())
            set_interrupt(dev->get_irq_num());
        else if (dev->event_irq_dropped())
            clr_interrupt(dev->get_irq_num());
    }
}
//-----------------------------------------------------------------
// find_device: Find device by name and index
//-----------------------------------------------------------------
device * cpu::find_device(std::string name, int idx)
{
    int count = 0;
    for (device *d = m_devices; d; d = d->device_next)
    {
        if (d->get_name() == name)
        {
            if (idx == count)
                return d;
            count++;
        }
    }

    return NULL;
}
