
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

    // Exported from BD (we will add these ports in Tcl)
    logic        pdh_rst_n;

    logic [31:0] axi_from_ps;     // PS -> pdh_core
    logic [31:0] axi_to_ps;       // pdh_core -> PS

    logic [31:0] pdh_adc_axis_tdata;  // ADC stream -> pdh_core
    logic        pdh_adc_axis_tvalid;

    logic [31:0] pdh_dac_axis_tdata;  // pdh_core -> DAC stream
    logic        pdh_dac_axis_tvalid;

    // Platform wrapper (PS + ADC/DAC + GPIO + clocks)
    system_wrapper u_system_wrapper (
        // existing external pins
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

        // new exported “platform” signals (added by Tcl)
        .pdh_rst_n(pdh_rst_n),
        .axi_from_ps(axi_from_ps),
        .axi_to_ps(axi_to_ps)
    );



    logic pdh_clk_0, pdh_clk;

    IBUFGDS adc_clk_inst0 (.I(adc_clk_p_i), .IB(adc_clk_n_i), .O(pdh_clk_0));
    BUFG adc_clk_inst (.I(pdh_clk_0), .O(pdh_clk));

    pdh_core #(
        .ADC_DATA_WIDTH(16),
        .DAC_DATA_WIDTH(14),
        .AXIS_TDATA_WIDTH(32),
        .AXI_GPIO_IN_WIDTH(32),
        .AXI_GPIO_OUT_WIDTH(32)
    ) u_pdh_core (
        .clk(pdh_clk),
        .axi_from_ps_i(axi_from_ps),
        .axi_to_ps_o(axi_to_ps),
        .led_o(led_o),
        .dac_clk_o(dac_clk_o),
        .dac_dat_o(dac_dat_o),
        .dac_rst_o(dac_rst_o),
        .dac_sel_o(dac_sel_o),
        .dac_wrt_o(dac_wrt_o),
        .adc_dat_a_i(adc_dat_a_i),
        .adc_dat_b_i(adc_dat_b_i),
        .adc_csn_o(adc_csn_o)
  );

endmodule
