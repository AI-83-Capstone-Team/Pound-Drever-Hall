`timescale 1 ns / 1 ps

module pdh_core #
(
    parameter ADC_DATA_WIDTH = 16, //To account for padding
    parameter DAC_DATA_WIDTH = 14,
    parameter AXIS_TDATA_WIDTH = 32, //Delivered as 2 16-bit unsigned ints packed together
    parameter AXI_GPIO_IN_WIDTH = 32,
    parameter AXI_GPIO_OUT_WIDTH = 32
)
(
    input wire rst_n, //xlc_reset.dout
    input wire clk, //FCLK_CLK0

    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [AXIS_TDATA_WIDTH-1:0] S_AXIS_tdata,
    input wire S_AXIS_tvalid,

    input wire [AXI_GPIO_IN_WIDTH-1:0] axi_from_ps,
    output wire [AXI_GPIO_OUT_WIDTH-1:0] axi_to_ps
);
    wire [ADC_DATA_WIDTH-1:0] adc_1_tdata;
    wire adc_1_tvalid;
    
    wire [ADC_DATA_WIDTH-1:0] adc_2_tdata;
    wire adc_2_tvalid;

    assign adc_1_tdata = S_AXIS_tdata[ADC_DATA_WIDTH-1:0];
    assign adc_2_tdata = S_AXIS_tdata[AXIS_TDATA_WIDTH-1:ADC_DATA_WIDTH];
    
    assign adc_1_tvalid = S_AXIS_tvalid;
    assign adc_2_tvalid = S_AXIS_tvalid;






endmodule
