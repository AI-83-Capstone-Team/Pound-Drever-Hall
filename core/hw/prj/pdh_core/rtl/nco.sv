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

    typedef enum logic [1:0]
    {
        ST_Q1 = 2'b00,
        ST_Q2 = 2'b01,
        ST_Q3 = 2'b10,
        ST_Q4 = 2'b11
    }   state_t;


    logic [13:0] phi1_r, next_phi1_w, phi2_w;
    logic [11:0] addr1_w, addr2_w;
    logic [1:0] state1_w, state2_w;

    logic [15:0] rom1_lookup_w, rom2_lookup_w;
    logic signed [15:0] rom1_signed_w, rom1_signed_r, rom2_intermediate_w, rom2_signed_w, rom2_signed_r;

    sine_qtr_rom u_qtr_rom(
        .clk(clk),
        .addr1(addr1_w),
        .addr2(addr2_w),
        .dout1(rom1_lookup_w),
        .dout2(rom2_lookup_w)
    );

    assign next_phi1_w = phi1_r + {2'b0, stride_i};
    assign phi2_w = sub_i? phi1_r - {2'b0, shift_i} : phi1_r + {2'b0, shift_i}; 

    assign state1_w = phi1_r[13:12];
    assign state2_w = phi2_w[13:12];

    assign addr1_w = (state1_w == ST_Q1 || state1_w == ST_Q3)? phi1_r[11:0] : 12'd4095 - phi1_r[11:0];
    assign addr2_w = (state2_w == ST_Q1 || state2_w == ST_Q3)? phi2_w[11:0] : 12'd4095 - phi2_w[11:0];

    assign rom1_signed_w = (state1_w == ST_Q1 || state1_w == ST_Q2)? rom1_lookup_w : -rom1_lookup_w;
    assign rom2_intermediate_w = (state2_w == ST_Q1 || state2_w == ST_Q2)? rom2_lookup_w : -rom2_lookup_w;
    assign rom2_signed_w = invert_i? -rom2_intermediate_w : rom2_intermediate_w;


    always_ff @(posedge clk) begin
        if(rst_i) begin
            phi1_r <= 14'b0;
            {rom1_signed_r, rom2_signed_r} <= {16'sd0, 16'sd0};
        end else if(enable_i) begin
            phi1_r <= next_phi1_w;
            {rom1_signed_r, rom2_signed_r} <= {rom1_signed_w, rom2_signed_w};
        end else begin
            phi1_r <= 14'b0;
            {rom1_signed_r, rom2_signed_r} <= {16'sd0, 16'sd0};
        end
    end 

    assign out1_o = rom1_signed_r;
    assign out2_o = rom2_signed_r;

endmodule
