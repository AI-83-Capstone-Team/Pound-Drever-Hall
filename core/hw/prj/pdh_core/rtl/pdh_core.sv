`timescale 1 ns / 1 ps

module pdh_core #
(
    parameter ADC_DATA_WIDTH = 14, //To account for padding
    parameter DAC_DATA_WIDTH = 14,
    parameter AXI_GPIO_IN_WIDTH = 32,
    parameter AXI_GPIO_OUT_WIDTH = 32
)
(

    input logic clk, 
    input logic [ADC_DATA_WIDTH-1:0] adc_dat_a_i,
    input logic [ADC_DATA_WIDTH-1:0] adc_dat_b_i,
    output logic adc_csn_o,
    
    output logic [DAC_DATA_WIDTH-1:0] dac_dat_o,
    output logic dac_wrt_o,
    output logic dac_rst_o,
    output logic dac_sel_o,

    input logic [AXI_GPIO_IN_WIDTH-1:0] axi_from_ps_i,
    output logic [AXI_GPIO_OUT_WIDTH-1:0] axi_to_ps_o,
    output logic [7:0] led_o

);
/////////////////////  LOCAL PARAMS   //////////////////////////////////
    localparam int unsigned NUM_MODULES = 2;
    localparam int unsigned CMD_BITS = 4;
    localparam int unsigned DATA_BITS = 26;

    localparam int unsigned CMD_END = 29;
    localparam int unsigned CMD_START = 26;
    localparam int unsigned DATA_END = 25;
    localparam int unsigned DATA_START = 0;


////////////////////// ADC BLOCK ////////////////////////////////////

    assign adc_csn_o = 1'b1;

/////////////////////////////////////////////////////////


    logic rst_i, strobe_w, strobe_edge_w;
    assign rst_i = axi_from_ps_i[31];
    
    typedef enum logic [CMD_BITS-1:0] 
    {
        CMD_IDLE = 4'b0000,
        CMD_SET_LED = 4'b0001,
        CMD_SET_DAC = 4'b0010,
        CMD_GET_ADC = 4'b0100
    } cmd_t;
    logic [AXI_GPIO_IN_WIDTH-1:0] axi_from_ps_r;
    logic [CMD_BITS-1:0] cmd_w;


    assign cmd_w = axi_from_ps_r[CMD_END:CMD_START];
    
    assign strobe_w = axi_from_ps_r[30];
    posedge_detector u_strobe_edge_detector(
        .D(strobe_w),
        .clk(clk),
        .rst(rst_i),
        .Q(strobe_edge_w)
    );


    logic [DATA_BITS-1:0] data_w;
    assign data_w = axi_from_ps_r[DATA_END:DATA_START];

    logic [7:0] led_r, next_led_w;
    
    logic [27:0] dac_tdata_r, next_dac_tdata_w;

    logic [AXI_GPIO_OUT_WIDTH-1:0] idle_cb_w, led_cb_w, dac_cb_w, adc_cb_w;
    assign idle_cb_w = 32'b0;
    assign led_cb_w = {CMD_SET_LED, 20'd0, led_r};
    assign dac_cb_w = {CMD_SET_DAC, dac_tdata_r[27:14], dac_tdata_r[13:0]};
    assign adc_cb_w = {CMD_GET_ADC, adc_dat_b_i, adc_dat_a_i};
    

    always_comb begin
        case(cmd_w)
            CMD_IDLE: begin
                next_led_w = led_r;
                next_dac_tdata_w = dac_tdata_r;
                next_callback_w = idle_cb_w;
            end

            CMD_SET_LED: begin
                next_led_w = strobe_edge_w? data_w[7:0] : led_r;
                next_dac_tdata_w = dac_tdata_r;
                next_callback_w = led_cb_w;
            end
            
            CMD_SET_DAC: begin
                next_led_w = led_r;
                next_dac_tdata_w = strobe_edge_w? (data_w[14]? {data_w[13:0], dac_tdata_r[13:0]} : {dac_tdata_r[27:14], data_w[13:0]}) : dac_tdata_r;
                next_callback_w = dac_cb_w;
            end

            CMD_GET_ADC: begin
                next_led_w = led_r;
                next_dac_tdata_w = dac_tdata_r;
                next_callback_w = adc_cb_w;
            end

            default: begin
                next_led_w = led_r;
                next_dac_tdata_w = dac_tdata_r;
                next_callback_w = 32'b0;
            end
        endcase
    end

    assign led_o = led_r; 

    assign dac_dat_o = data_w[13:0];
    assign dac_wrt_o = strobe_edge_w & (cmd_w == CMD_SET_DAC); //pin at 1 for now
    assign dac_sel_o = data_w[14];
    assign dac_rst_o = rst_i;
    assign dac_clk_o = clk;

    logic [AXI_GPIO_OUT_WIDTH-1 : 0] callback_r, next_callback_w;
    assign axi_to_ps_o = callback_r;

    always_ff @(posedge clk) begin
        if(rst_i)begin
            axi_from_ps_r <= 0;
            led_r <= 0;
            dac_tdata_r <= {2'b00, 14'h2000, 2'b00, 14'h2000}; //0x2000 -> ~0V
            callback_r <= 0;
        end else begin
            axi_from_ps_r <= axi_from_ps_i;
            led_r <= next_led_w;
            dac_tdata_r <= next_dac_tdata_w;
            callback_r <= next_callback_w;
        end
    end

endmodule
