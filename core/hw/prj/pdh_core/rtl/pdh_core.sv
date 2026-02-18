`timescale 1 ns / 1 ps

//TODO: Synchronous reset sig out to DMA block + DMA_RDY 2ff callback
////TODO: See if 3-ff sync is worth it
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

    output logic dma_enable_o,
    output logic bram_enable_o,
    input  logic dma_ready_i,
    input  logic bram_ready_i,
    output logic [21:0] dma_decimation_code_o,    
    output logic [63:0] dma_data_o
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


    wire rst_i;
    assign rst_i = axi_from_ps_i[31];
    assign rst_o = rst_i;
    
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
        CMD_SET_ALPHA_SAT_EN = 4'b1101,
        CMD_CONFIG_IO = 4'b1110
    } cmd_t;
    logic [AXI_GPIO_IN_WIDTH-1:0] axi_from_ps_r, next_axi_from_ps_w;
    logic [CMD_BITS-1:0] cmd_w;
    
    logic strobe_bit_w, strobe_edge_w;
    logic [31:0] axi_1ff_r, axi_2ff_r, axi_3ff_r;

    assign strobe_bit_w = axi_3ff_r[30];
    posedge_detector u_strobe_edge_detector(
        .D(strobe_bit_w),
        .clk(clk),
        .rst(rst_sync_r),
        .Q(strobe_edge_w)
    );

    always_comb begin
        next_axi_from_ps_w = strobe_edge_w? axi_3ff_r : axi_from_ps_r;
    end


    assign cmd_w = axi_from_ps_r[CMD_END:CMD_START];

    logic [25:0] data_w;
    assign data_w = axi_from_ps_r[DATA_END:DATA_START];

    logic [7:0] led_r, next_led_w;
    

    logic signed [15:0] i_feed_w, q_feed_w, i_feed_r, q_feed_r;
    logic signed [15:0] cos_theta_r, next_cos_theta_w, rot_cos_theta_r, next_rot_cos_theta_w, sin_theta_r, next_sin_theta_w, rot_sin_theta_r, next_rot_sin_theta_w;

    logic signed [15:0] kp_r, kd_r, ki_r, kp_w, kd_w, ki_w;
    logic signed [13:0] sp_w, sp_r;
    logic [13:0] dec_w, dec_r;
    logic [3:0] alpha_w, alpha_r;
    logic [4:0] satwidth_r, satwidth_w;
    logic pid_enable_w, pid_enable_r;
    
    logic [13:0] pid_out_w;


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
        set_alpha_sat_en_cb_w,
        config_io_cb_w;

        
    assign idle_cb_w    = 32'b0;
    assign led_cb_w     = {CMD_SET_LED, 20'd0, led_r};
    assign dac_cb_w     = {CMD_SET_DAC, dac1_dat_r, dac2_dat_r};
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
           
            5'b00110: cs_cb_w = {base_bus, i_feed_r};

            5'b00111: cs_cb_w = {base_bus, q_feed_r};

            5'b01000: cs_cb_w = {base_bus, 15'b0, dma_ready_3ff};

            5'b01010: cs_cb_w = {base_bus, kp_r};

            5'b01011: cs_cb_w = {base_bus, kd_r};

            5'b01100: cs_cb_w = {base_bus, ki_r};

            5'b01101: cs_cb_w = {base_bus, 2'b0, dec_r};
            
            5'b01110: cs_cb_w = {base_bus, {2{sp_r[13]}}, sp_r};
            
            5'b01111: cs_cb_w = {base_bus, 6'b0, satwidth_r, alpha_r, pid_enable_r};


            default: cs_cb_w = {base_bus, 16'd0};
            
        endcase
    end

    logic [21:0] dma_decimation_code_r, next_dma_decimation_code_w; //TODO: Add this as CS option

    assign set_rot_cb_w = {CMD_SET_ROT_COEFFS, sin_theta_r[15:2], cos_theta_r[15:2]};
    assign commit_rot_cb_w = {CMD_COMMIT_ROT_COEFFS, q_feed_r[15:2], i_feed_r[15:2]};
    assign get_frame_cb_w = {CMD_GET_FRAME, 1'd0, frame_code_r, dma_decimation_code_r, dma_ready_3ff};
    assign set_kp_cb_w = {CMD_SET_KP, 12'd0, kp_r};
    assign set_kd_cb_w = {CMD_SET_KD,12'd0, kd_r};
    assign set_ki_cb_w = {CMD_SET_KI, 12'd0, ki_r};
    assign set_dec_cb_w = {CMD_SET_DEC, 14'b0, dec_r};
    assign set_sp_cb_w = {CMD_SET_SP, 12'd0, {2{sp_r[13]}}, sp_r};
    assign set_alpha_sat_en_cb_w = {CMD_SET_ALPHA_SAT_EN, 18'b0, alpha_r, satwidth_r, pid_enable_r};
    assign config_io_cb_w = {CMD_CONFIG_IO, 23'b0, pid_sel_r, dac2_dat_sel_r, dac1_dat_sel_r};


    logic [AXI_GPIO_OUT_WIDTH-1 : 0] callback_r, next_callback_w;
    assign axi_to_ps_o = callback_r;

    logic [13:0] dac1_dat_r, next_dac1_dat_w, dac2_dat_r, next_dac2_dat_w;
    logic dac_sel_r, next_dac_sel_w;


    
/////////////////////// IO ROUTING ///////////////////////////////////////    

    typedef enum logic [2:0]
    {
        I_FEED_W = 3'b000,
        Q_FEED_W = 3'b001,
        SAT_A_16S = 3'b010,
        SAT_B_16S = 3'b011
        //SQUARED_SIGN_I = 3'b100, TODO: Implement these
        //SQUARED_SIGN_Q = 3'b101
    }   pid_sel_t;
    logic [2:0] pid_sel_r, next_pid_sel_w;

    typedef enum logic
    {
        SELECT_DAC = 1'b0,
        SELECT_PID = 1'b1
    }   dac_sel_t;

    logic dac1_dat_sel_r, next_dac1_dat_sel_w, dac2_dat_sel_r, next_dac2_dat_sel_w;
    assign dac_dat_o = dac_sel_r? ((dac2_dat_sel_r == SELECT_PID)? pid_out_w : dac2_dat_r) : ((dac1_dat_sel_r == SELECT_PID)? pid_out_w : dac1_dat_r);


    logic signed [15:0] pid_in_w;
    always_comb begin
        unique case(pid_sel_r)
            I_FEED_W: begin
                pid_in_w = i_feed_r;
            end

            Q_FEED_W: begin
                pid_in_w = q_feed_r;
            end
            
            SAT_A_16S: begin
                pid_in_w = adc_dat_a_16s_w;
            end 

            SAT_B_16S: begin
                pid_in_w = adc_dat_b_16s_w;
            end

            default begin
                pid_in_w = adc_dat_a_16s_w;
            end
        endcase
    end


    assign next_dac1_dat_sel_w = (cmd_w == CMD_CONFIG_IO)? data_w[0] : dac1_dat_sel_r;
    assign next_dac2_dat_sel_w = (cmd_w == CMD_CONFIG_IO)? data_w[1] : dac2_dat_sel_r;
    assign next_pid_sel_w = (cmd_w == CMD_CONFIG_IO)? data_w[4:2] : pid_sel_r;


///////////////////////////////////////////////////////////////////////////


    logic signed [15:0] err_tap_w, perr_tap_w, derr_tap_w, ierr_tap_w;
    logic signed [31:0] sum_err_tap_w;
    pid_core u_pid(
        .clk(clk),
        .rst(rst_sync_r),
        .kp_i(kp_r),
        .kd_i(kd_r),
        .ki_i(ki_r),
        .decimate_i(dec_r),
        .sp_i(sp_r),
        .alpha_i(alpha_r),
        .satwidth_i(satwidth_r),
        .dat_i(pid_in_w), //TODO: Move from I-feed to (I^2+Q^2)sign(I)
        .enable_i(pid_enable_r),
        .pid_out(pid_out_w),
        .err_tap(err_tap_w),
        .perr_tap(perr_tap_w),
        .derr_tap(derr_tap_w),
        .ierr_tap(ierr_tap_w),
        .sum_err_tap(sum_err_tap_w)
    );

    typedef enum logic [3:0]
    {
        ANGLES_AND_ESIGS = 4'b0000,
        PID_ERR_TAPS = 4'b0001,
        IO_SUM_ERR = 4'b0010
    }   frame_code_t;
    logic [3:0] frame_code_r, next_frame_code_w;

    always_comb begin
        unique case(frame_code_r)
            ANGLES_AND_ESIGS: dma_data_o = {i_feed_r, q_feed_r, cos_theta_r, sin_theta_r}; 
            PID_ERR_TAPS: dma_data_o = {err_tap_w, perr_tap_w, derr_tap_w, ierr_tap_w};
            IO_SUM_ERR: dma_data_o = {err_tap_w, 2'b0, pid_out_w, sum_err_tap_w};
            default: dma_data_o = {i_feed_r, q_feed_r, cos_theta_r, sin_theta_r}; 
        endcase
    end

    always_comb begin
        unique case(cmd_w)
            CMD_IDLE: begin
                next_callback_w = idle_cb_w;
            end

            CMD_SET_LED: begin
                next_callback_w = led_cb_w;
            end
            
            CMD_SET_DAC: begin
                next_callback_w = dac_cb_w;
            end

            CMD_GET_ADC: begin
                next_callback_w = adc_cb_w;
            end

            CMD_CHECK_SIGNED: begin
                next_callback_w = cs_cb_w;
            end

            CMD_SET_ROT_COEFFS: begin
                next_callback_w = set_rot_cb_w;
            end

            CMD_COMMIT_ROT_COEFFS: begin 
                next_callback_w = commit_rot_cb_w;
            end

            CMD_GET_FRAME: begin
                next_callback_w = get_frame_cb_w;
            end

            CMD_SET_KP: begin
                next_callback_w = set_kp_cb_w;
            end

            CMD_SET_KD: begin
                next_callback_w = set_kd_cb_w;
            end

            CMD_SET_KI: begin
                next_callback_w = set_ki_cb_w;
            end

            CMD_SET_DEC: begin
                next_callback_w = set_dec_cb_w;
            end

            CMD_SET_SP: begin
                next_callback_w = set_sp_cb_w;
            end

            CMD_SET_ALPHA_SAT_EN: begin
                next_callback_w = set_alpha_sat_en_cb_w;
            end

            CMD_CONFIG_IO: begin
                next_callback_w = config_io_cb_w;
            end

            default: begin
                next_callback_w = '0;
            end
        endcase
    end
    
    assign next_led_w = (cmd_w == CMD_SET_LED)? data_w[7:0] : led_r;
    assign next_dac1_dat_w = ((cmd_w == CMD_SET_DAC) && ~data_w[14])? data_w[13:0] : dac1_dat_r;
    assign next_dac2_dat_w = ((cmd_w == CMD_SET_DAC) && data_w[14])? data_w[13:0] : dac2_dat_r;
    assign next_sin_theta_w = ((cmd_w == CMD_SET_ROT_COEFFS) && data_w[16])? data_w[15:0] : sin_theta_r;
    assign next_cos_theta_w = ((cmd_w == CMD_SET_ROT_COEFFS) && ~data_w[16])? data_w[15:0] : cos_theta_r;
    assign next_rot_sin_theta_w = (cmd_w == CMD_COMMIT_ROT_COEFFS)? sin_theta_r : rot_sin_theta_r;
    assign next_rot_cos_theta_w = (cmd_w == CMD_COMMIT_ROT_COEFFS)? cos_theta_r : rot_cos_theta_r;
    assign next_dma_decimation_code_w = (cmd_w == CMD_GET_FRAME)? data_w[21:0] : dma_decimation_code_r;
    assign next_frame_code_w = (cmd_w == CMD_GET_FRAME)? data_w[25:22] : frame_code_r;
    assign kp_w = (cmd_w == CMD_SET_KP)? data_w[15:0] : kp_r;
    assign kd_w = (cmd_w == CMD_SET_KD)? data_w[15:0] : kd_r;
    assign ki_w = (cmd_w == CMD_SET_KI)? data_w[15:0] : ki_r;
    assign dec_w = (cmd_w == CMD_SET_DEC)? data_w[13:0] : dec_r;
    assign sp_w = (cmd_w == CMD_SET_SP)? data_w[13:0] : sp_r;
    assign alpha_w = (cmd_w == CMD_SET_ALPHA_SAT_EN)? data_w[9:6] : alpha_r;
    assign satwidth_w = (cmd_w == CMD_SET_ALPHA_SAT_EN)? data_w[5:1] : satwidth_r;
    assign pid_enable_w = (cmd_w == CMD_SET_ALPHA_SAT_EN)? data_w[0] : pid_enable_r;

//////////////////  IQ FEED LOGIC /////////////////////

    logic signed [15:0] adc_dat_a_16s_w, adc_dat_b_16s_w, adc_dat_a_16s_r, adc_dat_b_16s_r;
    logic signed [14:0] tmp_s_a, tmp_s_b;
    logic signed [32:0] i_rot_w, q_rot_w;
    logic signed [31:0] i_rot_sat_w, q_rot_sat_w;
    always_comb begin
        tmp_s_a = -($signed({1'b0, adc_dat_a_i}) - ADC_OFFSET);
        tmp_s_b = -($signed({1'b0, adc_dat_b_i}) - ADC_OFFSET);
        adc_dat_a_16s_w = {tmp_s_a[14], tmp_s_a};
        adc_dat_b_16s_w = {tmp_s_b[14], tmp_s_b};
    end

    always_comb begin
        i_rot_w = $signed(rot_cos_theta_r)*$signed(adc_dat_a_16s_r) - $signed(rot_sin_theta_r)*$signed(adc_dat_b_16s_r);
        q_rot_w = $signed(rot_sin_theta_r)*$signed(adc_dat_a_16s_r) + $signed(rot_cos_theta_r)*$signed(adc_dat_b_16s_r);
        
        i_rot_sat_w = i_rot_w[32] ^ i_rot_w[31]? (i_rot_w[31]? {1'b1, {31{1'b0}}} : {1'b0, {31{1'b1}}}) : i_rot_w[31:0];
        q_rot_sat_w = q_rot_w[32] ^ q_rot_w[31]? (q_rot_w[31]? {1'b1, {31{1'b0}}} : {1'b0, {31{1'b1}}}) : q_rot_w[31:0];

        i_feed_w = i_rot_sat_w >>> 15;
        q_feed_w = q_rot_sat_w >>> 15;
    end

////////////////////////////////////////////////////////

    assign led_o = led_r; 
    assign dac_rst_o = rst_sync_r;
    assign dma_decimation_code_o = dma_decimation_code_r;
   

///////////////////////// DMA ORCHESTRATOR ///////////////////////////

    logic dma_ready_edge_w, dma_ready_1ff, dma_ready_2ff, dma_ready_3ff;
    logic bram_ready_edge_w, bram_ready_r, bram_edge_acquired_r, next_bram_edge_acquired_w;

    posedge_detector u_dma_edge_detector(
       .D(dma_ready_3ff),
       .Q(dma_ready_edge_w),
       .rst(rst_sync_r),
       .clk(clk)
    );

    posedge_detector u_bram_edge_detector(
       .D(bram_ready_r),
       .Q(bram_ready_edge_w),
       .rst(rst_sync_r),
       .clk(clk)
    );


    logic dma_enable_r, next_dma_enable_w, bram_enable_r, next_bram_enable_w;


    typedef enum logic [1:0]
    {
        DMA_ARMED = 2'b00,
        DMA_RUN_BRAM = 2'b01,
        DMA_RUN_AXI = 2'b10,
        DMA_STALE = 2'b11
    }   dma_control_state_t;

    dma_control_state_t dma_state_r, next_dma_state_w;

    always_comb begin
        unique case(dma_state_r)
            DMA_ARMED: begin
                next_dma_state_w = bram_ready_r && (cmd_w == CMD_GET_FRAME)? DMA_RUN_BRAM : DMA_ARMED;
                next_dma_enable_w = 1'b0;
                next_bram_enable_w = 1'b0;
                next_bram_edge_acquired_w = 1'b0;
            end

            DMA_RUN_BRAM: begin
                next_dma_state_w = (bram_edge_acquired_r && dma_ready_3ff)? DMA_RUN_AXI : DMA_RUN_BRAM;
                next_dma_enable_w = 1'b0;
                next_bram_enable_w = 1'b1;
                next_bram_edge_acquired_w = bram_ready_edge_w? 1'b1 : bram_edge_acquired_r;
            end

            DMA_RUN_AXI: begin
                next_dma_state_w = dma_ready_edge_w? DMA_STALE : DMA_RUN_AXI;
                next_dma_enable_w = 1'b1;
                next_bram_enable_w = 1'b0;
                next_bram_edge_acquired_w = 1'b0;
            end

            DMA_STALE: begin
                next_dma_state_w = (cmd_w != CMD_GET_FRAME)? DMA_ARMED : DMA_STALE;
                next_dma_enable_w = 1'b0;
                next_bram_enable_w = 1'b0;
                next_bram_edge_acquired_w = 1'b0;
            end

            default: begin
                next_dma_state_w = DMA_ARMED;
                next_dma_enable_w = 1'b0;
                next_bram_enable_w = 1'b0;
                next_bram_edge_acquired_w = 1'b0;
            end
        endcase
    end



    assign dma_enable_o = dma_enable_r;
    assign bram_enable_o = bram_enable_r;



/////////////////////////////////////////////////////////////////////



    logic rst_sync_r, rst_pipe1_r, rst_sync_ne_r, rst_pipe1_ne_r;    

    always_ff @(posedge clk or posedge rst_i) begin
        if(rst_i) begin
            {rst_sync_r, rst_pipe1_r} <= {1'b1, 1'b1};
        end else begin
            {rst_sync_r, rst_pipe1_r} <= {rst_pipe1_r, 1'b0};
        end
    end


    always_ff @(posedge clk) begin
        if(rst_sync_r)begin
            {axi_3ff_r, axi_2ff_r, axi_1ff_r} <= '0;
            axi_from_ps_r <= 0;
            led_r <= 0;
            sin_theta_r <= 0; 
            cos_theta_r <= 16'sh7FFF;
            rot_sin_theta_r <= 0;
            rot_cos_theta_r <= 16'sh7FFF;
            adc_dat_a_16s_r <= '0;
            adc_dat_b_16s_r <= '0;
            i_feed_r <= '0;
            q_feed_r <= '0;

            dma_decimation_code_r <= 26'd1;
            frame_code_r <= ANGLES_AND_ESIGS;
            dma_enable_r <= 1'b0;
            bram_enable_r <= 1'b0;
            bram_edge_acquired_r <= 1'b0;

            {dma_ready_3ff, dma_ready_2ff, dma_ready_1ff} <= '0;
            bram_ready_r <= '0;
            dma_state_r <= DMA_ARMED;

            kp_r <= '0;
            kd_r <= '0;
            ki_r <= '0;
            dec_r <= 14'd1;
            sp_r <= 16'd0;
            alpha_r <= 4'd4;
            satwidth_r <= 5'd31;
            pid_enable_r <= 1'b0;
            
            dac1_dat_sel_r <= '0;
            dac2_dat_sel_r <= '0;
            pid_sel_r <= '0;

            callback_r <= 0;

        end else begin
            {axi_3ff_r, axi_2ff_r, axi_1ff_r} <= {axi_2ff_r, axi_1ff_r, axi_from_ps_i};
            axi_from_ps_r <= next_axi_from_ps_w;
            led_r <= next_led_w;
            sin_theta_r <= next_sin_theta_w; 
            cos_theta_r <= next_cos_theta_w;
            rot_sin_theta_r <= next_rot_sin_theta_w;
            rot_cos_theta_r <= next_rot_cos_theta_w;
            adc_dat_a_16s_r <= adc_dat_a_16s_w;
            adc_dat_b_16s_r <= adc_dat_b_16s_w;
            i_feed_r <= i_feed_w;
            q_feed_r <= q_feed_w;

            dma_decimation_code_r <= next_dma_decimation_code_w;
            frame_code_r <= next_frame_code_w;
            dma_enable_r <= next_dma_enable_w;
            bram_enable_r <= next_bram_enable_w;
            bram_edge_acquired_r <= next_bram_edge_acquired_w;

            {dma_ready_3ff, dma_ready_2ff, dma_ready_1ff} <= {dma_ready_2ff, dma_ready_1ff, dma_ready_i};
            bram_ready_r <= bram_ready_i;
            dma_state_r <= next_dma_state_w;

            kp_r <= kp_w;
            kd_r <= kd_w;
            ki_r <= ki_w;
            dec_r <= dec_w;
            sp_r <= sp_w;
            alpha_r <= alpha_w;
            satwidth_r <= satwidth_w;
            pid_enable_r <= pid_enable_w;

            dac1_dat_sel_r <= next_dac1_dat_sel_w;
            dac2_dat_sel_r <= next_dac2_dat_sel_w;
            pid_sel_r <= next_pid_sel_w;

            callback_r <= next_callback_w;
        end
    end
    
    assign next_dac_sel_w = ~dac_sel_r;

    always_ff @(negedge clk or posedge rst_i) begin
        if(rst_i) begin
            {rst_sync_ne_r, rst_pipe1_ne_r} <= {1'b1, 1'b1}; 
        end else begin
            {rst_sync_ne_r, rst_pipe1_ne_r} <= {rst_pipe1_ne_r, 1'b0};
        end
    end

    always_ff @(negedge clk) begin
        if(rst_sync_ne_r) begin
            dac_sel_r <= 1'b0;
            dac1_dat_r <= 14'h2000;
            dac2_dat_r <= 14'h2000;
        end else begin
            dac_sel_r <= next_dac_sel_w;
            dac1_dat_r <= next_dac1_dat_w; //next_dac1_dat_w;
            dac2_dat_r <= next_dac2_dat_w;
        end
    end

    assign dac_wrt_o = clk; //1'b1; //dac_wrt_r;
    assign dac_sel_o = dac_sel_r;
    //assign dac_dat_o = dac_sel_r? dac2_dat_r : dac1_dat_r; 

endmodule
