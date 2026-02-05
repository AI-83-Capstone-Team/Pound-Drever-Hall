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

    output logic [13:0] pid_out
);


    logic signed [S16_W-1:0] kp_r, kd_r, ki_r;
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
    localparam int unsigned W1U = 14;
    localparam int unsigned W2U = 20;


    function automatic logic signed [S32_W-1:0] apply_satwidth_truncation (input logic signed [S32_W:0] in, input logic signed [S32_W-1:0] threshold);
        if($signed(in) >= $signed({1'b0, threshold})) apply_satwidth_truncation = threshold-1;
        else if($signed(in) <= $signed({1'b1, -threshold})) apply_satwidth_truncation = -threshold + 1;
        else apply_satwidth_truncation = in[S32_W-1:0];
    endfunction
    
    function automatic logic unsigned [W1U-1:0] sat_unsigned_from_signed (input logic signed [W2U-1:0] x);
        if (W2U < W1U) $fatal("sat_unsigned_from_signed: W2 must be >= W1");
        if($signed(x) > $signed({{(W2U-W1U){1'b0}}, {W1U{1'b1}}})) sat_unsigned_from_signed = {W1U{1'b1}};
        else if(x[W2U-1] == 1) sat_unsigned_from_signed = {W1U{1'b0}};
        else sat_unsigned_from_signed = x[W1U-1:0];
    endfunction



    logic signed [S16_W-1:0] error_w, error_pipe1_r;

    logic signed [S32_W-1:0] sum_error1_w, sum_error2_w, sum_error_r;
    logic [S32_W-1:0] sat_threshold_w;

    logic signed [S32_W:0] sum_error_wide_w;
    
    logic signed [S16_W-1:0] yk_w, yk_r;

    logic signed [S32_W-1:0] p_error_r, p_error_w, d_error_r, d_error_w; //rshift by 15
    logic signed [(S16_W+S32_W)-1:0] i_error_r, i_error_w;  //rshift by 31

    logic signed [19:0] total_error_wide_w;

    logic signed [S16_W-1:0] p_error_shifted_w, d_error_shifted_w, i_error_shifted_w;


    always_comb begin
        error_w = dat_i - sp_r; //We dont worry about the overflow here because our core data feed is nominally a 14-bit signed int extended into s16 so there should be enough room at 16 bits as-is
        sat_threshold_w = 1 << satwidth_r;
        
        sum_error_wide_w = $signed({sum_error_r[S32_W-1], sum_error_r}) + $signed({{(S32_W-S16_W + 1){error_w[S16_W-1]}}, error_w});
        sum_error1_w = apply_satwidth_truncation(.in(sum_error_wide_w), .threshold(sat_threshold_w));
        sum_error2_w = (tick1_r && !(pid_out == {14{1'b1}} && sum_error1_w > sum_error_r) && !(pid_out == 14'd0 && sum_error1_w < sum_error_r))? sum_error1_w : sum_error_r; //Want to include last piped error in sum

        //EMA: y[k] = ax[k] + (1-a)y[k-1] where a = 2^(-alpha)
        yk_w = tick1_r? (((error_w - yk_r)>>>alpha_r) + yk_r) : yk_r; 

        p_error_w = tick2_r? kp_r * error_pipe1_r : p_error_r;
        d_error_w = tick2_r? kd_r * (error_w - yk_r) : d_error_r;
        i_error_w = tick2_r? ki_r * sum_error_r : i_error_r;

        p_error_shifted_w = p_error_r>>>15;
        d_error_shifted_w = d_error_r>>>15;
        i_error_shifted_w = i_error_r>>>satwidth_r;

        total_error_wide_w = p_error_shifted_w + d_error_shifted_w + i_error_shifted_w;
    end
    
    logic [4:0] next_satwidth_w;
    assign next_satwidth_w = ((satwidth_i <= 5'd31) && (satwidth_i >= 5'd15))? satwidth_i : 5'd31;

    //assume rst, enable are synchronous with feeder block (pdh_core)
    always_ff @(posedge clk or posedge rst) begin
        if(rst) begin
            {kp_r, kd_r, ki_r, sp_r} <= '0; 
            alpha_r <= '0;
            decimate_r <= {{(DEC_W-1){1'b0}}, 1'b1};
            satwidth_r <= 5'd31;

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

            sum_error_r <= sum_error2_w;
            error_pipe1_r <= error_w;
            yk_r <= yk_w;
            {p_error_r, d_error_r, i_error_r} <= {p_error_w, d_error_w, i_error_w};
            cnt_r <= next_cnt_w;
            {tick2_r, tick1_r} <= {tick1_r, tick1_w};
        end else begin
            {kp_r, kd_r, ki_r, sp_r} <= {kp_r, kd_r, ki_r, sp_r};
            alpha_r <= alpha_r;
            decimate_r <= decimate_r;
            satwidth_r <= next_satwidth_w;

            sum_error_r <= '0;
            error_pipe1_r <= '0;
            yk_r <= '0;
            {p_error_r, d_error_r, i_error_r} <= '0;
            cnt_r <= '0;
            {tick2_r, tick1_r} <= {1'b0, 1'b0};
        end
    end


    assign pid_out = sat_unsigned_from_signed(total_error_wide_w + 20'sd8191);


endmodule
