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
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include "console.h"
#include "elf_load.h"
#include "bin_load.h"

#include "platform_armv6m_basic.h"
#include "platform_rv32im_basic.h"
#include "platform_rv64im_basic.h"
#include "platform_device_tree.h"

static volatile bool m_user_abort = false;

//-----------------------------------------------------------------
// sigint_handler: Abort execution
//-----------------------------------------------------------------
static void sigint_handler(int s)
{
    if (m_user_abort)
    {
        printf("Aborted\n");
        exit(0);
    }
    m_user_abort = true;
}
//-----------------------------------------------------------------
// main
//-----------------------------------------------------------------
int main(int argc, char *argv[])
{
    uint64_t       cycles         = 0;
    int64_t        max_cycles     = (int64_t)-1;
    char *         filename       = NULL;
    int            help           = 0;
    int            trace          = 0;
    uint32_t       trace_mask     = 1;
    uint32_t       stop_pc        = 0xFFFFFFFF;
    uint32_t       trace_pc       = 0xFFFFFFFF;
    uint32_t       mem_base       = 0x00000000;
    uint32_t       mem_size       = (32 * 1024 * 1024);
    bool           explicit_mem   = false;
    const char *   device_blob    = NULL;
    const char *   platform_name  = NULL;
    int c;

    while ((c = getopt (argc, argv, "t:v:f:c:r:b:s:e:D:P:")) != -1)
    {
        switch(c)
        {
            case 't':
                trace = strtoul(optarg, NULL, 0);
                break;
            case 'v':
                trace_mask = strtoul(optarg, NULL, 0);
                break;
            case 'r':
                stop_pc = strtoul(optarg, NULL, 0);
                break;
            case 'f':
                filename = optarg;
                break;
            case 'D':
                device_blob = optarg;
                break;
            case 'c':
                max_cycles = (int64_t)strtoull(optarg, NULL, 0);
                break;
            case 'b':
                mem_base = strtoul(optarg, NULL, 0);
                explicit_mem = true;
                break;
            case 's':
                mem_size = strtoul(optarg, NULL, 0);
                explicit_mem = true;
                break;
            case 'e':
                trace_pc = strtoul(optarg, NULL, 0);
                break;
            case 'P':
                platform_name = optarg;
                break;
            case '?':
            default:
                help = 1;   
                break;
        }
    }

    if (help || (filename == NULL))
    {
        fprintf (stderr,"Usage:\n");
        fprintf (stderr,"-f filename.bin/elf = Executable to load\n");
        fprintf (stderr,"-P platform         = (Optional) Platform to simulate (rv32im-basic|rv64im-basic|armv6m-basic)\n");
        fprintf (stderr,"-D device.dtb       = (Optional) Device tree blob (binary)\n");
        fprintf (stderr,"-t                  = (Optional) Enable program trace\n");
        fprintf (stderr,"-v 0xX              = (Optional) Trace Mask\n");
        fprintf (stderr,"-c nnnn             = (Optional) Max instructions to execute\n");
        fprintf (stderr,"-r 0xnnnn           = (Optional) Stop at PC address\n");
        fprintf (stderr,"-e 0xnnnn           = (Optional) Trace from PC address\n");
        fprintf (stderr,"-b 0xnnnn           = (Optional) Memory base address (for binary loads)\n");
        fprintf (stderr,"-s nnnn             = (Optional) Memory size (for binary loads)\n");
        exit(-1);
    }

    console_io *con = new console();

    if (!platform_name)
        platform_name = "rv32im-basic"; 

    // Resolve platform
    platform * plat = NULL;

    // Device tree blob
    if (device_blob)
        plat = new platform_device_tree(device_blob, con);
    else if (!strcmp(platform_name, "armv6m-basic"))
    {
        if (!explicit_mem)
        {
            mem_base = 0x20000000;
            mem_size = 32 << 20;
        }

        plat = new platform_armv6m_basic(mem_base, mem_size, con);
    }
    else if (!strcmp(platform_name, "rv32im-basic"))
        plat = new platform_rv32im_basic(0, 0, con);
    else if (!strcmp(platform_name, "rv64im-basic"))
        plat = new platform_rv64im_basic(0, 0, con);
    else
    {
        fprintf (stderr,"Error: Unsupported platform\n");
        fprintf (stderr,"Supported: armv6m-basic, rv32im-basic, rv64im-basic\n");
        exit(-1);
    }

    cpu *sim = plat->get_cpu();
    if (!sim)
        return -1;
    sim->set_console(con);

    if (explicit_mem)
    {
        printf("MEM: Create memory 0x%08x-%08x\n", mem_base, mem_base + mem_size-1);
        sim->create_memory(mem_base, mem_size);
    }

    uint32_t start_addr = 0;

    char *ext   = filename ? strrchr(filename, '.') : NULL;
    bool is_bin = ext && !strcmp(ext, ".bin");

    // Binary
    if (is_bin)
    {
        bin_load bin(filename, sim);
        if (!bin.load(mem_base, mem_size))
        {
            fprintf (stderr,"Error: Could not open %s\n", filename);
            return -1;
        }
        start_addr = mem_base;
    }
    // ELF
    else
    {
        elf_load elf(filename, sim);
        if (!elf.load())
        {
            fprintf (stderr,"Error: Could not open %s\n", filename);
            return -1;
        }

        // Find boot vectors if ELF file
        if (!elf.get_symbol("vectors", start_addr))
            start_addr = elf.get_entry_point();
    }

    // Reset CPU to given start PC
    printf("Starting from 0x%08x\n", start_addr);
    sim->reset(start_addr);

    // Enable trace?
    if (trace)
        sim->enable_trace(trace_mask);

    cycles = 0;

    // Catch SIGINT to restore terminal settings on exit
    signal(SIGINT, sigint_handler);

    uint32_t current_pc = 0;
    while (!sim->get_fault() && !sim->get_stopped() && current_pc != stop_pc && !m_user_abort)
    {
        current_pc = sim->get_pc();
        sim->step();
        cycles++;

        if (max_cycles != (int64_t)-1 && max_cycles == cycles)
            break;

        // Turn trace on
        if (trace_pc == current_pc)
            sim->enable_trace(trace_mask);
    }
    // Fault occurred?
    if (sim->get_fault())
        return 1;
    else
    {
        sim->stats_dump();
        return 0;
    }
}
