`timescale 1 ns / 1 ps

module fir_tap #
(
    parameter DW
)
(
    input logic clk,
    input logic rst,
    input logic wren_i,
    input logic signed [DW-1:0] din_i,
    input logic signed [DW-1:0] coeff_in_i,

    output logic signed [DW-1:0] din_pipe1_o,
    output logic signed [DW-1:0] dout_o
);

    logic signed [DW-1:0] coeff_r, next_coeff_w;
    logic signed [DW-1:0] din_pipe1_r;


    assign next_coeff_w = wren_i? coeff_in_i : coeff_r;

    function automatic logic signed [DW-1:0] saturate_product (input logic signed [DW:0] in);
        if(in[DW] ^ in[DW-1]) begin
            if(in[DW] == 1'b1) saturate_product = {1'b1, {(DW-1){1'b0}}};
            else if(in[DW] == 1'b0) saturate_product = {1'b0, {(DW-1){1'b1}}};
        end else saturate_product = in[DW-1:0]; 
    endfunction
    
    logic signed [DW:0] prod_wide_w;
    assign prod_wide_w = coeff_r * din_i;
    
    logic signed [DW-1:0] out_w, out_r;
    assign out_w = saturate_product(prod_wide_w);

    always_ff @(posedge clk) begin
        if(rst) begin
            coeff_r <= '0;
            din_pipe1_r <= '0;
            out_r <= '0;
        end else begin
            coeff_r <= next_coeff_w;
            din_pipe1_r <= din_i;
            out_r <= out_w;
        end
    end

            
    assign dout_o = out_r;

    assign din_pipe1_o = din_pipe1_r;


endmodule

