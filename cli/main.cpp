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

#include "platform_basic.h"
#include "platform_virt.h"
#include "platform_device_tree.h"

static volatile bool m_user_abort = false;

//-----------------------------------------------------------------
// Command line options
//-----------------------------------------------------------------
#define GETOPTS_ARGS "m:t:v:f:c:r:b:s:e:D:P:p:j:k:h"

static struct option long_options[] =
{
    {"trace",      required_argument, 0, 't'},
    {"trace-mask", required_argument, 0, 'v'},
    {"stop-pc",    required_argument, 0, 'r'},
    {"elf",        required_argument, 0, 'f'},
    {"dtb",        required_argument, 0, 'D'},
    {"march",      required_argument, 0, 'm'},
    {"cycles",     required_argument, 0, 'c'},
    {"mem-base",   required_argument, 0, 'b'},
    {"mem-size",   required_argument, 0, 's'},
    {"trace-pc",   required_argument, 0, 'e'},
    {"platform",   required_argument, 0, 'P'},
    {"dump-file",  required_argument, 0, 'p'},
    {"dump-start", required_argument, 0, 'j'},
    {"dump-end",   required_argument, 0, 'k'},
    {"dump-reg-f", required_argument, 0, 'R'},
    {"dump-reg-s", required_argument, 0, 'S'},
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
    fprintf (stderr,"  --mem-base   | -b VAL        Memory base address (for binary loads)\n");
    fprintf (stderr,"  --mem-size   | -s VAL        Memory size (for binary loads)\n");
    fprintf (stderr,"  --dump-file  | -p FILE       File to dump memory contents to after completion\n");
    fprintf (stderr,"  --dump-start | -j SYM/A      Symbol name for memory dump start (or 0xADDR)\n");
    fprintf (stderr,"  --dump-end   | -k SYM/A      Symbol name for memory dump end (or 0xADDR)\n");
    fprintf (stderr,"  --dump-reg-f | -R FILE       File to dump register file contents to after completion\n");
    fprintf (stderr,"  --dump-reg-s | -S NUM        Number of register file entries to dump\n");
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
// create_dump_file: Create memory dump file
//-----------------------------------------------------------------
static bool create_dump_file(cpu *sim, const char *dump_file, uint32_t dump_start, uint32_t dump_end)
{
    int  dump_size  = dump_end - dump_start;

    // Nothing to do...
    if (dump_size <= 0 || !dump_file)
        return false;

    const char *ext = strrchr(dump_file, '.');
    bool sig_txt_file = ext && !strcmp(ext, ".output");

    printf("Dumping post simulation memory: 0x%08x-0x%08x (%d bytes) [%s]\n", dump_start, dump_end, dump_size, dump_file);

    uint8_t *buffer = new uint8_t[dump_size];
    for (uint32_t i=0;i<dump_size;i++)
        buffer[i] = sim->read(dump_start + i);

    // Binary block
    if (!sig_txt_file)
    {
        // Write file data
        FILE *f = fopen(dump_file, "wb");
        if (f)
        {
            fwrite(buffer, 1, dump_size, f);
            fclose(f);
        }
    }
    // Signature text file
    else
    {
        // Write file data
        FILE *f = fopen(dump_file, "w");
        if (f)
        {
            uint32_t *w = (uint32_t*)buffer;
            for (int r=0;r<(dump_size/4);r++)
                fprintf(f, "%08x\n", *w++);
            fclose(f);
        }
    }

    delete buffer;
    buffer = NULL;
    return true;
}
//-----------------------------------------------------------------
// create_dump_regfile: Dump register file contents
//-----------------------------------------------------------------
static bool create_dump_regfile(cpu *sim, const char *dump_file, uint32_t dump_num)
{
    // Nothing to do...
    if (dump_num == 0 || !dump_file)
        return false;

    const char *ext = strrchr(dump_file, '.');
    bool sig_txt_file = ext && !strcmp(ext, ".txt");

    printf("Dumping post simulation registers: %s\n", dump_file);

    // 64-bit
    if (sim->get_reg_width() == 64)
    {
        uint64_t *buffer = new uint64_t[dump_num];
        for (uint32_t i=0;i<dump_num;i++)
            buffer[i] = sim->get_register(i);

        // Binary block
        if (!sig_txt_file)
        {
            // Write file data
            FILE *f = fopen(dump_file, "wb");
            if (f)
            {
                fwrite(buffer, sizeof(buffer[0]), dump_num, f);
                fclose(f);
            }
        }
        // Text file
        else
        {
            // Write file data
            FILE *f = fopen(dump_file, "w");
            if (f)
            {
                for (uint32_t i=0;i<dump_num;i++)
                    fprintf(f, "%16llx\n", buffer[i]);
                fclose(f);
            }
        }

        delete[] buffer;
        buffer = NULL;
    }
    // 32-bit
    else
    {
        uint32_t *buffer = new uint32_t[dump_num];
        for (uint32_t i=0;i<dump_num;i++)
            buffer[i] = sim->get_register(i);

        // Binary block
        if (!sig_txt_file)
        {
            // Write file data
            FILE *f = fopen(dump_file, "wb");
            if (f)
            {
                fwrite(buffer, sizeof(buffer[0]), dump_num, f);
                fclose(f);
            }
        }
        // Text file
        else
        {
            // Write file data
            FILE *f = fopen(dump_file, "w");
            if (f)
            {
                for (uint32_t i=0;i<dump_num;i++)
                    fprintf(f, "%08x\n", buffer[i]);
                fclose(f);
            }
        }

        delete[] buffer;
        buffer = NULL;
    }

    return true;
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
    char *         stop_pc_sym    = NULL;
    uint32_t       trace_pc       = 0xFFFFFFFF;
    uint32_t       mem_base       = 0x00000000;
    uint32_t       mem_size       = (32 * 1024 * 1024);
    bool           explicit_mem   = false;
    const char *   device_blob    = NULL;
    const char *   platform_name  = NULL;
    char *         dump_file      = NULL;
    char *         dump_sym_start = NULL;
    char *         dump_sym_end   = NULL;
    uint32_t       dump_start     = 0;
    uint32_t       dump_end       = 0;
    char *         dump_reg_file  = NULL;
    uint32_t       dump_reg_num   = 32;
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
                if (!strncmp(optarg, "0x", 2))
                    stop_pc = strtoul(optarg, NULL, 0);
                else
                    stop_pc_sym = optarg;
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
            case 'p':
                dump_file = optarg;
                break;
            case 'j':
                if (!strncmp(optarg, "0x", 2))
                    dump_start = strtoul(optarg, NULL, 0);
                else
                    dump_sym_start = optarg;
                break;
            case 'k':
                if (!strncmp(optarg, "0x", 2))
                    dump_end = strtoul(optarg, NULL, 0);
                else
                    dump_sym_end = optarg;
                break;
            case 'R':
                dump_reg_file = optarg;
                break;
            case 'S':
                dump_reg_num = strtoul(optarg, NULL, 0);
                break;
            case '?':
            default:
                help = 1;   
                break;
        }
    }

    if (help || (filename == NULL))
        help_options();

    console_io *con = new console();

    if (!march)
        march = "RV32IMAC";

    if (!platform_name)
        platform_name = "basic"; 

    // Resolve platform
    platform * plat = NULL;

    // Device tree blob specified SoC
    if (device_blob)
        plat = new platform_device_tree(march, device_blob, con);
    // Basic platform
    else if (!strcmp(platform_name, "basic"))
        plat = new platform_basic(march, 0, 0, con);
    else if (!strcmp(platform_name, "virt"))
        plat = new platform_virt(march, 0x80000000, (64 << 20), con);
    else
    {
        fprintf (stderr,"Error: Unsupported platform\n");
        fprintf (stderr,"Supported: basic, virt\n");
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

    const char *ext   = filename ? strrchr(filename, '.') : NULL;
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

        // Lookup memory dump addresses?
        uint32_t sym_addr;
        if (dump_sym_start && elf.get_symbol(dump_sym_start, sym_addr))
            dump_start = sym_addr;
        if (dump_sym_end && elf.get_symbol(dump_sym_end, sym_addr))
            dump_end = sym_addr;
        if (stop_pc_sym && elf.get_symbol(stop_pc_sym, sym_addr))
            stop_pc = sym_addr;
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
        // Dump memory contents after execution?
        if (dump_file)
            create_dump_file(sim, dump_file, dump_start, dump_end);
        if (dump_reg_file)
            create_dump_regfile(sim, dump_reg_file, dump_reg_num);

        sim->stats_dump();
        return 0;
    }
}
