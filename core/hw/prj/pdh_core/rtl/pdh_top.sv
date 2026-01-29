
`timescale 1 ps / 1 ps

module pdh_top
(
    // Zynq PS external
    inout  [14:0] DDR_addr,
    inout  [2:0]  DDR_ba,
    inout         DDR_cas_n,
    inout         DDR_ck_n,
    inout         DDR_ck_p,
    inout         DDR_cke,
    inout         DDR_cs_n,
    inout  [3:0]  DDR_dm,
    inout  [31:0] DDR_dq,
    inout  [3:0]  DDR_dqs_n,
    inout  [3:0]  DDR_dqs_p,
    inout         DDR_odt,
    inout         DDR_ras_n,
    inout         DDR_reset_n,
    inout         DDR_we_n,

    inout         FIXED_IO_ddr_vrn,
    inout         FIXED_IO_ddr_vrp,
    inout  [53:0] FIXED_IO_mio,
    inout         FIXED_IO_ps_clk,
    inout         FIXED_IO_ps_porb,
    inout         FIXED_IO_ps_srstb,

    // ADC
    input         adc_clk_n_i,
    input         adc_clk_p_i,
    output        adc_csn_o,
    input  [13:0] adc_dat_a_i,
    input  [13:0] adc_dat_b_i,
    output        adc_enc_n_o,
    output        adc_enc_p_o,

    // DAC
    output        dac_clk_o,
    output [13:0] dac_dat_o,
    output        dac_rst_o,
    output        dac_sel_o,
    output        dac_wrt_o,

    // LED
    output [7:0]  led_o
);

    // -----------------------------
    // PS <-> GPIO to pdh_core
    // -----------------------------
    logic [0:0]  pdh_rst_n;      // wrapper exports [0:0]
    logic [31:0] axi_from_ps;    // PS -> pdh_core
    logic [31:0] axi_to_ps;      // pdh_core -> PS

    // -----------------------------
    // Clocks
    // -----------------------------
    logic pdh_clk_0, pdh_clk;

    IBUFGDS adc_clk_inst0 (.I(adc_clk_p_i), .IB(adc_clk_n_i), .O(pdh_clk_0));
    BUFG    adc_clk_inst  (.I(pdh_clk_0),   .O(pdh_clk));

    assign dac_clk_o = pdh_clk;

    // -----------------------------
    // HP0 clock/reset exported by BD
    // -----------------------------
    logic fclk0;
    logic fclk0_resetn;


    // -----------------------------
    // HP0 AXI constants (pinned)
    // -----------------------------

    wire [5:0] hp0_awid    = 6'd0;
    wire [3:0] hp0_awcache = 4'd0;
    wire [2:0] hp0_awprot  = 3'd0;
    wire [1:0] hp0_awlock  = 2'd0;
    wire [3:0] hp0_awqos   = 4'd0;

    wire [5:0] hp0_wid     = 6'd0;

    // -----------------------------
    // HP0 AXI READ CHANNEL (tied off)
    // Only the INPUTS to the wrapper need driving.
    // -----------------------------
    wire [31:0] hp0_araddr  = 32'd0;
    wire [1:0]  hp0_arburst = 2'd0;
    wire [3:0]  hp0_arcache = 4'd0;
    wire [5:0]  hp0_arid    = 6'd0;
    wire [3:0]  hp0_arlen   = 4'd0;
    wire [1:0]  hp0_arlock  = 2'd0;
    wire [2:0]  hp0_arprot  = 3'd0;
    wire [3:0]  hp0_arqos   = 4'd0;
    wire [2:0]  hp0_arsize  = 3'd0;
    wire        hp0_arvalid = 1'b0;

    wire        hp0_rready  = 1'b0;     // never accept reads


    // Hot AXI signals
    logic [31:0] m_axi_awaddr;
    logic        m_axi_awvalid;
    logic        m_axi_awready;
    logic [3:0]  m_axi_awlen;   //num beats in burst
    logic [2:0]  m_axi_awsize;  //num bytes in beat
    logic [1:0]  m_axi_awburst; //burst code

    logic [63:0] m_axi_wdata;
    logic        m_axi_wvalid;
    logic        m_axi_wready;
    logic        m_axi_bvalid;
    logic        m_axi_bready;
    logic [1:0]  m_axi_bresp; 
    logic [7:0]  m_axi_wstrb;
    logic        m_axi_wlast;

    // -----------------------------
    // Platform wrapper (PS + DDR + GPIO + HP0 exported)
    // -----------------------------
    system_wrapper u_system_wrapper (
        // external pins
        .DDR_addr(DDR_addr),
        .DDR_ba(DDR_ba),
        .DDR_cas_n(DDR_cas_n),
        .DDR_ck_n(DDR_ck_n),
        .DDR_ck_p(DDR_ck_p),
        .DDR_cke(DDR_cke),
        .DDR_cs_n(DDR_cs_n),
        .DDR_dm(DDR_dm),
        .DDR_dq(DDR_dq),
        .DDR_dqs_n(DDR_dqs_n),
        .DDR_dqs_p(DDR_dqs_p),
        .DDR_odt(DDR_odt),
        .DDR_ras_n(DDR_ras_n),
        .DDR_reset_n(DDR_reset_n),
        .DDR_we_n(DDR_we_n),

        .FIXED_IO_ddr_vrn(FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp(FIXED_IO_ddr_vrp),
        .FIXED_IO_mio(FIXED_IO_mio),
        .FIXED_IO_ps_clk(FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),

        .adc_enc_n_o(adc_enc_n_o),
        .adc_enc_p_o(adc_enc_p_o),

        // GPIO bridge to pdh_core
        .pdh_rst_n(pdh_rst_n),
        .axi_from_ps(axi_from_ps),
        .axi_to_ps(axi_to_ps),

        // exported PS clock/reset
        .fclk0(fclk0),
        .fclk0_resetn(fclk0_resetn),

        // HP0 READ (tied off inputs)
        .S_AXI_HP0_EXT_araddr (hp0_araddr),
        .S_AXI_HP0_EXT_arburst(hp0_arburst),
        .S_AXI_HP0_EXT_arcache(hp0_arcache),
        .S_AXI_HP0_EXT_arid   (hp0_arid),
        .S_AXI_HP0_EXT_arlen  (hp0_arlen),
        .S_AXI_HP0_EXT_arlock (hp0_arlock),
        .S_AXI_HP0_EXT_arprot (hp0_arprot),
        .S_AXI_HP0_EXT_arqos  (hp0_arqos),
        .S_AXI_HP0_EXT_arsize (hp0_arsize),
        .S_AXI_HP0_EXT_arvalid(hp0_arvalid),
        .S_AXI_HP0_EXT_rready (hp0_rready),

        // HP0 WRITE (minimal + pinned constants)
        .S_AXI_HP0_EXT_awaddr (m_axi_awaddr),
        .S_AXI_HP0_EXT_awburst(m_axi_awburst),
        .S_AXI_HP0_EXT_awcache(hp0_awcache),
        .S_AXI_HP0_EXT_awid   (hp0_awid),
        .S_AXI_HP0_EXT_awlen  (m_axi_awlen),
        .S_AXI_HP0_EXT_awlock (hp0_awlock),
        .S_AXI_HP0_EXT_awprot (hp0_awprot),
        .S_AXI_HP0_EXT_awqos  (hp0_awqos),
        .S_AXI_HP0_EXT_awsize (m_axi_awsize),
        .S_AXI_HP0_EXT_awvalid(m_axi_awvalid),
        .S_AXI_HP0_EXT_awready(m_axi_awready),

        .S_AXI_HP0_EXT_wdata  (m_axi_wdata),
        .S_AXI_HP0_EXT_wid    (hp0_wid),
        .S_AXI_HP0_EXT_wlast  (m_axi_wlast),
        .S_AXI_HP0_EXT_wstrb  (m_axi_wstrb),
        .S_AXI_HP0_EXT_wvalid (m_axi_wvalid),
        .S_AXI_HP0_EXT_wready (m_axi_wready),

        .S_AXI_HP0_EXT_bvalid (m_axi_bvalid),
        .S_AXI_HP0_EXT_bready (m_axi_bready)

        // NOTE: all other HP0 outputs (ARREADY/RDATA/etc, BRESP/BID, etc)
        // are intentionally left unconnected.
    );

    logic core_rst, dma_enable_w, dma_finished_w, dma_engaged_w;
    logic [63:0] dma_data_w;


    pdh_core #(
        .ADC_DATA_WIDTH(14),
        .DAC_DATA_WIDTH(14),
        .AXI_GPIO_IN_WIDTH(32),
        .AXI_GPIO_OUT_WIDTH(32)
    ) u_pdh_core (
        .clk(pdh_clk),
        .axi_from_ps_i(axi_from_ps),
        .axi_to_ps_o(axi_to_ps),
        .led_o(led_o),
        .dac_dat_o(dac_dat_o),
        .dac_rst_o(dac_rst_o),
        .dac_sel_o(dac_sel_o),
        .dac_wrt_o(dac_wrt_o),
        .adc_dat_a_i(adc_dat_a_i),
        .adc_dat_b_i(adc_dat_b_i),
        .adc_csn_o(adc_csn_o),
        .rst_o(core_rst),

        .dma_enable_o(dma_enable_w),
        .dma_data_o(dma_data_w),
        .dma_finished_i(dma_finished_w),
        .dma_engaged_i(dma_engaged_w)
    );


    
    //(* ram_style = "block" *) reg[63:0] mem[0:1];
    //logic [63:0] dma_dout;
    //always @(posedge pdh_clk) begin
    //    mem[0] <= dma_data_w;
    //    dma_dout <= mem[0];
    //end

    logic [31:0] bram_addr_w;
    logic [63:0] bram_out_w;
    logic dma_controller_enable_w, dma_controller_finished_w;
    bram_controller u_bram(
        .pdh_clk(pdh_clk),
        .axi_clk(fclk0),
        .rst_i(core_rst),
        .addr_i(bram_addr_w),
        .din(dma_data_w),
        .dout(bram_out_w),
        .enable_i(dma_enable_w),
        .dma_enable(dma_controller_enable_w),
        .dma_termination_sig(dma_controller_finished_w),
        .transaction_complete(dma_finished_w)
    );


    dma_controller u_dma (
        .aclk(fclk0),
        .rst_i(core_rst),
        .m_axi_awaddr(m_axi_awaddr),
        .m_axi_awvalid(m_axi_awvalid),
        .m_axi_awready(m_axi_awready),
        .m_axi_awlen(m_axi_awlen),
        .m_axi_awsize(m_axi_awsize),
        .m_axi_awburst(m_axi_awburst),
        .m_axi_wdata(m_axi_wdata),
        .m_axi_wvalid(m_axi_wvalid),
        .m_axi_wready(m_axi_wready),
        .m_axi_bvalid(m_axi_bvalid),
        .m_axi_bready(m_axi_bready),
        .m_axi_bresp(m_axi_bresp),
        .m_axi_wstrb(m_axi_wstrb),
        .m_axi_wlast(m_axi_wlast),
        
        .enable_i(dma_controller_enable_w),
        .data_i(bram_out_w),
        .finished_o(dma_controller_finished_w),
        .dma_engaged_o(dma_engaged_w),
        .bram_addr_o(bram_addr_w)
    );

endmodule
