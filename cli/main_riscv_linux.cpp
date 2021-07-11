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
#include <getopt.h>

#include "console.h"
#include "elf_load.h"
#include "bin_load.h"

#include "platform_device_tree.h"
#include "sbi.h"

#include "virtio_block.h"
#include "virtio_net.h"

static volatile bool m_user_abort = false;

//-----------------------------------------------------------------
// Command line options
//-----------------------------------------------------------------
#define GETOPTS_ARGS "t:v:r:f:D:m:c:e:V:T:i:h"

static struct option long_options[] =
{
    {"trace",      required_argument, 0, 't'},
    {"trace-mask", required_argument, 0, 'v'},
    {"stop-pc",    required_argument, 0, 'r'},
    {"elf",        required_argument, 0, 'f'},
    {"dtb",        required_argument, 0, 'D'},
    {"march",      required_argument, 0, 'm'},
    {"cycles",     required_argument, 0, 'c'},
    {"trace-pc",   required_argument, 0, 'e'},
    {"vda",        required_argument, 0, 'V'},
    {"tap",        required_argument, 0, 'T'},
    {"initrd",     required_argument, 0, 'i'},
    {"help",       no_argument,       0, 'h'},
    {0, 0, 0, 0}
};

static void help_options(void)
{
    fprintf (stderr,"Usage:\n");
    fprintf (stderr,"  --elf        | -f FILE       File to load (ELF or BIN)\n");
    fprintf (stderr,"  --march      | -m MISA       Machine variant (e.g. RV32IMAC, RV64I, ...)\n");
    fprintf (stderr,"  --platform   | -P PLATFORM   Platform to simulate (basic|virt)\n");
    fprintf (stderr,"  --dtb        | -D FILE       Device tree blob (binary)\n");
    fprintf (stderr,"  --trace      | -t 1/0        Enable instruction trace\n");
    fprintf (stderr,"  --trace-mask | -v 0xXX       Trace mask (verbosity level)\n");
    fprintf (stderr,"  --cycles     | -c NUM        Max instructions to execute\n");
    fprintf (stderr,"  --stop-pc    | -r PC         Stop at PC address\n");
    fprintf (stderr,"  --trace-pc   | -e PC         Trace from PC address\n");
    fprintf (stderr,"  --vda        | -V FILE       Disk image for VirtIO block device (/dev/vda)\n");
    fprintf (stderr,"  --tap        | -T TAP        Tap device for VirtIO net device\n");
    fprintf (stderr,"  --initrd     | -i FILE       initrd binary (optional)\n");
    exit(-1);
}
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
    const char *   filename       = NULL;
    const char *   march          = NULL;
    int            help           = 0;
    int            trace          = 0;
    uint32_t       trace_mask     = 1;
    uint32_t       stop_pc        = 0xFFFFFFFF;
    uint32_t       trace_pc       = 0xFFFFFFFF;
    const char *   device_blob    = NULL;
    const char *   platform_name  = NULL;
    const char *   vda_file       = NULL;
    const char *   tap_device     = NULL;
    const char *   initrd_filename= NULL;
    int c;

    int option_index = 0;
    while ((c = getopt_long (argc, argv, GETOPTS_ARGS, long_options, &option_index)) != -1)
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
            case 'm':
                march = optarg;
                break;
            case 'D':
                device_blob = optarg;
                break;
            case 'c':
                max_cycles = (int64_t)strtoull(optarg, NULL, 0);
                break;
            case 'e':
                trace_pc = strtoul(optarg, NULL, 0);
                break;
            case 'V':
                vda_file = optarg;
                break;
            case 'T':
                tap_device = optarg;
                break;
            case 'i':
                initrd_filename = optarg;
                break;
            case '?':
            default:
                help = 1;   
                break;
        }
    }

    if (help || (filename == NULL || device_blob == NULL))
        help_options();

    console_io *con = new console();

    if (!march)
        march = "RV32IMAC";

    // Device tree blob specified SoC
    platform_device_tree * plat = new platform_device_tree(march, device_blob, con);
    cpu *sim = plat->get_cpu();
    if (!sim)
        return -1;
    sim->set_console(con);

    // Get memory
    uint32_t mem_base = plat->get_mem_base();
    uint32_t mem_size = plat->get_mem_size();

    // Create some extra space for the DTB
    uint32_t dtb_base = (mem_base + mem_size + 4096) & ~(4096-1);
    uint32_t dtb_size = (64 * 1024);

    printf("MEM: Create memory 0x%08x-%08x\n", mem_base, mem_base + mem_size-1);
    if (!sim->create_memory(mem_base, mem_size))
    {
        fprintf (stderr,"Error: Could not create memory\n");
        return -1;
    }

    printf("MEM: Create memory 0x%08x-%08x [DTB]\n", dtb_base, dtb_base + dtb_size-1);
    if (!sim->create_memory(dtb_base, dtb_size))
    {
        fprintf (stderr,"Error: Could not create memory\n");
        return -1;
    }

    // Load kernel
    const char *ext   = filename ? strrchr(filename, '.') : NULL;
    bool is_bin = ext && !strcmp(ext, ".bin");

    // Binary
    if (is_bin)
    {
        bin_load bin(filename, sim);
        if (!bin.load(mem_base))
        {
            fprintf (stderr,"Error: Could not open %s\n", filename);
            return -1;
        }
    }
    // ELF
    else
    {
        int64_t load_offset;

        // RV64
        if (sim->get_reg_width() == 64)
            load_offset = 0xffffffe000000000 - mem_base;
        // RV32
        else
            load_offset = 0xC0000000 - mem_base;

        elf_load elf(filename, sim, false, -load_offset);
        if (!elf.load())
        {
            fprintf (stderr,"Error: Could not open %s\n", filename);
            return -1;
        }
    }

    // Optional initrd
    if (initrd_filename)
    {
        uint32_t initrd_base = plat->get_initrd_base();
        uint32_t initrd_size = plat->get_initrd_size();

        if (initrd_base == 0 || initrd_size == 0)
        {
            fprintf (stderr,"Error: linux,initrd-start / linux,initrd-end not specified\n");
            return -1;
        }
        else
        {
            printf("Loading initrd to 0x%08x-0x%08x\n", initrd_base, initrd_base + initrd_size);
            bin_load bin(initrd_filename, sim);
            if (!bin.load(initrd_base))
            {
                fprintf (stderr,"Error: Could not open %s\n", initrd_filename);
                return -1;
            }
        }
    }

    // Load device tree blob
    bin_load bin_dtb(device_blob, sim);
    if (!bin_dtb.load(dtb_base))
    {
        fprintf (stderr,"Error: Could not open %s\n", device_blob);
        return -1;
    }

    // Setup SBI
    sim->reset(mem_base);
    sbi::setup(sim, con, mem_base, dtb_base);

    // User specified virtio block device file
    int vda_idx = 0;
    if (vda_file)
    {
        virtio * vda_dev = (virtio *)sim->find_device("virtio", vda_idx++);
        if (vda_dev)
        {
            virtio_block *vda_blk_dev = new virtio_block(vda_dev);
            if (!vda_blk_dev->open(vda_file))
            {
                fprintf (stderr,"Error: Could not open %s\n", vda_file);
                return -1;
            }
        }
    }

    // User specified tap device for virtio networking
#ifdef INCLUDE_NET_DEVICE
    if (tap_device)
    {
        virtio * vda_dev = (virtio *)sim->find_device("virtio", vda_idx++);
        if (vda_dev)
        {
            virtio_net *vda_net_dev = new virtio_net(vda_dev);
            if (!vda_net_dev->open(tap_device, NULL))
            {
                fprintf (stderr,"Error: Could not open %s\n", tap_device);
                return -1;
            }
        }
    }
#endif

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
