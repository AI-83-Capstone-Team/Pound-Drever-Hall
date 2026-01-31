`timescale 1 ns / 1 ps

module pdh_core #
(
    parameter ADC_DATA_WIDTH = 14, //To account for padding
    parameter DAC_DATA_WIDTH = 14,
    parameter AXI_GPIO_IN_WIDTH = 32,
    parameter AXI_GPIO_OUT_WIDTH = 32
)
(

    input  logic clk, 
    input  logic [ADC_DATA_WIDTH-1:0] adc_dat_a_i,
    input  logic [ADC_DATA_WIDTH-1:0] adc_dat_b_i,
    output logic adc_csn_o,
    
    output logic [DAC_DATA_WIDTH-1:0] dac_dat_o,
    output logic dac_wrt_o,
    output logic dac_rst_o,
    output logic dac_sel_o,

    input  logic [AXI_GPIO_IN_WIDTH-1:0] axi_from_ps_i,
    output logic [AXI_GPIO_OUT_WIDTH-1:0] axi_to_ps_o,
    output logic [7:0] led_o,
    output logic rst_o,

    output logic [25:0] decimation_code_o,

    output logic dma_enable_o,
    output logic [63:0] dma_data_o,
    input  logic dma_finished_i,
    input  logic dma_engaged_i
);
/////////////////////  LOCAL PARAMS   //////////////////////////////////
    localparam int unsigned NUM_MODULES = 2;
    localparam int unsigned CMD_BITS = 4;

    localparam int unsigned CMD_END = 29;
    localparam int unsigned CMD_START = 26;
    localparam int unsigned DATA_END = 25;
    localparam int unsigned DATA_START = 0;

    localparam logic signed [14:0] ADC_OFFSET = 15'd8192;

////////////////////// ADC BLOCK ////////////////////////////////////

    assign adc_csn_o = 1'b1;

/////////////////////////////////////////////////////////


    logic rst_i, rst_r, strobe_meta_w, strobe_sync_r, strobe_pipe1_r, strobe_edge_w;
    assign rst_i = axi_from_ps_i[31];
    assign rst_o = rst_r;
    
    typedef enum logic [CMD_BITS-1:0] 
    {
        CMD_IDLE = 4'b0000,
        CMD_SET_LED = 4'b0001,
        CMD_SET_DAC = 4'b0010,
        CMD_GET_ADC = 4'b0011,
        CMD_CHECK_SIGNED = 4'b0100,
        CMD_SET_ROT_COEFFS = 4'b0101,
        CMD_COMMIT_ROT_COEFFS = 4'b0110,
        CMD_GET_FRAME = 4'b0111
    } cmd_t;
    logic [AXI_GPIO_IN_WIDTH-1:0] axi_from_ps_r, next_axi_from_ps_w;
    logic [CMD_BITS-1:0] cmd_w;
    
    assign strobe_meta_w = axi_from_ps_i[30];
    posedge_detector u_strobe_edge_detector(
        .D(strobe_sync_r),
        .clk(clk),
        .rst(rst_i),
        .Q(strobe_edge_w)
    );

    always_comb begin
        next_axi_from_ps_w = strobe_edge_w? axi_from_ps_i : axi_from_ps_r;
    end


    assign cmd_w = axi_from_ps_r[CMD_END:CMD_START];

    logic [25:0] data_w;
    assign data_w = axi_from_ps_r[DATA_END:DATA_START];

    logic [7:0] led_r, next_led_w;
    


    logic signed [15:0] adc_dat_a_16s_w, adc_dat_b_16s_w;
    logic signed [14:0] tmp_s_a, tmp_s_b;
    always_comb begin
        tmp_s_a = -($signed({1'b0, adc_dat_a_i}) - ADC_OFFSET);
        tmp_s_b = -($signed({1'b0, adc_dat_b_i}) - ADC_OFFSET);
        adc_dat_a_16s_w = {tmp_s_a[14], tmp_s_a};
        adc_dat_b_16s_w = {tmp_s_b[14], tmp_s_b};
    end

    logic signed [15:0] i_feed_w, q_feed_w;

    logic signed [15:0] cos_theta_r, next_cos_theta_w, rot_cos_theta_r, next_rot_cos_theta_w, sin_theta_r, next_sin_theta_w, rot_sin_theta_r, next_rot_sin_theta_w;

    logic dma_engaged_r, dma_finished_r;
    always_ff @(posedge clk or posedge rst_i) begin
        if(rst_i) begin
            dma_engaged_r <= 1'b0;
            dma_finished_r <= 1'b0;
        end else begin
            dma_engaged_r <= dma_engaged_i;
            dma_finished_r <= dma_finished_i;
        end
    end
    

    logic [AXI_GPIO_OUT_WIDTH-1:0] idle_cb_w, led_cb_w, dac_cb_w, adc_cb_w, cs_cb_w, set_rot_cb_w, commit_rot_cb_w, get_frame_cb_w;
    assign idle_cb_w    = 32'b0;
    assign led_cb_w     = {CMD_SET_LED, 20'd0, led_r};
    assign dac_cb_w     = {CMD_SET_DAC, 13'b0, dac_sel_r, dac_dat_r};
    assign adc_cb_w     = {CMD_GET_ADC, adc_dat_b_i, adc_dat_a_i};

    logic [15:0] base_bus;
    always_comb begin
        base_bus = {CMD_CHECK_SIGNED, 8'd0, axi_from_ps_r[3:0]};
        case(axi_from_ps_r[3:0])
            4'b0000: cs_cb_w = {base_bus, adc_dat_a_16s_w};
           
            4'b0001: cs_cb_w = {base_bus, adc_dat_b_16s_w};

            4'b0010: cs_cb_w = {base_bus, cos_theta_r};
            
            4'b0011: cs_cb_w = {base_bus, sin_theta_r};

            4'b0100: cs_cb_w = {base_bus, rot_cos_theta_r};

            4'b0101: cs_cb_w = {base_bus, rot_sin_theta_r};
           
            4'b0110: cs_cb_w = {base_bus, i_feed_w};

            4'b0111: cs_cb_w = {base_bus, q_feed_w};

            4'b1000: cs_cb_w = {base_bus, 15'b0, dma_finished_r};

            4'b1001: cs_cb_w = {base_bus, 15'b0, dma_engaged_r};

            default: cs_cb_w = {base_bus, 16'd0};
            
        endcase
    end

    logic [25:0] decimation_code_r, next_decimation_code_w;

    assign set_rot_cb_w = {CMD_SET_ROT_COEFFS, sin_theta_r[15:2], cos_theta_r[15:2]};
    assign commit_rot_cb_w = {CMD_COMMIT_ROT_COEFFS, q_feed_w[15:2], i_feed_w[15:2]};
    assign get_frame_cb_w = {CMD_GET_FRAME, 1'd0, decimation_code_r, dma_engaged_r};

    logic [AXI_GPIO_OUT_WIDTH-1 : 0] callback_r, next_callback_w;
    assign axi_to_ps_o = callback_r;

    logic [13:0] dac_dat_r, next_dac_dat_w;
    logic dac_sel_r, next_dac_sel_w, dac_wrt_r, next_dac_wrt_w;

    always_comb begin
        unique case(cmd_w)
            CMD_IDLE: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = idle_cb_w;
                next_decimation_code_w = decimation_code_r;
                next_dac_wrt_w = 1'b0;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
            end

            CMD_SET_LED: begin
                next_led_w = data_w[7:0];
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = led_cb_w;
                next_decimation_code_w = decimation_code_r;
                next_dac_wrt_w = 1'b0;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
            end
            
            CMD_SET_DAC: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = dac_cb_w;
                next_decimation_code_w = decimation_code_r;
                next_dac_wrt_w = 1'b1;
                next_dac_sel_w = data_w[14];
                next_dac_dat_w = data_w[13:0];
            end

            CMD_GET_ADC: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = adc_cb_w;
                next_decimation_code_w = decimation_code_r;
                next_dac_wrt_w = 1'b0;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
            end

            CMD_CHECK_SIGNED: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = cs_cb_w;
                next_decimation_code_w = decimation_code_r;
                next_dac_wrt_w = 1'b0;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
            end

            CMD_SET_ROT_COEFFS: begin
                next_led_w = led_r;
                next_sin_theta_w = data_w[16]? data_w[15:0] : sin_theta_r;
                next_cos_theta_w = ~data_w[16]? data_w[15:0] : cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = set_rot_cb_w;
                next_decimation_code_w = decimation_code_r;
                next_dac_wrt_w = 1'b0;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
            end

            CMD_COMMIT_ROT_COEFFS: begin 
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = sin_theta_r;
                next_rot_cos_theta_w = cos_theta_r;
                next_callback_w = commit_rot_cb_w;
                next_decimation_code_w = decimation_code_r;
                next_dac_wrt_w = 1'b0;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
            end

            CMD_GET_FRAME: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = get_frame_cb_w;
                next_decimation_code_w = data_w[25:0];
                next_dac_wrt_w = 1'b0;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
            end

            default: begin
                next_led_w = 8'd0;
                next_sin_theta_w = 16'd0;
                next_cos_theta_w = 16'sh7FFF;
                next_rot_sin_theta_w = 16'd0;
                next_rot_cos_theta_w = 16'sh7FFF;
                next_callback_w = 32'b0;
                next_decimation_code_w = 26'd1;
                next_dac_wrt_w = 1'b0;
                next_dac_sel_w = 1'b0;
                next_dac_dat_w = 14'h2000;
            end
        endcase
    end


//////////////////  IQ FEED LOGIC /////////////////////

    logic signed [31:0] i_rot_w, q_rot_w;
    always_comb begin
        i_rot_w = $signed(rot_cos_theta_r)*$signed(adc_dat_a_16s_w) - $signed(rot_sin_theta_r)*$signed(adc_dat_b_16s_w);
        q_rot_w = $signed(rot_sin_theta_r)*$signed(adc_dat_a_16s_w) + $signed(rot_cos_theta_r)*$signed(adc_dat_b_16s_w);
        i_feed_w = i_rot_w >>> 15;
        q_feed_w = q_rot_w >>> 15;
    end



////////////////////////////////////////////////////////

    assign led_o = led_r; 
    
    assign dac_rst_o = 1'b0; //rst_i; //TODO: rst_r async set sync release

    assign dma_enable_o = (cmd_w==CMD_GET_FRAME);
    assign dma_data_o = {i_feed_w, q_feed_w, cos_theta_r, sin_theta_r};

    assign decimation_code_o = decimation_code_r;
    

    always_ff @(posedge clk or posedge rst_i) begin
        if(rst_i || rst_r)begin
            {strobe_sync_r, strobe_pipe1_r} <= {1'b0, 1'b0};
            axi_from_ps_r <= 0;
            led_r <= 0;
            sin_theta_r <= 0; 
            cos_theta_r <= 16'sh7FFF;
            rot_sin_theta_r <= 0;
            rot_cos_theta_r <= 16'sh7FFF;
            callback_r <= 0;
            decimation_code_r <= 26'd1;
            dac_wrt_r <= 1'b0;
            dac_sel_r <= 1'b0;
            dac_dat_r <= 14'h2000;
        end else begin
            {strobe_sync_r, strobe_pipe1_r} <= {strobe_pipe1_r, strobe_meta_w};
            axi_from_ps_r <= next_axi_from_ps_w;
            led_r <= next_led_w;
            sin_theta_r <= next_sin_theta_w; 
            cos_theta_r <= next_cos_theta_w;
            rot_sin_theta_r <= next_rot_sin_theta_w;
            rot_cos_theta_r <= next_rot_cos_theta_w;
            callback_r <= next_callback_w;
            dac_wrt_r <= next_dac_wrt_w;
            dac_sel_r <= next_dac_sel_w;
            dac_dat_r <= next_dac_dat_w;
        end

        rst_r <= rst_i;
    end

    assign dac_wrt_o = dac_wrt_r;
    assign dac_sel_o = dac_sel_r;
    assign dac_dat_o = dac_dat_r;

endmodule
