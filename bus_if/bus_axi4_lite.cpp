//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#include "bus_axi4_lite.h"

//-----------------------------------------------------------------
// Construction
//-----------------------------------------------------------------
bus_axi4_lite::bus_axi4_lite(sc_module_name name, uint32_t base, uint32_t size, device *irq_ctrl, int irq_num): 
               sc_module(name),
               device("bus_axi4", base, size, irq_ctrl, irq_num)
{
    SC_CTHREAD(monitor_irq, clk_in.pos());
}
//-----------------------------------------------------------------
// monitor_irq: Monitor IRQ line and translate to func call
//-----------------------------------------------------------------
void bus_axi4_lite::monitor_irq(void)
{
    while (true)
    {
        if (rst_in.read())
            ;
        else if (intr_in.read())
            raise_interrupt();

        wait();
    }
}
//-----------------------------------------------------------------
// write32
//-----------------------------------------------------------------
bool bus_axi4_lite::write32(uint32_t address, uint32_t data)
{
    axi4_lite_master axi_o;
    axi4_lite_slave  axi_i;

    axi_o.AWVALID = true;
    axi_o.AWADDR  = address;
    axi_o.WVALID  = true;
    axi_o.WDATA   = data;
    axi_o.WSTRB   = 0xF;
    axi_o.BREADY  = true;

    axi_out.write(axi_o);

    // Wait for accept
    do
    {
        wait();
        axi_i = axi_in.read();

        if (axi_i.AWREADY)
        {
            axi_o.AWVALID = false;
            axi_o.AWADDR  = 0;
        }

        if (axi_i.WREADY)
        {
            axi_o.WVALID  = false;
            axi_o.WDATA   = 0;
            axi_o.WSTRB   = 0;
        }

        axi_out.write(axi_o);
    }
    while (axi_o.AWVALID || axi_o.WVALID);

    axi_o.BREADY  = true;
    axi_out.write(axi_o);

    while (!axi_i.BVALID)
    {
        wait();
        axi_i = axi_in.read();
    }

    return axi_i.BRESP == 0;
}
//-----------------------------------------------------------------
// read32
//-----------------------------------------------------------------
bool bus_axi4_lite::read32(uint32_t address, uint32_t &data)
{
    axi4_lite_master axi_o;
    axi4_lite_slave  axi_i;

    axi_o.ARVALID = true;
    axi_o.ARADDR  = address;
    axi_o.RREADY  = true;

    axi_out.write(axi_o);

    // Wait for accept
    do
    {
        wait();
        axi_i = axi_in.read();
    }
    while (!axi_i.ARREADY);

    axi_o.init();
    axi_o.RREADY  = true;
    axi_out.write(axi_o);

    while (!axi_i.RVALID)
    {
        wait();
        axi_i = axi_in.read();
    }
    data = axi_i.RDATA;
    return axi_i.RRESP == 0;
}
