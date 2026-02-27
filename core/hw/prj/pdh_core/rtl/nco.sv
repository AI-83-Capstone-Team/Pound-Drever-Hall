module nco(
    input logic clk,
    input logic enable_i,
    input logic rst_i,
    input logic [11:0] stride_i,
    input logic [11:0] shift_i,
    input logic invert_i,
    input logic sub_i,
    output logic signed [15:0] out1_o,
    output logic signed [15:0] out2_o
);

    typedef enum logic [2:0]
    {
        ST_IDLE = 3'b000,
        ST_Q1 = 3'b001,
        ST_Q2 = 3'b010,
        ST_Q3 = 3'b011,
        ST_Q4 = 3'b100
    }   state_t;
    state_t state1_r, next_state1_w, state2_r, next_state2_w;

    logic [11:0] addr1_r, next_addr1_w, addr2_r, next_addr2_w;
    logic [15:0] rom1_lookup_w, rom2_lookup_w;
    logic signed [15:0] rom1_signed_w, rom1_signed_r, rom2_signed_w, rom2_signed_r;

    sine_qtr_rom u_qtr_rom(
        .clk(clk),
        .addr1(addr1_r),
        .addr2(addr2_r),
        .dout1(rom1_lookup_w),
        .dout2(rom2_lookup_w)
    );

    always_comb begin
        unique case(state1_r)
            ST_IDLE: begin
                next_addr1_w = 12'd0;
                next_state1_w = enable_i? ST_Q1 : ST_IDLE;
                rom1_signed_w = 16'sd0;
            end

            ST_Q1: begin
                next_addr1_w = (addr1_r + stride_i <= addr1_r)? 12'd4095 - (addr1_r + stride_i) : addr1_r + stride_i;
                next_state1_w = (addr1_r + stride_i <= addr1_r)? ST_Q2 : ST_Q1; 
                rom1_signed_w = rom1_lookup_w;
            end

            ST_Q2: begin
                next_addr1_w = (addr1_r - stride_i >= addr1_r)? 12'd4095 - (addr1_r - stride_i) : addr1_r - stride_i; 
                next_state1_w = (addr1_r - stride_i >= addr1_r)? ST_Q3 : ST_Q2;
                rom1_signed_w = rom1_lookup_w;
            end

            ST_Q3: begin
                next_addr1_w = (addr1_r + stride_i <= addr1_r)? 12'd4095 - (addr1_r + stride_i) : addr1_r + stride_i;
                next_state1_w = (addr1_r + stride_i <= addr1_r)? ST_Q4 : ST_Q3;
                rom1_signed_w = -rom1_lookup_w;
            end

            ST_Q4: begin
                next_addr1_w = (addr1_r - stride_i >= addr1_r)? 12'd4095 - (addr1_r - stride_i) : addr1_r - stride_i; 
                next_state1_w = (addr1_r - stride_i >= addr1_r)? ST_Q1 : ST_Q4;
                rom1_signed_w = -rom1_lookup_w;
            end

            default: begin
                next_addr1_w = 12'd0;
                next_state1_w = ST_IDLE;
                rom1_signed_w = 16'sd0;
            end
        endcase
    end


    always_comb begin
        unique case(state2_r)
            ST_IDLE: begin
                next_addr2_w = shift_i;
                next_state2_w = enable_i? (sub_i? ST_Q4 : ST_Q1) : ST_IDLE;
                rom2_signed_w = 16'sd0;
            end

            ST_Q1: begin
                next_addr2_w = (addr2_r + stride_i <= addr2_r)? 12'd4095 - (addr2_r + stride_i) : addr2_r + stride_i;
                next_state2_w = (addr2_r + stride_i <= addr2_r)? ST_Q2 : ST_Q1; 
                rom2_signed_w = invert_i? -rom2_lookup_w : rom2_lookup_w;
            end

            ST_Q2: begin
                next_addr2_w = (addr2_r - stride_i >= addr2_r)? 12'd4095 - (addr2_r - stride_i) : addr2_r - stride_i;
                next_state2_w = (addr2_r - stride_i >= addr2_r)? ST_Q3 : ST_Q2;
                rom2_signed_w = invert_i? -rom2_lookup_w : rom2_lookup_w;
            end

            ST_Q3: begin
                next_addr2_w = (addr2_r + stride_i <= addr2_r)? 12'd4095 - (addr2_r + stride_i) : addr2_r + stride_i;
                next_state2_w = (addr2_r + stride_i <= addr2_r)? ST_Q4 : ST_Q3;
                rom2_signed_w = invert_i? rom2_lookup_w : -rom2_lookup_w;
            end

            ST_Q4: begin
                next_addr2_w = (addr2_r - stride_i >= addr2_r)? 12'd4095 - (addr2_r - stride_i) : addr2_r - stride_i;
                next_state2_w = (addr2_r - stride_i >= addr2_r)? ST_Q1 : ST_Q4;
                rom2_signed_w = invert_i? rom2_lookup_w : -rom2_lookup_w;
            end

            default: begin
                next_addr2_w = 12'd0;
                next_state2_w = ST_IDLE;
                rom2_signed_w = 16'sd0;
            end
        endcase
    end


    always_ff @(posedge clk) begin
        if(rst_i) begin
            {state1_r, state2_r} <= {ST_IDLE, ST_IDLE};
            {addr1_r, addr2_r} <= {12'd0, 12'd0};
            {rom1_signed_r, rom2_signed_r} <= {16'sd0, 16'sd0};
        end else if(enable_i) begin
            {state1_r, state2_r} <= {next_state1_w, next_state2_w};
            {addr1_r, addr2_r} <= {next_addr1_w, next_addr2_w};
            {rom1_signed_r, rom2_signed_r} <= {rom1_signed_w, rom2_signed_w};
        end else begin
            {state1_r, state2_r} <= {ST_IDLE, ST_IDLE};
            {addr1_r, addr2_r} <= {12'd0, 12'd0};
            {rom1_signed_r, rom2_signed_r} <= {16'sd0, 16'sd0};
        end

    end 

    assign out1_o = rom1_signed_r;
    assign out2_o = rom2_signed_r;

endmodule
