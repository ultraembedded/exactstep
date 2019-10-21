//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef BUS_AXI4_LITE_H
#define BUS_AXI4_LITE_H

#include "axi4_lite.h"
#include "memory.h"

//-------------------------------------------------------------
// bus_axi4_lite: AXI4 lite driver interface
//-------------------------------------------------------------
class bus_axi4_lite: public sc_module, public device
{
public:
    //-------------------------------------------------------------
    // Interface I/O
    //-------------------------------------------------------------
    sc_in  <bool>             clk_in;
    sc_in  <bool>             rst_in;

    sc_out <axi4_lite_master> axi_out;
    sc_in  <axi4_lite_slave>  axi_in;

    sc_in  <bool>             intr_in;

    //-------------------------------------------------------------
    // Constructor
    //-------------------------------------------------------------
    SC_HAS_PROCESS(bus_axi4_lite);
    bus_axi4_lite(sc_module_name name, uint32_t base, uint32_t size, device *irq_ctrl, int irq_num);

    //-------------------------------------------------------------
    // Trace
    //-------------------------------------------------------------
    void add_trace(sc_trace_file *vcd, std::string prefix)
    {
        #undef  TRACE_SIGNAL
        #define TRACE_SIGNAL(s) sc_trace(vcd,s,prefix + #s)

        TRACE_SIGNAL(axi_out);
        TRACE_SIGNAL(axi_in);

        #undef  TRACE_SIGNAL
    }

    bool write32(uint32_t address, uint32_t data);
    bool read32(uint32_t address, uint32_t &data);
    int  clock(void)
    {
        return 0;
    }

    void monitor_irq(void);

protected:
    //-------------------------------------------------------------
    // Members
    //-------------------------------------------------------------
};

#endif