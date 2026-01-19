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

    localparam logic signed [14:0] ADC_OFFSET = 15'd8192;

////////////////////// ADC BLOCK ////////////////////////////////////

    assign adc_csn_o = 1'b1;

/////////////////////////////////////////////////////////


    logic rst_i, strobe_meta_w, strobe_sync_r, strobe_edge_w;
    assign rst_i = axi_from_ps_i[31];
    
    typedef enum logic [CMD_BITS-1:0] 
    {
        CMD_IDLE = 4'b0000,
        CMD_SET_LED = 4'b0001,
        CMD_SET_DAC = 4'b0010,
        CMD_GET_ADC = 4'b0011,
        CMD_CHECK_SIGNED = 4'b0100,
        CMD_SET_ROT_COEFFS = 4'b0101,
        CMD_COMMIT_ROT_COEFFS = 4'b0110
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

    logic [DATA_BITS-1:0] data_w;
    assign data_w = axi_from_ps_r[DATA_END:DATA_START];

    logic [7:0] led_r, next_led_w;
    
    logic [27:0] dac_tdata_r, next_dac_tdata_w;


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

    logic [AXI_GPIO_OUT_WIDTH-1:0] idle_cb_w, led_cb_w, dac_cb_w, adc_cb_w, cs_cb_w, set_rot_cb_w, commit_rot_cb_w;
    assign idle_cb_w    = 32'b0;
    assign led_cb_w     = {CMD_SET_LED, 20'd0, led_r};
    assign dac_cb_w     = {CMD_SET_DAC, dac_tdata_r[27:14], dac_tdata_r[13:0]};
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

            default: cs_cb_w = {base_bus, 16'd0};
            
        endcase
    end


    assign set_rot_cb_w = {CMD_SET_ROT_COEFFS, sin_theta_r[15:2], cos_theta_r[15:2]};
    assign commit_rot_cb_w = {CMD_COMMIT_ROT_COEFFS, q_feed_w[15:2], i_feed_w[15:2]};

    logic [AXI_GPIO_OUT_WIDTH-1 : 0] callback_r, next_callback_w;
    assign axi_to_ps_o = callback_r;

    always_comb begin
        unique case(cmd_w)
            CMD_IDLE: begin
                next_led_w = led_r;
                next_dac_tdata_w = dac_tdata_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = idle_cb_w;
            end

            CMD_SET_LED: begin
                next_led_w = data_w[7:0];
                next_dac_tdata_w = dac_tdata_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = led_cb_w;
            end
            
            CMD_SET_DAC: begin
                next_led_w = led_r;
                next_dac_tdata_w = data_w[14]? {data_w[13:0], dac_tdata_r[13:0]} : {dac_tdata_r[27:14], data_w[13:0]};
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = dac_cb_w;
            end

            CMD_GET_ADC: begin
                next_led_w = led_r;
                next_dac_tdata_w = dac_tdata_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = adc_cb_w;
            end

            CMD_CHECK_SIGNED: begin
                next_led_w = led_r;
                next_dac_tdata_w = dac_tdata_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = cs_cb_w;
            end

            CMD_SET_ROT_COEFFS: begin
                next_led_w = led_r;
                next_dac_tdata_w = dac_tdata_r;
                next_sin_theta_w = data_w[16]? data_w[15:0] : sin_theta_r;
                next_cos_theta_w = ~data_w[16]? data_w[15:0] : cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = set_rot_cb_w;
            end

            CMD_COMMIT_ROT_COEFFS: begin 
                next_led_w = led_r;
                next_dac_tdata_w = dac_tdata_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = sin_theta_r;
                next_rot_cos_theta_w = cos_theta_r;
                next_callback_w = commit_rot_cb_w;
            end

            default: begin
                next_led_w = led_r;
                next_dac_tdata_w = dac_tdata_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = 32'b0;
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

    assign dac_dat_o = data_w[13:0];
    assign dac_wrt_o = strobe_edge_w & (cmd_w == CMD_SET_DAC); //pin at 1 for now
    assign dac_sel_o = data_w[14];
    assign dac_rst_o = rst_i;
    assign dac_clk_o = clk;


    always_ff @(posedge clk) begin
        if(rst_i)begin
            strobe_sync_r <= 0;
            axi_from_ps_r <= 0;
            led_r <= 0;
            dac_tdata_r <= {2'b00, 14'h2000, 2'b00, 14'h2000}; //0x2000 -> ~0V
            sin_theta_r <= 0; 
            cos_theta_r <= 16'sh7FFF;
            rot_sin_theta_r <= 0;
            rot_cos_theta_r <= 16'sh7FFF;
            callback_r <= 0;
        end else begin
            strobe_sync_r <= strobe_meta_w;
            axi_from_ps_r <= next_axi_from_ps_w;
            led_r <= next_led_w;
            dac_tdata_r <= next_dac_tdata_w;
            sin_theta_r <= next_sin_theta_w; 
            cos_theta_r <= next_cos_theta_w;
            rot_sin_theta_r <= next_rot_sin_theta_w;
            rot_cos_theta_r <= next_rot_cos_theta_w;
            callback_r <= next_callback_w;
        end
    end

endmodule
