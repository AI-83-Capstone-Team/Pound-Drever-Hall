module pid_core 
#(
    parameter int unsigned S32_W = 32;
    parameter int unsigned S16_W = 16;
    parameter int unsigned DEC_W = 14;
)
(
    input logic clk,
    input logic rst,
    input logic signed [S16_W-1:0] kp_i,
    input logic signed [S16_W-1:0] kd_i,
    input logic signed [S16_W-1:0] ki_i,
    input logic signed [S16_W-1:0] sp_i,
    input logic [3:0] alpha_i,

    input logic signed [S16_W-1:0] dat_i,

    input logic [DEC_W-1:0] decimate_i,

    input logic strobe_i,
    input logic enable_i,

    output logic [13:0] pid_out
);
    logic strobe_edge_w;
    posedge_detector strobe_pd
    (
        .clk(clk),
        .rst(rst),
        .D(strobe_i),
        .Q(strobe_edge_w)
    );


    logic signed [S16_W-1:0] kp_r, kd_r, ki_r, sp_r;
    logic [3:0]  alpha_r;
    logic [DEC_W-1:0] decimate_r;
    logic [DEC_W-1:0] cnt_r, next_cnt_w;
    logic tick1_w, tick2_r;
    always_comb begin
        next_cnt_w = (cnt_r == (decimate_r-1))? 0 : cnt_r + 1;
        tick1_w = enable_r && (cnt_r == 0);
    end


    function automatic logic signed [W1-1:0] sat_signed_from_signed #(
        int unsigned W1,
        int unsigned W2
    )(input logic signed [W2-1:0] x);
        if (W2 < W1) $fatal("sat_signed_from_signed: W2 must be >= W1");
        if(x > {{(W2-W1){1'b0}}, 1'b0, {(W1-1){1'b1}}}) sat_signed_from_signed = {1'b0, {(W1-1){1'b1}}};
        else if(x < {{(W2-W1){1'b1}}, 1'b1, {(W1-1){1'b0}}}) sat_signed_from_signed = {1'b1, {(W1-1){1'b0}}};
        else sat_signed_from_signed = x[W1-1:0];
    endfunction

    
    function automatic logic unsigned [W1-1:0] sat_unsigned_from_signed #(
        int unsigned W1,
        int unsigned W2
    )(input logic signed [W2-1:0] x);
        if (W2 < W1) $fatal("sat_unsigned_from_signed: W2 must be >= W1");
        if(x > $signed({{(W2-W1){1'b0}}, {W1{1'b1}}})) sat_unsigned_from_signed = {W1{1'b1}};
        else if(x[W2-1] == 1) sat_unsigned_from_signed = {W1{1'b0}};
        else sat_unsigned_from_signed = x[W1-1:0];
    endfunction



    logic signed [S16_W-1:0] error_w, error_pipe1_r;

    logic signed [S32_W-1:0] sum_error_w, sum_error_r;
    logic signed [S32_W:0] sum_error_wide_w;
    
    //EMA: y[k] = ax[k] + (1-a)y[k-1] where a = 2^(-alpha)
    logic signed [S16_W-1:0] yk_w, yk_r;

    logic signed [S32_W-1:0] p_error_r, p_error_w, d_error_r, d_error_w; //rshift by 15
    logic signed [(S16_W+S32_W)-1:0] i_error_r, i_error_w;  //rshift by 31

    logic signed [19:0] total_error_wide_w;

    always_comb begin
        error_w = dat_i - sp_r; //We dont worry about the overflow here because our core data feed is nominally a 14-bit signed int extended into s16 so there should be enough room at 16 bits as-is
        sum_error_wide_w = sum_error_r + error_w;
        sum_error_w = tick1_w? sat_signed_from_signed(.W1(S32_W), .W2(S32_W+1))(sum_error_wide_w) : sum_error_r; //Want to include last piped error in sum
        yk_w = tick1_w? ((error_w - yk_r)>>>alpha_r) + yk_r) : yk_r; 

        p_error_w = tick2_r? kp_r * error_pipe1_r : p_error_r;
        d_error_w = tick2_r? kd_r * yk_r : d_error_r;
        i_error_w = tick2_r? ki_r * sum_error_r : i_error_r;

        total_error_wide_w = (p_error_r>>>15) + (d_error_r>>>15) + (i_error_r>>>31) + 20'sd8192;
    end
    

    //assume rst, enable are synchrnous with feeder block (pdh_core)
    logic enable_r;
    always_ff @(posedge clk or posedge rst) begin
        if(rst) begin
            {kp_r, kd_r, ki_r, sp_r} <= '0; 
            alpha_r <= '0;
            decimate_r <= '1;
            sum_error_r <= '0;
            error_pipe1_r <= '0;
            yk_r <= '0;
            {p_error_r, d_error_r, i_error_r} <= '0;
            enable_r <= '0;
            cnt_r <= '0;
            tick2_r <= '0;
        end else if(enable_r) begin
            sum_error_r <= sum_error_w;
            error_pipe1_r <= error_w;
            yk_r <= yk_w;
            {p_error_r, d_error_r, i_error_r} <= {p_error_w, d_error_w, i_error_w};
            cnt_r <= next_cnt_w;
            tick2_r <= tick1_w;
        end else begin
            sum_error_r <= '0;
            error_pipe1_r <= '0;
            yk_r <= 0';
            {p_error_r, d_error_r, i_error_r} <= '0;
            cnt_r <= '0;
            tick2_r <= '0;
        end

        if(strobe_edge_w) begin
            enable_r <= enable_i;
            {kp_r, kd_r, ki_r, sp_r} <= {kp_i, kd_i, ki_i, sp_i};
            alpha_r <= alpha_i;
            decimate_r <= (decimate_i < 1)? 1 : decimate_i;
        end else begin
            enable_r <= enable_r;
            {kp_r, kd_r, ki_r, sp_r} <= {kp_r, kd_r, ki_r, sp_r};
            alpha_r <= alpha_r;
            decimate_r <= decimate_r;
        end
    end


    assign pid_out = sat_unsigned_from_signed #(.W1(14), .W2(20))(total_error_wide_w);


endmodule
