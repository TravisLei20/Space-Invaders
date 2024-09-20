`timescale 1 ns / 1 ps
`default_nettype wire

module pit_sv #(
    // Width of S_AXI address bus
    parameter integer C_S_AXI_ADDR_WIDTH = 4
)
(
    input logic                             s_axi_aclk,
    input logic                             s_axi_aresetn,

    // AW - write to 
    input logic                             s_axi_awvalid,
    output logic                            s_axi_awready,
    input logic [C_S_AXI_ADDR_WIDTH-1:0]    s_axi_awaddr,
    input logic [2:0]                       s_axi_awprot, // prot

    // W
    input logic                             s_axi_wvalid,
    output logic                            s_axi_wready,
    input logic [31:0]                      s_axi_wdata,
    input logic [3:0]                       s_axi_wstrb, //strobe

    // B
    output logic                            s_axi_bvalid,
    input logic                             s_axi_bready,
    output logic [1:0]                      s_axi_bresp,
    
    // all read 

    // AR
    input logic                             s_axi_arvalid,
    output logic                            s_axi_arready,
    input logic [C_S_AXI_ADDR_WIDTH-1:0]    s_axi_araddr,
    input logic [2:0]                       s_axi_arprot, // prot

    // R
    output logic [31:0]                     s_axi_rdata,
    output logic [1:0]                      s_axi_rresp,
    output logic                            s_axi_rvalid,
    input logic                             s_axi_rready,

    output logic                            irq
);

// States definition
typedef enum logic [1:0] {
    WAIT,
    WRITE,
    READ,
    RESPONSE
} state_t;

// Internal signals
state_t write_state;
state_t read_state;
bit int_en;
bit counter_en;
bit counter_set;

// Register definitions
logic [31:0] ctrl_reg;
logic [31:0] delay_reg;
logic [31:0] counter;
logic [C_S_AXI_ADDR_WIDTH-1:0] addr;
logic [C_S_AXI_ADDR_WIDTH-1:0] read_addr;

// Response
localparam OKAY = 2'd0;
assign s_axi_rresp = OKAY;
assign s_axi_bresp = OKAY;

always_ff @(posedge s_axi_aclk) begin
    if (!s_axi_aresetn) begin
        s_axi_awready <= 1'b0;
        s_axi_wready <= 1'b0;
        s_axi_bvalid <= 1'b0;
        delay_reg <= 32'hf;
        counter_set <= 1'b0;
        write_state <= WAIT;
    end
    else
        case(write_state)
            WAIT: begin
                s_axi_bvalid <= 1'b0;
                if (s_axi_awvalid) begin
                    s_axi_awready <= 1'b1;
                    addr <= s_axi_awaddr;
                    write_state <= WRITE;
                end
                else begin
                    write_state <= WAIT;
                end
            end

            WRITE: begin
                s_axi_wready <= 1'b1;
                s_axi_awready <= 1'b0;
                if (s_axi_wvalid) begin
                    if (addr == 4'h0) begin
                        ctrl_reg <= s_axi_wdata;
                    end
                    else if (addr == 4'h4) begin
                        delay_reg <= s_axi_wdata;
                        counter_set <= 1'b1;
                    end
                    s_axi_wready <= 1'b0;
                    write_state <= RESPONSE;
                end
                else begin
                    write_state <= WRITE;
                end
            end

            RESPONSE: begin
                s_axi_bvalid <= 1'b1;
                if (s_axi_bready) begin
                    counter_set <= 1'b0;
                    write_state <= WAIT;
                end
                else begin
                    write_state <= RESPONSE;
                end
            end

            default: begin
                write_state <= WAIT;
                delay_reg <= 32'hf;
            end
        endcase
end

always_ff @(posedge s_axi_aclk) begin
    if (!s_axi_aresetn) begin
        s_axi_arready <= 1'b0;
        s_axi_rvalid <= 1'b0;
        read_state <= WAIT;
    end
    else
        case(read_state)
            WAIT: begin
            if (s_axi_arvalid) begin
                s_axi_arready <= 1'b1;
                read_addr <= s_axi_araddr;
                read_state <= READ;
            end
            else read_state <= WAIT;
            end

            READ: begin
                //read_addr <= s_axi_araddr;
                if (read_addr == 4'h0) begin
                    s_axi_rdata <= ctrl_reg;
                end
                else if (read_addr == 4'h4) begin
                    s_axi_rdata <= delay_reg;
                end
                //s_axi_arready <= 1'b0;
                read_state <= RESPONSE;
            end

            RESPONSE: begin
                if (!s_axi_rvalid) begin
                    s_axi_arready <= 1'b0;
                    s_axi_rvalid <= 1'b1;
                    read_state <= RESPONSE;
                end
                else if (s_axi_rready) begin
                    s_axi_rvalid <= 1'b0;
                    read_state <= WAIT;
                end
                else 
                    read_state <= RESPONSE;
            end
            default: begin
                read_state <= WAIT;
            end
        endcase
end


// Interrupt generation
assign int_en = ctrl_reg[1];
assign counter_en = ctrl_reg[0];
assign irq = (counter == 0) && (int_en);

// Counter behavior
always_ff @(posedge s_axi_aclk) begin
    if (!s_axi_aresetn || counter_set)
        counter <= delay_reg;
    else if (counter_en && (counter == 0))
        counter <= delay_reg;
    else if (counter_en)
        counter <= counter - 1;
end

endmodule
