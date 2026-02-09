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

    output logic [25:0] dma_decimation_code_o,

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
        CMD_GET_FRAME = 4'b0111,
        CMD_SET_KP = 4'b1000,
        CMD_SET_KD = 4'b1001,
        CMD_SET_KI = 4'b1010,
        CMD_SET_DEC = 4'b1011,
        CMD_SET_SP = 4'b1100,
        CMD_SET_ALPHA_SAT_EN = 4'b1101
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
    

    
    logic signed [15:0] kp_r, kd_r, ki_r, kp_w, kd_w, ki_w;
    logic signed [13:0] sp_w, sp_r;
    logic [13:0] dec_w, dec_r;
    logic [3:0] alpha_w, alpha_r;
    logic [4:0] satwidth_r, satwidth_w;
    logic pid_enable_w, pid_enable_r;
    
    logic [15:0] pid_out_w;



    logic [AXI_GPIO_OUT_WIDTH-1:0] idle_cb_w, 
        led_cb_w, 
        dac_cb_w, 
        adc_cb_w, 
        cs_cb_w, 
        set_rot_cb_w, 
        commit_rot_cb_w, 
        get_frame_cb_w,
        set_kp_cb_w,
        set_kd_cb_w,
        set_ki_cb_w,
        set_dec_cb_w,
        set_sp_cb_w,
        set_alpha_sat_en_cb_w;

        

    assign idle_cb_w    = 32'b0;
    assign led_cb_w     = {CMD_SET_LED, 20'd0, led_r};
    assign dac_cb_w     = {CMD_SET_DAC, 12'b0, dac_wrt_r, dac_sel_r, dac_dat_r};
    assign adc_cb_w     = {CMD_GET_ADC, adc_dat_b_i, adc_dat_a_i};

    logic [15:0] base_bus;
    always_comb begin
        base_bus = {CMD_CHECK_SIGNED, 7'd0, axi_from_ps_r[4:0]}; 
        case(axi_from_ps_r[4:0])
            5'b00000: cs_cb_w = {base_bus, adc_dat_a_16s_w};
           
            5'b00001: cs_cb_w = {base_bus, adc_dat_b_16s_w};

            5'b00010: cs_cb_w = {base_bus, cos_theta_r};
            
            5'b00011: cs_cb_w = {base_bus, sin_theta_r};

            5'b00100: cs_cb_w = {base_bus, rot_cos_theta_r};

            5'b00101: cs_cb_w = {base_bus, rot_sin_theta_r};
           
            5'b00110: cs_cb_w = {base_bus, i_feed_w};

            5'b00111: cs_cb_w = {base_bus, q_feed_w};

            5'b01000: cs_cb_w = {base_bus, 15'b0, dma_finished_r};

            5'b01001: cs_cb_w = {base_bus, 15'b0, dma_engaged_r};

            5'b01010: cs_cb_w = {base_bus, kp_r};

            5'b01011: cs_cb_w = {base_bus, kd_r};

            5'b01100: cs_cb_w = {base_bus, ki_r};

            5'b01101: cs_cb_w = {base_bus, 2'b0, dec_r};
            
            5'b01110: cs_cb_w = {base_bus, 2'b0, sp_r};
            
            5'b01111: cs_cb_w = {base_bus, 6'b0, satwidth_r, alpha_r, pid_enable_r};

            default: cs_cb_w = {base_bus, 16'd0};
            
        endcase
    end

    logic [25:0] dma_decimation_code_r, next_dma_decimation_code_w; //TODO: Add this as CS option

    assign set_rot_cb_w = {CMD_SET_ROT_COEFFS, sin_theta_r[15:2], cos_theta_r[15:2]};
    assign commit_rot_cb_w = {CMD_COMMIT_ROT_COEFFS, q_feed_w[15:2], i_feed_w[15:2]};
    assign get_frame_cb_w = {CMD_GET_FRAME, 1'd0, dma_decimation_code_r, dma_engaged_r};
    assign set_kp_cb_w = {CMD_SET_KP, kp_r};
    assign set_kd_cb_w = {CMD_SET_KD, kd_r};
    assign set_ki_cb_w = {CMD_SET_KI, ki_r};
    assign set_dec_cb_w = {CMD_SET_DEC, 2'b0, dec_r};
    assign set_sp_cb_w = {CMD_SET_SP, {2{sp_r[13]}}, sp_r};
    assign set_alpha_sat_en_cb_w = {CMD_SET_ALPHA_SAT_EN, 6'b0, alpha_r, satwidth_r, pid_enable_r};


    logic [AXI_GPIO_OUT_WIDTH-1 : 0] callback_r, next_callback_w;
    assign axi_to_ps_o = callback_r;

    logic [13:0] dac_dat_r, next_dac_dat_w;
    logic dac_sel_r, next_dac_sel_w, dac_wrt_r, next_dac_wrt_w;


    
        
    pid_core u_pid(
        .clk(clk),
        .rst(rst_r),
        .kp_i(kp_r),
        .kd_i(kd_r),
        .ki_i(ki_r),
        .decimate_i(decimate_r),
        .sp_i(sp_r),
        .alpha_i(alpha_r),
        .satwidth_i(satwidth_r),
        .dat_i(i_feed_w), //TODO: Move from I-feed to (I^2+Q^2)sign(I)
        .enable_i(pid_enable_r),
        .pid_out(pid_out_w)
    );

    

    //TODO: Make this more concise 
    always_comb begin
        unique case(cmd_w)
            CMD_IDLE: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = idle_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = dac_wrt_r;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
                kp_w = kp_r;
                kd_w = kd_r;
                ki_w = ki_r;
                dec_w = dec_r;
                sp_w = sp_r;
                alpha_w = alpha_r;
                satwidth_w = satwidth_r;
                pid_enable_w = pid_enable_r;
            end

            CMD_SET_LED: begin
                next_led_w = data_w[7:0];
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = led_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = dac_wrt_r;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
                kp_w = kp_r;
                kd_w = kd_r;
                ki_w = ki_r;
                dec_w = dec_r;
                sp_w = sp_r;
                alpha_w = alpha_r;
                satwidth_w = satwidth_r;
                pid_enable_w = pid_enable_r;
            end
            
            CMD_SET_DAC: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = dac_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = data_w[15]; //TODO: Pass this as an arg
                next_dac_sel_w = data_w[14];
                next_dac_dat_w = data_w[13:0];
                kp_w = kp_r;
                kd_w = kd_r;
                ki_w = ki_r;
                dec_w = dec_r;
                sp_w = sp_r;
                alpha_w = alpha_r;
                satwidth_w = satwidth_r;
                pid_enable_w = pid_enable_r;
            end

            CMD_GET_ADC: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = adc_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = dac_wrt_r;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
                kp_w = kp_r;
                kd_w = kd_r;
                ki_w = ki_r;
                dec_w = dec_r;
                sp_w = sp_r;
                alpha_w = alpha_r;
                satwidth_w = satwidth_r;
                pid_enable_w = pid_enable_r;
            end

            CMD_CHECK_SIGNED: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = cs_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = dac_wrt_r;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
                kp_w = kp_r;
                kd_w = kd_r;
                ki_w = ki_r;
                dec_w = dec_r;
                sp_w = sp_r;
                alpha_w = alpha_r;
                satwidth_w = satwidth_r;
                pid_enable_w = pid_enable_r;
            end

            CMD_SET_ROT_COEFFS: begin
                next_led_w = led_r;
                next_sin_theta_w = data_w[16]? data_w[15:0] : sin_theta_r;
                next_cos_theta_w = ~data_w[16]? data_w[15:0] : cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = set_rot_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = dac_wrt_r;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
                kp_w = kp_r;
                kd_w = kd_r;
                ki_w = ki_r;
                dec_w = dec_r;
                sp_w = sp_r;
                alpha_w = alpha_r;
                satwidth_w = satwidth_r;
                pid_enable_w = pid_enable_r;
            end

            CMD_COMMIT_ROT_COEFFS: begin 
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = sin_theta_r;
                next_rot_cos_theta_w = cos_theta_r;
                next_callback_w = commit_rot_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = dac_wrt_r;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
                kp_w = kp_r;
                kd_w = kd_r;
                ki_w = ki_r;
                dec_w = dec_r;
                sp_w = sp_r;
                alpha_w = alpha_r;
                satwidth_w = satwidth_w;
                pid_enable_w = pid_enable_r;
            end

            CMD_GET_FRAME: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = get_frame_cb_w;
                next_dma_decimation_code_w = data_w[25:0];
                next_dac_wrt_w = dac_wrt_r;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
                kp_w = kp_r;
                kd_w = kd_r;
                ki_w = ki_r;
                dec_w = dec_r;
                sp_w = sp_r;
                alpha_w = alpha_r;
                satwidth_w = satwidth_w;
                pid_enable_w = pid_enable_r;
            end

            
            CMD_SET_KP: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = set_kp_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = dac_wrt_r;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
                kp_w = data_w[15:0];
                kd_w = kd_r;
                ki_w = ki_r;
                dec_w = dec_r;
                sp_w = sp_r;
                alpha_w = alpha_r;
                satwidth_w = satwidth_r;
                pid_enable_w = pid_enable_r;
            end

            CMD_SET_KD: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = set_kd_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = dac_wrt_r;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
                kp_w = kp_r;
                kd_w = data_w[15:0];
                ki_w = ki_r;
                dec_w = dec_r;
                sp_w = sp_r;
                alpha_w = alpha_r;
                satwidth_w = satwidth_r;
                pid_enable_w = pid_enable_r;
            end

            CMD_SET_KI: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = set_ki_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = dac_wrt_r;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
                kp_w = kp_r;
                kd_w = kd_r;
                ki_w = data_w[15:0];
                dec_w = dec_r;
                sp_w = sp_r;
                alpha_w = alpha_r;
                satwidth_w = satwidth_r;
                pid_enable_w = pid_enable_r;
            end

            CMD_SET_DEC: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = set_dec_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = dac_wrt_r;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
                kp_w = kp_r;
                kd_w = kd_r;
                ki_w = ki_r;
                dec_w = data_w[13:0];
                sp_w = sp_r;
                alpha_w = alpha_r;
                satwidth_w = satwidth_r;
                pid_enable_w = pid_enable_r;
            end

            CMD_SET_SP: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = set_sp_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = dac_wrt_r;
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = dac_dat_r;
                kp_w = kp_r;
                kd_w = kd_r;
                ki_w = ki_r;
                dec_w = dec_r;
                sp_w = data_w[13:0];
                alpha_w = alpha_r;
                satwidth_w = satwidth_r;
                pid_enable_w = pid_enable_r;
            end

            CMD_SET_ALPHA_SAT_EN: begin
                next_led_w = led_r;
                next_sin_theta_w = sin_theta_r;
                next_cos_theta_w = cos_theta_r;
                next_rot_sin_theta_w = rot_sin_theta_r;
                next_rot_cos_theta_w = rot_cos_theta_r;
                next_callback_w = set_alpha_sat_en_cb_w;
                next_dma_decimation_code_w = dma_decimation_code_r;
                next_dac_wrt_w = data_w[0];
                next_dac_sel_w = dac_sel_r;
                next_dac_dat_w = pid_out_w;
                kp_w = kp_r;
                kd_w = kd_r;
                ki_w = ki_r;
                dec_w = dec_r;
                sp_w = sp_r;
                alpha_w = data_w[9:6];
                satwidth_w = data_w[5:1];
                pid_enable_w = data_w[0];
            end

            default: begin
                next_led_w = 8'd0;
                next_sin_theta_w = 16'd0;
                next_cos_theta_w = 16'sh7FFF;
                next_rot_sin_theta_w = 16'd0;
                next_rot_cos_theta_w = 16'sh7FFF;
                next_callback_w = 32'b0;
                next_dma_decimation_code_w = 26'd1;
                next_dac_wrt_w = 1'b0;
                next_dac_sel_w = 1'b0;
                next_dac_dat_w = 14'h2000;
                kp_w = '0;
                kd_w = '0;
                ki_w = '0;
                dec_w = 14'd1;
                sp_w = 16'sh7FFF;
                alpha_w = 4'd4;
                satwidth_w = 5'd31;
                pid_enable_w = 1'b0;
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

    assign dma_decimation_code_o = dma_decimation_code_r;
    

    always_ff @(posedge clk or posedge rst_i) begin
        if(rst_i || rst_r)begin
            {strobe_sync_r, strobe_pipe1_r} <= {1'b0, 1'b0};
            axi_from_ps_r <= 0;
            led_r <= 0;
            sin_theta_r <= 0; 
            cos_theta_r <= 16'sh7FFF;
            rot_sin_theta_r <= 0;
            rot_cos_theta_r <= 16'sh7FFF;
            dma_decimation_code_r <= 26'd1;
            kp_r <= '0;
            kd_r <= '0;
            ki_r <= '0;
            dec_r <= 14'd1;
            sp_r <= 16'sh7FFF;
            alpha_r <= 4'd4;
            satwidth_r <= 5'd31;
            pid_enable_r <= 1'b0;
            callback_r <= 0;

        end else begin
            {strobe_sync_r, strobe_pipe1_r} <= {strobe_pipe1_r, strobe_meta_w};
            axi_from_ps_r <= next_axi_from_ps_w;
            led_r <= next_led_w;
            sin_theta_r <= next_sin_theta_w; 
            cos_theta_r <= next_cos_theta_w;
            rot_sin_theta_r <= next_rot_sin_theta_w;
            rot_cos_theta_r <= next_rot_cos_theta_w;
            dma_decimation_code_r <= next_dma_decimation_code_w;
            kp_r <= kp_w;
            kd_r <= kd_w;
            ki_r <= ki_w;
            dec_r <= dec_w;
            sp_r <= sp_w;
            alpha_r <= alpha_w;
            satwidth_r <= satwidth_w;
            pid_enable_r <= pid_enable_w;
            callback_r <= next_callback_w;
        end

        rst_r <= rst_i;
    end

    always_ff @(negedge clk) begin
        dac_wrt_r <= next_dac_wrt_w;
        dac_sel_r <= next_dac_sel_w;
        dac_dat_r <= next_dac_dat_w;
    end

    assign dac_wrt_o = dac_wrt_r;
    assign dac_sel_o = dac_sel_r;
    assign dac_dat_o = dac_dat_r;

endmodule
