`timescale 1 ns / 1 ps

// Minimal AXI3 write-only “DMA” interface into PS7 S_AXI_HP0
// Assumes top-level pins all sideband constants:
//   AWLEN=0, AWSIZE=3, AWBURST=INCR, WSTRB=FF, WLAST=1, IDs=0, BREADY=1

module dma_controller
(
    // Clock / reset (HP0 domain: fclk0)
    input  logic         aclk,
    input  logic         aresetn,

    // AXI Write Address (minimal)
    output logic [31:0]  m_axi_awaddr,
    output logic         m_axi_awvalid,
    input  logic         m_axi_awready,

    // AXI Write Data (minimal)
    output logic [63:0]  m_axi_wdata,
    output logic         m_axi_wvalid,
    input  logic         m_axi_wready,

    // AXI Write Response (optional to observe)
    input  logic         m_axi_bvalid
);

    // -------------------------------------------------------------------------
    // Stub / skeleton implementation
    // Replace this with your real FSM later.
    //
    // For now, this does nothing (no writes). It just holds valid low.
    // -------------------------------------------------------------------------
    always_ff @(posedge aclk) begin
        if (!aresetn) begin
            m_axi_awaddr  <= 32'd0;
            m_axi_awvalid <= 1'b0;
            m_axi_wdata   <= 64'd0;
            m_axi_wvalid  <= 1'b0;
        end else begin
            // Idle by default
            m_axi_awvalid <= 1'b0;
            m_axi_wvalid  <= 1'b0;

            // Example placeholder:
            // If you want to test a single write, you can hardcode a one-shot here.
            // (but you said you’ll implement the real FSM next)
        end
    end

endmodule

