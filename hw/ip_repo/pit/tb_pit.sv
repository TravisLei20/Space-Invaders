`timescale 1ns / 1ps

import axi_vip_pkg::*;
import my_block_design_axi_vip_0_0_pkg::*;

module tb(
  );
     
  bit                                     clock;
  bit                                     reset_n;
  bit                                     irq;
  
  my_block_design_axi_vip_0_0_mst_t              master_agent;
   
  xil_axi_ulong addrCtrl = 32'h0000_0000; //0x0000_0000
  xil_axi_ulong addrPeriod = 32'h0000_0004; // 0x0000_0FFF
  xil_axi_prot_t prot = 0;
  xil_axi_resp_t resp;
  bit[31:0] data;
  bit[31:0] read_data;
  
  // instantiate block diagram design
  my_block_design design_1_i
       (.aclk_0(clock),
        .aresetn_0(reset_n),
        .irq_0(irq));
  
  always #5ns clock <= ~clock;

  initial begin
    master_agent = new("master vip agent",design_1_i.axi_vip_0.inst.IF);
    
    //Start the agent
    master_agent.start_master();
    
    reset_n = 1'b0;
    #170ns;
    
    reset_n = 1'b1;
    #10ns;
    
    $display("\n");
    
    data = 32'h00000003;
    master_agent.AXI4LITE_WRITE_BURST(addrCtrl, prot, data, resp);
    $display("Write Data to control register");
    $display("Value of write data: %h\n", data);
    #50ns;

    data = 32'h00000004;
    master_agent.AXI4LITE_WRITE_BURST(addrPeriod, prot, data, resp);
    $display("Write Data to delay-value register");
    $display("Value of write data: %h\n", data);
    #50ns;

    master_agent.AXI4LITE_READ_BURST(addrCtrl, prot, read_data, resp);
    $display("Read Data from control register");
    $display("Read data: %h should equal %h\n", read_data, 32'h00000003);
    #50ns;
    
    master_agent.AXI4LITE_READ_BURST(addrPeriod, prot, read_data, resp);
    $display("Read Data from delay-value register");
    $display("Read data: %h should equal %h\n", read_data, 32'h000000004);
    #50ns;
    
    $display("Run simulartion 150ns more to show interrupts work correctly.\n");
    #150ns;

    data = 32'h00000001;
    master_agent.AXI4LITE_WRITE_BURST(addrCtrl, prot, data, resp);
    $display("Write Data to control register");
    $display("Value of write data: %h", data);
    $display("Disable interrupts but enable the clock counter.\n");
    #50ns;

    data = 32'h00000000;
    master_agent.AXI4LITE_WRITE_BURST(addrCtrl, prot, data, resp);
    $display("Write Data to control register");
    $display("Value of write data: %h", data);
    $display("Disable interrupts and clock counter\n");
    #50ns;

    data = 32'h00000003;
    master_agent.AXI4LITE_WRITE_BURST(addrCtrl, prot, data, resp);
    $display("Write Data to control register");
    $display("Value of write data: %h", data);
    $display("Enable interrupts and clock counter, check if counter is continuing where it left off.\n");
    #50ns;
    
    $display("\n");
    $display("Simulation finished");
    $finish; // Terminate the simulation
    $display("\n");
  end

endmodule