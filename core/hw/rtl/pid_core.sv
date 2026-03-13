// Fixed-point PID controller with decimation, EMA noise filter, and integrator anti-windup.
//
// Error is computed as (dat_i - setpoint).  The proportional, derivative (approximate, via EMA),
// and integral terms are multiplied by Q15 coefficients Kp/Kd/Ki, shifted, and summed.
// The output is a 14-bit unsigned offset-binary value suitable for the DAC.
//
// Decimation divides the update rate: only every decimate_i-th sample triggers a PID update.
module pid_core
#(
    parameter int unsigned S32_W = 32,
    parameter int unsigned S16_W = 16,
    parameter int unsigned DEC_W = 14
)
(
    input logic clk,
    input logic rst,

    input logic signed [S16_W-1:0] kp_i,
    input logic signed [S16_W-1:0] kd_i,
    input logic signed [S16_W-1:0] ki_i,
    input logic [DEC_W-1:0] decimate_i,

    input logic signed [13:0] sp_i,
    input logic [3:0] alpha_i,
    input logic [4:0] satwidth_i, //assume shift range of 15 to 31

    input logic signed [S16_W-1:0] dat_i,


    input logic enable_i,
    input logic signed [S16_W-1:0] gain_i,   // Q10 output gain: gain_float = gain_i / 1024
    input logic signed [S16_W-1:0] egain_i,  // Q10 input gain applied to error before Kp/Ki/Kd
    input logic signed [13:0] bias_i,

    output logic [13:0] pid_out,

    output logic signed [15:0] err_tap,
    output logic signed [15:0] perr_tap,
    output logic signed [15:0] derr_tap,
    output logic signed [15:0] ierr_tap,
    output logic signed [31:0] sum_err_tap
);


    logic signed [S16_W-1:0] kp_r, kd_r, ki_r;
    logic signed [S16_W-1:0] gain_r;
    logic signed [S16_W-1:0] egain_r;
    logic signed [13:0] bias_r;
    logic signed [13:0] sp_r;
    logic [3:0] alpha_r;
    logic [4:0] satwidth_r;

    logic [DEC_W-1:0] decimate_r;
    logic [DEC_W-1:0] cnt_r, next_cnt_w;
    logic tick1_w, tick1_r, tick2_r;
    always_comb begin
        next_cnt_w = (cnt_r >= (decimate_r-1))? 0 : cnt_r + 1;
        tick1_w = enable_i && (cnt_r == 0);
    end

    localparam int unsigned W1S = S32_W;
    localparam int unsigned W2S = S32_W+1;
    localparam int unsigned W1U = 14;  // unsigned output width (DAC range)
    localparam int unsigned W2U = 20;  // signed input width to sat_unsigned_from_signed

    // Elaboration-time check: sat_unsigned_from_signed requires W2U >= W1U
    if (W2U < W1U) $fatal(1, "pid_core: sat_unsigned_from_signed: W2U (%0d) must be >= W1U (%0d)", W2U, W1U);

    // Saturate a (S32_W+1)-bit signed value to the range [-(threshold), threshold-1].
    function automatic logic signed [S32_W-1:0] apply_satwidth_truncation (input logic signed [S32_W:0] in, input logic signed [S32_W-1:0] threshold);
        if($signed(in) >= $signed({1'b0, threshold})) apply_satwidth_truncation = threshold-1;
        else if($signed(in) <= $signed({1'b1, -threshold})) apply_satwidth_truncation = -threshold + 1;
        else apply_satwidth_truncation = in[S32_W-1:0];
    endfunction

    // Saturate a W2U-bit signed value to [0, 2^W1U - 1] (unsigned W1U-bit output).
    function automatic logic unsigned [W1U-1:0] sat_unsigned_from_signed (input logic signed [W2U-1:0] x);
        if($signed(x) > $signed({{(W2U-W1U){1'b0}}, {W1U{1'b1}}})) sat_unsigned_from_signed = {W1U{1'b1}};
        else if(x[W2U-1] == 1) sat_unsigned_from_signed = {W1U{1'b0}};
        else sat_unsigned_from_signed = x[W1U-1:0];
    endfunction

    // Saturate a 26-bit signed value to 20-bit signed range.
    function automatic logic signed [19:0] sat20_from_26(input logic signed [25:0] x);
        if      ($signed(x) >  26'sd524287)  sat20_from_26 =  20'sd524287;  // 2^19 − 1
        else if ($signed(x) < -26'sd524288)  sat20_from_26 = -20'sd524288;  // −2^19
        else                                 sat20_from_26 = x[19:0];
    endfunction

    // Saturate a 22-bit signed value to 16-bit signed range.
    function automatic logic signed [15:0] sat16_from_22(input logic signed [21:0] x);
        if      ($signed(x) >  22'sd32767)  sat16_from_22 =  16'sd32767;
        else if ($signed(x) < -22'sd32768)  sat16_from_22 = -16'sd32768;
        else                                sat16_from_22 = x[15:0];
    endfunction



    logic signed [S16_W-1:0] error_w, error_pipe1_r;
    logic signed [31:0] egain_prod_w;
    logic signed [21:0] egain_shifted_w;
    logic signed [S16_W-1:0] error_gained_w;
    assign egain_prod_w    = $signed(error_w) * $signed(egain_r);
    assign egain_shifted_w = $signed(egain_prod_w[31:10]);  // >>> 10 for Q10 scaling
    assign error_gained_w  = sat16_from_22(egain_shifted_w);

    logic signed [S32_W-1:0] sum_error1_w, sum_error2_w, sum_error_r;
    logic [S32_W-1:0] sat_threshold_w;

    logic signed [S32_W:0] sum_error_wide_w;
    
    logic signed [S16_W-1:0] yk_w, yk_r;

    logic signed [S32_W-1:0] p_error_r, p_error_w, d_error_r, d_error_w; //rshift by 15
    logic signed [(S16_W+S32_W)-1:0] i_error_r, i_error_w;  //rshift by 31

    logic signed [19:0] total_error_wide_w;

    logic signed [S16_W-1:0] p_error_shifted_w, d_error_shifted_w, i_error_shifted_w;
    // Isolated to 16-bit context so Verilator doesn't propagate the 32-bit d_error context
    // down into the subtraction operands.
    logic signed [S16_W-1:0] err_minus_ema_w;

    always_comb begin
        // Explicit sign-extend sp_r (14-bit Q13) to match dat_i (16-bit) before subtracting.
        // The core data feed is nominally 14-bit signed extended to 16-bit, so no overflow here.
        error_w = dat_i - {{2{sp_r[13]}}, sp_r};
        sat_threshold_w = 1 << satwidth_r;

        sum_error_wide_w = $signed({sum_error_r[S32_W-1], sum_error_r}) + $signed({{(S32_W-S16_W + 1){error_gained_w[S16_W-1]}}, error_gained_w});
        sum_error1_w = apply_satwidth_truncation(.in(sum_error_wide_w), .threshold(sat_threshold_w));
        sum_error2_w = (tick1_r && !(pid_out == {14{1'b1}} && sum_error1_w > sum_error_r) && !(pid_out == 14'd0 && sum_error1_w < sum_error_r))? sum_error1_w : sum_error_r; //Want to include last piped error in sum

        // EMA: y[k] = α·x[k] + (1−α)·y[k−1]  where α = 2^(−alpha)
        err_minus_ema_w = error_gained_w - yk_r;
        yk_w = tick1_r? (((err_minus_ema_w)>>>alpha_r) + yk_r) : yk_r;

        p_error_w = tick2_r? kp_r * error_pipe1_r : p_error_r;
        d_error_w = tick2_r? kd_r * err_minus_ema_w : d_error_r;
        i_error_w = tick2_r? ki_r * sum_error_r : i_error_r;

        // Q30 → Q15 for P and D (shift by 15 = S16_W-1 fractional bits, take upper 16 bits).
        p_error_shifted_w = p_error_r[(S32_W-2):(S16_W-1)];
        d_error_shifted_w = d_error_r[(S32_W-2):(S16_W-1)];
        // Integral shift is variable (anti-windup saturation width); cast to 16-bit result.
        i_error_shifted_w = 16'($signed(i_error_r) >>> satwidth_r);

        // Explicit sign-extension from 16-bit to 20-bit before summing to avoid overflow.
        total_error_wide_w = {{4{p_error_shifted_w[S16_W-1]}}, p_error_shifted_w}
                           + {{4{d_error_shifted_w[S16_W-1]}}, d_error_shifted_w}
                           + {{4{i_error_shifted_w[S16_W-1]}}, i_error_shifted_w};
    end

    logic [4:0] next_satwidth_w;
    // satwidth_i is 5-bit unsigned (0..31); values below 15 are clamped to the maximum (31).
    assign next_satwidth_w = (satwidth_i >= 5'd15) ? satwidth_i : 5'd31;

    //assume rst, enable are synchronous with feeder block (pdh_core)
    always_ff @(posedge clk) begin
        if(rst) begin
            {kp_r, kd_r, ki_r, sp_r} <= '0;
            alpha_r <= '0;
            decimate_r <= {{(DEC_W-1){1'b0}}, 1'b1};
            satwidth_r <= 5'd31;
            gain_r <= 16'sd1024;   // power-on default: gain = 1.0
            egain_r <= 16'sd1024;  // power-on default: egain = 1.0
            bias_r <= '0;

            sum_error_r <= '0;
            error_pipe1_r <= '0;
            yk_r <= '0;
            {p_error_r, d_error_r, i_error_r} <= '0;
            cnt_r <= '0;
            {tick2_r, tick1_r} <= {1'b0, 1'b0};
        end else if(enable_i) begin
            {kp_r, kd_r, ki_r, sp_r} <= {kp_i, kd_i, ki_i, sp_i};
            alpha_r <= alpha_i;
            decimate_r <= (decimate_i < 1)? 1 : decimate_i;
            satwidth_r <= next_satwidth_w;
            gain_r <= gain_i;
            egain_r <= egain_i;
            bias_r <= bias_i;

            sum_error_r <= sum_error2_w;
            error_pipe1_r <= error_gained_w;
            yk_r <= yk_w;
            {p_error_r, d_error_r, i_error_r} <= {p_error_w, d_error_w, i_error_w};
            cnt_r <= next_cnt_w;
            {tick2_r, tick1_r} <= {tick1_r, tick1_w};
        end else begin
            {kp_r, kd_r, ki_r, sp_r} <= {kp_r, kd_r, ki_r, sp_r};
            alpha_r <= alpha_r;
            decimate_r <= decimate_r;
            satwidth_r <= satwidth_r;
            gain_r <= gain_r;
            egain_r <= egain_r;
            bias_r <= bias_r;

            sum_error_r <= '0;
            error_pipe1_r <= '0;
            yk_r <= '0;
            {p_error_r, d_error_r, i_error_r} <= '0;
            cnt_r <= '0;
            {tick2_r, tick1_r} <= {1'b0, 1'b0};
        end
    end


    // Apply Q10 output gain to the signed PID sum before the DAC offset.
    logic signed [35:0] gain_prod_w;
    logic signed [25:0] gain_shifted_w;
    logic signed [19:0] gained_wide_w;
    assign gain_prod_w    = $signed(total_error_wide_w) * $signed(gain_r);
    assign gain_shifted_w = $signed(gain_prod_w[35:10]);  // >>> 10 for Q10 scaling
    assign gained_wide_w  = sat20_from_26(gain_shifted_w);
    assign pid_out = sat_unsigned_from_signed(gained_wide_w + 20'sd8191 + {{6{bias_r[13]}}, bias_r});

    assign err_tap = error_pipe1_r;
    assign perr_tap = p_error_shifted_w;
    assign derr_tap = d_error_shifted_w;
    assign ierr_tap = i_error_shifted_w;
    assign sum_err_tap = sum_error2_w;
    

endmodule
