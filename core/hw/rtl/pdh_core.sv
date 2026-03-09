`timescale 1 ns / 1 ps

// PDH Controller Core.
//
// Decodes 32-bit commands arriving via the PS AXI GPIO register, drives the NCO, FIR, and
// PID submodules, and orchestrates BRAM capture + DMA transfer frames.
//
// Command protocol: every command is strobed in twice (0→1) by the PS.  On each rising strobe
// edge the core latches the command word, executes it, and posts a 32-bit callback to the PS.
//
// TODO: Add FIFO between NCO/PID and DMA feed

module pdh_core #
(
    parameter int unsigned ADC_DATA_WIDTH    = 14,  // ADC sample width — offset binary
    parameter int unsigned DAC_DATA_WIDTH    = 14,  // DAC output width — offset binary
    parameter int unsigned AXI_GPIO_IN_WIDTH  = 32, // PS→PL GPIO register width
    parameter int unsigned AXI_GPIO_OUT_WIDTH = 32, // PL→PS callback register width
    parameter int unsigned NTAPS             = 32,  // FIR tap count (must be a power of 2)
    parameter int unsigned AW               = $clog2(NTAPS) // FIR coefficient address width
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
    localparam int unsigned CMD_BITS    = 4;

    // AXI command word layout: [31]=rst, [30]=strobe, [29:26]=cmd, [25:0]=data
    localparam int unsigned CMD_END    = 29;
    localparam int unsigned CMD_START  = 26;
    localparam int unsigned DATA_END   = 25;
    localparam int unsigned DATA_START = 0;

    // ADC uses offset-binary encoding; subtract the midpoint to get a signed centered value.
    // Negated because the hardware drives the ADC input low for positive optical power.
    localparam logic signed [ADC_DATA_WIDTH:0] ADC_OFFSET = (ADC_DATA_WIDTH+1)'(1 << (ADC_DATA_WIDTH - 1));

    // DAC midpoint in unsigned offset-binary: 0 V output = 2^(DAC_DATA_WIDTH-1)
    localparam logic [DAC_DATA_WIDTH-1:0] DAC_INIT = DAC_DATA_WIDTH'(1 << (DAC_DATA_WIDTH - 1));

    // NCO-to-DAC conversion bias: maps Q15 ±1.0 to [0, 2^DAC_DATA_WIDTH-1]
    // formula: u14 = -(q15 >> 2) + (2^(DAC_DATA_WIDTH-1) - 1)
    localparam logic signed [15:0] NCO_DAC_BIAS = 16'($signed({1'b0, DAC_INIT}) - 1);

    // Q15 fixed-point +1.0 (maximum positive value); used as identity rotation coefficient
    localparam logic signed [15:0] Q15_MAX = 16'sh7FFF;

    // PID controller power-on reset defaults
    localparam int unsigned PID_DEC_INIT      = 1;   // update every sample (no decimation)
    localparam int unsigned PID_ALPHA_INIT    = 4;   // EMA smoothing shift: α = 2^(-4)
    localparam int unsigned PID_SATWIDTH_INIT = 31;  // integrator saturation at ±2^31
    localparam int unsigned PID_GAIN_INIT     = 1024; // gain = 1.0 in Q10

    // NCO power-on reset default
    localparam int unsigned NCO_STRIDE_INIT = 1;     // minimum stride ≈ 7629 Hz

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
        CMD_SET_PID_COEFFS = 4'b1000,
        CMD_SET_NCO = 4'b1001,
        CMD_SET_FIR = 4'b1010,
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
    logic signed [15:0] gain_r, gain_w;
    logic signed [13:0] bias_r, bias_w;
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
        set_pid_cb_w,
        set_nco_cb_w,
        config_io_cb_w,
        set_fir_cb_w;

        
    assign idle_cb_w    = 32'b0;
    assign led_cb_w     = {CMD_SET_LED, 20'd0, led_r};
    assign dac_cb_w     = {CMD_SET_DAC, dac1_dat_r, dac2_dat_r};
    assign adc_cb_w     = {CMD_GET_ADC, adc_dat_b_i, adc_dat_a_i};

    logic [15:0] base_bus;
    always_comb begin
        base_bus = {CMD_CHECK_SIGNED, 7'd0, axi_from_ps_r[4:0]}; 
        case(axi_from_ps_r[4:0])
            5'b00000: cs_cb_w = {base_bus, adc_dat_a_16s_r};
           
            5'b00001: cs_cb_w = {base_bus, adc_dat_b_16s_r};

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

            5'b10000: cs_cb_w = {base_bus, 7'b0, pid_sel_r, dac2_dat_sel_r, dac1_dat_sel_r};

            5'b10001: cs_cb_w = {base_bus, 13'b0, dac1_dat_sel_r};

            5'b10010: cs_cb_w = {base_bus, 13'b0, dac2_dat_sel_r};

            5'b10011: cs_cb_w = {base_bus, 13'b0, pid_sel_r};

            default: cs_cb_w = {base_bus, 16'd0};
            
        endcase
    end

    logic [21:0] dma_decimation_code_r, next_dma_decimation_code_w; //TODO: Add this as CS option


    typedef enum logic [3:0]
    {
        PID_SELECT_KP    = 4'b0000,
        PID_SELECT_KD    = 4'b0001,
        PID_SELECT_KI    = 4'b0010,
        PID_SELECT_DEC   = 4'b0011,
        PID_SELECT_SP    = 4'b0100,
        PID_SELECT_ALPHA = 4'b0101,
        PID_SELECT_SAT   = 4'b0110,
        PID_SELECT_EN    = 4'b0111,
        PID_SELECT_GAIN  = 4'b1000,
        PID_SELECT_BIAS  = 4'b1001
    }   pid_coeff_sel_t;


    logic [3:0] pid_coeff_select_w;
    assign pid_coeff_select_w = data_w[19:16];


    logic [15:0] pid_payload_w;

    always_comb begin
        case(pid_coeff_select_w)
            PID_SELECT_KP: begin
                pid_payload_w = kp_r;
            end

            PID_SELECT_KD: begin
                pid_payload_w = kd_r;
            end

            PID_SELECT_KI: begin
                pid_payload_w = ki_r;
            end

            PID_SELECT_SP: begin
                pid_payload_w = {{2{sp_r[13]}}, sp_r};
            end

            PID_SELECT_DEC: begin
                pid_payload_w = {2'b0, dec_r};
            end

            PID_SELECT_ALPHA: begin
                pid_payload_w = {12'b0, alpha_r};
            end
            PID_SELECT_SAT: begin
                pid_payload_w = {11'b0, satwidth_r};
            end
            PID_SELECT_EN: begin
                pid_payload_w = {15'b0, pid_enable_r};
            end

            PID_SELECT_GAIN: begin
                pid_payload_w = gain_r;
            end

            PID_SELECT_BIAS: begin
                pid_payload_w = 16'($signed(-bias_r));
            end

            default: begin
                pid_payload_w = 16'd0;
            end
        endcase
    end




    assign set_rot_cb_w = {CMD_SET_ROT_COEFFS, sin_theta_r[15:2], cos_theta_r[15:2]};
    assign commit_rot_cb_w = {CMD_COMMIT_ROT_COEFFS, q_feed_r[15:2], i_feed_r[15:2]};
    assign get_frame_cb_w = {CMD_GET_FRAME, 1'd0, frame_code_r, dma_decimation_code_r, dma_ready_3ff};
    assign set_pid_cb_w = {CMD_SET_PID_COEFFS, 8'd0, pid_coeff_select_w, pid_payload_w};
    assign config_io_cb_w = {CMD_CONFIG_IO, 19'b0, pid_sel_r, dac2_dat_sel_r, dac1_dat_sel_r};


    logic [AXI_GPIO_OUT_WIDTH-1 : 0] callback_r, next_callback_w;
    assign axi_to_ps_o = callback_r;

    logic [13:0] dac1_dat_r, next_dac1_dat_w, dac2_dat_r, next_dac2_dat_w;
    logic dac_sel_r, next_dac_sel_w;



//////////////////////////// NCO BLOCK /////////////////////////////////
    logic nco_en_r, nco_en_w;
    logic nco_inv_r, nco_inv_w;
    logic nco_sub_r, nco_sub_w;
    logic [11:0] nco_stride_r, nco_stride_w, nco_shift_r, nco_shift_w; 
    logic signed [15:0] nco_out1_w, nco_out2_w, nco_out1_r, nco_out2_r;
    logic [13:0] nco_feed1_r, nco_feed2_r, nco_feed1_w, nco_feed2_w;

    logic [11:0] addr1_w, addr2_w; 
    logic [13:0] phi1_w, phi2_w;

    nco u_nco(
        .clk(clk),
        .enable_i(nco_en_r),
        .rst_i(rst_sync_r),
        .stride_i(nco_stride_r),
        .shift_i(nco_shift_r),
        .invert_i(nco_inv_r),
        .sub_i(nco_sub_r),
        .out1_o(nco_out1_w),
        .out2_o(nco_out2_w),
        .addr1_o(addr1_w),
        .addr2_o(addr2_w),
        .phi1_o(phi1_w),
        .phi2_o(phi2_w)
    );


    assign set_nco_cb_w = {CMD_SET_NCO, 1'd0, nco_stride_r, nco_shift_r, nco_sub_r, nco_inv_r, nco_en_r};


    // Convert a Q15 signed sample (e.g. NCO output) to 14-bit unsigned offset-binary for the DAC.
    // The NCO outputs ±1.0 full-scale; attenuate by 4 and bias to DAC midpoint.
    function automatic logic [13:0] s16_to_u14(input logic signed [15:0] in);
        logic signed [15:0] t1 = -(in >> 2) + NCO_DAC_BIAS;
        s16_to_u14 = t1[13:0];
    endfunction


    assign nco_feed1_w = s16_to_u14(nco_out1_r);
    assign nco_feed2_w = s16_to_u14(nco_out2_r);




////////////////////// FIR BLOCK /////////////////////////////////////////
// Programmable NTAPS-tap FIR filter with saturation adder tree.
// NTAPS and AW are module parameters; defaults: 32 taps, AW = ceil(log2(NTAPS)) = 5.

logic signed [15:0] tap_coeff_r, tap_coeff_w;
logic [AW-1:0] tap_addr_r, tap_addr_w;
logic tap_mem_write_en_r, tap_mem_write_en_w;
logic tap_chain_write_en_r, tap_chain_write_en_w;

logic signed [15:0] fir_in_w, fir_out_w;


typedef enum logic [2:0]
{
    FIR_SELECT_ADDR = 3'b000,
    FIR_SELECT_COEFF = 3'b001,
    FIR_SELECT_INPUT_SEL = 3'b010,
    FIR_SELECT_MEM_WRITE_EN = 3'b011,
    FIR_SELECT_CHAIN_WRITE_EN = 3'b100
}   fir_update_sel_t;
logic [2:0] fir_update_sel_w;
assign fir_update_sel_w = data_w[18:16];


typedef enum logic [2:0]
{
    ADC1 = 3'b000,
    ADC2 = 3'b001,
    I_FEED = 3'b010,
    Q_FEED = 3'b011
}   fir_input_sel_t;
logic [2:0] fir_input_sel_r, fir_input_sel_w;



always_comb begin
    unique case(fir_input_sel_r)
        ADC1:    fir_in_w = adc_dat_a_16s_r;
        ADC2:    fir_in_w = adc_dat_b_16s_r;
        I_FEED:  fir_in_w = i_feed_r;
        Q_FEED:  fir_in_w = q_feed_r;
        default: fir_in_w = adc_dat_a_16s_r;
    endcase
end


assign tap_addr_w = (cmd_w == CMD_SET_FIR && fir_update_sel_w == FIR_SELECT_ADDR)? data_w[AW-1:0] : tap_addr_r;
assign tap_coeff_w = (cmd_w == CMD_SET_FIR && fir_update_sel_w == FIR_SELECT_COEFF)? data_w[15:0] : tap_coeff_r;
assign fir_input_sel_w = (cmd_w == CMD_SET_FIR && fir_update_sel_w == FIR_SELECT_INPUT_SEL)? data_w[2:0] : fir_input_sel_r;
assign tap_mem_write_en_w = (cmd_w == CMD_SET_FIR && fir_update_sel_w == FIR_SELECT_MEM_WRITE_EN)? data_w[0] : tap_mem_write_en_r;
assign tap_chain_write_en_w = (cmd_w == CMD_SET_FIR && fir_update_sel_w == FIR_SELECT_CHAIN_WRITE_EN)? data_w[0] : tap_chain_write_en_r;
logic [15:0] fir_payload_w;
always_comb begin
    case(fir_update_sel_w)
        FIR_SELECT_ADDR:           fir_payload_w = {{(16-AW){1'b0}}, tap_addr_r};
        FIR_SELECT_COEFF:          fir_payload_w = tap_coeff_r;
        FIR_SELECT_INPUT_SEL:      fir_payload_w = {13'b0, fir_input_sel_r};
        FIR_SELECT_MEM_WRITE_EN:   fir_payload_w = {15'b0, tap_mem_write_en_r};
        FIR_SELECT_CHAIN_WRITE_EN: fir_payload_w = {15'b0, tap_chain_write_en_r};
        default:                   fir_payload_w = 16'd0;
    endcase
end
assign set_fir_cb_w = {CMD_SET_FIR, 9'd0, fir_update_sel_w, fir_payload_w};

fir # (
    .NTAPS(NTAPS),
    .DW(16),
    .AW(AW)
) u_fir (
    .clk(clk),
    .rst(rst_sync_r),
    .tap_addr_i(tap_addr_r),
    .tap_coeff_i(tap_coeff_r),
    .tap_mem_write_en_i(tap_mem_write_en_r),
    .tap_chain_write_en_i(tap_chain_write_en_r),

    .din_i(fir_in_w),
    .dout_o(fir_out_w)
); 














    
/////////////////////// IO ROUTING ///////////////////////////////////////    

    typedef enum logic [2:0]
    {
        I_FEED_W = 3'b000,
        Q_FEED_W = 3'b001,
        SAT_A_16S = 3'b010,
        SAT_B_16S = 3'b011,
        FIR_OUT_W = 3'b100
    }   pid_sel_t;
    logic [2:0] pid_sel_r, next_pid_sel_w;

    typedef enum logic [2:0]
    {
        SELECT_DAC = 3'b000,
        SELECT_PID = 3'b001,
        SELECT_NCO_1 = 3'b010,
        SELECT_NCO_2 = 3'b011
    }   dac_sel_t;

    logic [2:0] dac1_dat_sel_r, next_dac1_dat_sel_w, dac2_dat_sel_r, next_dac2_dat_sel_w;

    logic [13:0] dac1_feed_w, dac2_feed_w;

    always_comb begin
        unique case(dac1_dat_sel_r)
            SELECT_DAC:   dac1_feed_w = dac1_dat_r;
            SELECT_PID:   dac1_feed_w = pid_out_w;
            SELECT_NCO_1: dac1_feed_w = nco_feed1_r;
            SELECT_NCO_2: dac1_feed_w = nco_feed2_r;
            default:      dac1_feed_w = DAC_INIT;
        endcase
    end

    always_comb begin
        unique case(dac2_dat_sel_r)
            SELECT_DAC:   dac2_feed_w = dac2_dat_r;
            SELECT_PID:   dac2_feed_w = pid_out_w;
            SELECT_NCO_1: dac2_feed_w = nco_feed1_r;
            SELECT_NCO_2: dac2_feed_w = nco_feed2_r;
            default:      dac2_feed_w = DAC_INIT;
        endcase
    end

    assign dac_dat_o = dac_sel_r? dac2_feed_w : dac1_feed_w;

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
                pid_in_w = adc_dat_a_16s_r;
            end 

            SAT_B_16S: begin
                pid_in_w = adc_dat_b_16s_r;
            end

            FIR_OUT_W: begin
                pid_in_w = fir_out_w;
            end

            default begin
                pid_in_w = adc_dat_a_16s_r;
            end
        endcase
    end


    assign next_dac1_dat_sel_w = (cmd_w == CMD_CONFIG_IO)? data_w[2:0] : dac1_dat_sel_r;
    assign next_dac2_dat_sel_w = (cmd_w == CMD_CONFIG_IO)? data_w[5:3] : dac2_dat_sel_r;
    assign next_pid_sel_w = (cmd_w == CMD_CONFIG_IO)? data_w[8:6] : pid_sel_r;


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
        .gain_i(gain_r),
        .bias_i(bias_r),
        .pid_out(pid_out_w),
        .err_tap(err_tap_w),
        .perr_tap(perr_tap_w),
        .derr_tap(derr_tap_w),
        .ierr_tap(ierr_tap_w),
        .sum_err_tap(sum_err_tap_w)
    );

    typedef enum logic [3:0]
    {
        ADC_DATA_IN = 4'b0000,
        PID_ERR_TAPS = 4'b0001,
        IO_SUM_ERR = 4'b0010,
        OSC_INSPECT = 4'b0011,
        OSC_ADDR_CHECK = 4'b0100,
        LOOPBACK = 4'b0101,
        FIR_IO = 4'b0110
    }   frame_code_t;
    logic [3:0] frame_code_r, next_frame_code_w;

    logic [63:0] dma_data_r, dma_data_w;
    always_comb begin
        unique case(frame_code_r)
            ADC_DATA_IN: dma_data_w = {i_feed_r, q_feed_r, adc_dat_a_16s_r, adc_dat_b_16s_r};
            PID_ERR_TAPS: dma_data_w = {err_tap_w, perr_tap_w, derr_tap_w, ierr_tap_w};
            IO_SUM_ERR: dma_data_w = {err_tap_w, 2'b0, pid_out_w, sum_err_tap_w};
            OSC_INSPECT: dma_data_w = {2'b0, nco_feed2_r, 2'b0, nco_feed1_r, nco_out2_r, nco_out1_r};
            OSC_ADDR_CHECK : dma_data_w = {4'b0, addr2_w, 4'b0, addr1_w, 2'b0, phi2_w, 2'b0, phi1_w};
            LOOPBACK: dma_data_w = {2'b0, dac1_feed_w, 2'b0, dac2_feed_w, 2'b0, adc_dat_a_i, 2'b0, adc_dat_b_i};
            FIR_IO: dma_data_w = {32'b0, fir_out_w, fir_in_w};
            default: dma_data_w = {i_feed_r, q_feed_r, adc_dat_a_16s_r, adc_dat_b_16s_r};
        endcase
    end

    always_comb begin
        case(cmd_w)
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

            CMD_SET_PID_COEFFS: begin
                next_callback_w = set_pid_cb_w;
            end

            CMD_SET_NCO: begin
                next_callback_w = set_nco_cb_w;
            end

            CMD_CONFIG_IO: begin
                next_callback_w = config_io_cb_w;
            end

            CMD_SET_FIR: begin
                next_callback_w = set_fir_cb_w;
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

    assign kp_w        = (cmd_w == CMD_SET_PID_COEFFS && (pid_coeff_select_w == PID_SELECT_KP))    ? data_w[15:0] : kp_r;
    assign kd_w        = (cmd_w == CMD_SET_PID_COEFFS && (pid_coeff_select_w == PID_SELECT_KD))    ? data_w[15:0] : kd_r;
    assign ki_w        = (cmd_w == CMD_SET_PID_COEFFS && (pid_coeff_select_w == PID_SELECT_KI))    ? data_w[15:0] : ki_r;
    assign dec_w       = (cmd_w == CMD_SET_PID_COEFFS && (pid_coeff_select_w == PID_SELECT_DEC))   ? data_w[13:0] : dec_r;
    assign sp_w        = (cmd_w == CMD_SET_PID_COEFFS && (pid_coeff_select_w == PID_SELECT_SP))    ? data_w[13:0] : sp_r;
    assign alpha_w     = (cmd_w == CMD_SET_PID_COEFFS && (pid_coeff_select_w == PID_SELECT_ALPHA)) ? data_w[3:0]  : alpha_r;
    assign satwidth_w  = (cmd_w == CMD_SET_PID_COEFFS && (pid_coeff_select_w == PID_SELECT_SAT))   ? data_w[4:0]  : satwidth_r;
    assign pid_enable_w= (cmd_w == CMD_SET_PID_COEFFS && (pid_coeff_select_w == PID_SELECT_EN))    ? data_w[0]    : pid_enable_r;
    assign gain_w      = (cmd_w == CMD_SET_PID_COEFFS && (pid_coeff_select_w == PID_SELECT_GAIN))  ? data_w[15:0] : gain_r;
    assign bias_w      = (cmd_w == CMD_SET_PID_COEFFS && (pid_coeff_select_w == PID_SELECT_BIAS))  ? -$signed(data_w[13:0]) : bias_r;


    typedef enum logic [2:0]
    {
        NCO_SELECT_STRIDE = 3'b000,
        NCO_SELECT_SHIFT  = 3'b001,
        NCO_SELECT_INV    = 3'b010,
        NCO_SELECT_SUB    = 3'b011,
        NCO_SELECT_EN     = 3'b100
    }   nco_sel_t;

    logic [2:0] nco_coeff_select_w;
    assign nco_coeff_select_w = data_w[18:16];

    assign nco_stride_w = (cmd_w == CMD_SET_NCO && (nco_coeff_select_w == NCO_SELECT_STRIDE))? data_w[11:0] : nco_stride_r;
    assign nco_shift_w  = (cmd_w == CMD_SET_NCO && (nco_coeff_select_w == NCO_SELECT_SHIFT)) ? data_w[11:0] : nco_shift_r;
    assign nco_inv_w    = (cmd_w == CMD_SET_NCO && (nco_coeff_select_w == NCO_SELECT_INV))   ? data_w[0]    : nco_inv_r;
    assign nco_sub_w    = (cmd_w == CMD_SET_NCO && (nco_coeff_select_w == NCO_SELECT_SUB))   ? data_w[0]    : nco_sub_r;
    assign nco_en_w     = (cmd_w == CMD_SET_NCO && (nco_coeff_select_w == NCO_SELECT_EN))    ? data_w[0]    : nco_en_r;

//////////////////  IQ FEED LOGIC /////////////////////
// ADC samples arrive as unsigned offset-binary.  Each is converted to signed, centred, and
// negated (hardware inversion), then fed through a 2×2 rotation matrix to produce the I/Q
// demodulation outputs.  Rotation coefficients are in Q15 format.

    logic signed [15:0] adc_dat_a_16s_w, adc_dat_b_16s_w, adc_dat_a_16s_r, adc_dat_b_16s_r;
    logic signed [14:0] tmp_s_a, tmp_s_b;
    logic signed [32:0] i_rot_w, q_rot_w;
    logic signed [31:0] i_rot_sat_w, q_rot_sat_w;
    always_comb begin
        // Convert offset-binary to signed: negate so that increasing optical power gives positive error.
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

        // The rotation products are Q30 (Q15 × Q15); arithmetic-right-shift by 15 gives Q15.
        // Explicitly select bits [30:15] to make the truncation intent clear to the linter.
        i_feed_w = i_rot_sat_w[30:15];
        q_feed_w = q_rot_sat_w[30:15];
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
            cos_theta_r <= Q15_MAX;     // identity rotation: cos(0) = 1.0 in Q15
            rot_sin_theta_r <= 0;
            rot_cos_theta_r <= Q15_MAX;
            adc_dat_a_16s_r <= '0;
            adc_dat_b_16s_r <= '0;
            i_feed_r <= '0;
            q_feed_r <= '0;

            dma_decimation_code_r <= 22'd1;
            frame_code_r <= ADC_DATA_IN;
            dma_enable_r <= 1'b0;
            bram_enable_r <= 1'b0;
            bram_edge_acquired_r <= 1'b0;

            {dma_ready_3ff, dma_ready_2ff, dma_ready_1ff} <= '0;
            bram_ready_r <= '0;
            dma_state_r <= DMA_ARMED;

            kp_r <= '0;
            kd_r <= '0;
            ki_r <= '0;
            dec_r <= 14'(PID_DEC_INIT);
            sp_r <= 14'd0;
            alpha_r <= 4'(PID_ALPHA_INIT);
            satwidth_r <= 5'(PID_SATWIDTH_INIT);
            pid_enable_r <= 1'b0;
            gain_r <= 16'(PID_GAIN_INIT);
            bias_r <= '0;

            nco_shift_r <= '0;
            nco_stride_r <= 12'(NCO_STRIDE_INIT);
            nco_en_r <= 1'b0;
            nco_inv_r <= 1'b0;
            nco_sub_r <= 1'b0;
            nco_out1_r <= '0;
            nco_out1_r <= '0;
            nco_feed1_r <= '0;
            nco_feed2_r <= '0;
            
            dac1_dat_sel_r      <= '0;
            dac2_dat_sel_r      <= '0;
            pid_sel_r           <= '0;

            tap_addr_r          <= '0;
            tap_coeff_r         <= '0;
            tap_mem_write_en_r  <= 1'b0;
            tap_chain_write_en_r<= 1'b0;
            fir_input_sel_r     <= '0;

            dma_data_r <= '0;

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
            gain_r <= gain_w;
            bias_r <= bias_w;

            nco_shift_r <= nco_shift_w;
            nco_stride_r <= nco_stride_w;
            nco_en_r <= nco_en_w;
            nco_inv_r <= nco_inv_w;
            nco_sub_r <= nco_sub_w;
            nco_out1_r <= nco_out1_w;
            nco_out2_r <= nco_out2_w;
            nco_feed1_r <= nco_feed1_w;
            nco_feed2_r <= nco_feed2_w;

            dac1_dat_sel_r      <= next_dac1_dat_sel_w;
            dac2_dat_sel_r      <= next_dac2_dat_sel_w;
            pid_sel_r           <= next_pid_sel_w;

            tap_addr_r          <= tap_addr_w;
            tap_coeff_r         <= tap_coeff_w;
            tap_mem_write_en_r  <= tap_mem_write_en_w;
            tap_chain_write_en_r<= tap_chain_write_en_w;
            fir_input_sel_r     <= fir_input_sel_w;

            dma_data_r <= dma_data_w;

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
            dac1_dat_r <= DAC_INIT;
            dac2_dat_r <= DAC_INIT;
        end else begin
            dac_sel_r <= next_dac_sel_w;
            dac1_dat_r <= next_dac1_dat_w; //next_dac1_dat_w;
            dac2_dat_r <= next_dac2_dat_w;
        end
    end

    assign dac_wrt_o = clk; //1'b1; //dac_wrt_r;
    assign dac_sel_o = dac_sel_r;
    //assign dac_dat_o = dac_sel_r? dac2_dat_r : dac1_dat_r; 
    //

    assign dma_data_o = dma_data_r;

endmodule
