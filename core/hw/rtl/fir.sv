`timescale 1ns / 1ps

// Symmetric FIR filter with a registered, saturating adder tree.
//
// Coefficients are loaded into tap_coeffs[] one at a time via tap_addr_i / tap_coeff_i while
// tap_mem_write_en_i is high.  A single pulse on tap_chain_write_en_i simultaneously latches
// all tap_coeffs[] values into the fir_tap registers (atomic coefficient update).
//
// The adder tree has AW = ceil(log2(NTAPS)) registered pipeline stages.  Total latency from
// din_i to dout_o is (1 tap register) + AW (adder stages) cycles.
//
// NTAPS must be a power of 2 so the binary adder tree is complete.
module fir # 
(
    parameter NTAPS,
    parameter DW,
    parameter AW
)
(
    input logic clk,
    input logic rst,
    input logic [AW-1:0] tap_addr_i,
    input logic signed [DW-1:0] tap_coeff_i,
    input logic tap_mem_write_en_i, 
    input logic tap_chain_write_en_i,

    input logic signed [DW-1:0] din_i,
    output logic signed [DW-1:0] dout_o
);

    `ifndef NOSYNTH
        (* rom_style = "block" *) logic signed [DW-1:0] tap_coeffs [0:NTAPS-1];
    `else
        logic signed [DW-1:0] tap_coeffs [0:NTAPS-1];
    `endif


    always_ff @(posedge clk) begin
        if(tap_mem_write_en_i) tap_coeffs[tap_addr_i] <= tap_coeff_i;
    end

    logic signed [NTAPS:0][DW-1:0] din_pipe; // din_pipe[0]=din_i, din_pipe[i+1]=tap[i].din_pipe1_o...
    logic signed [NTAPS:0][DW-1:0] tap_out_bus;
    assign din_pipe[0] = din_i; 



    function automatic logic signed [DW-1:0] sat_add(
        input logic signed [DW-1:0] a,
        input logic signed [DW-1:0] b
    );
        logic signed [DW:0] sum_wide;
        begin
            sum_wide = {a[DW-1], a} + {b[DW-1], b};
            if (sum_wide[DW] ^ sum_wide[DW-1]) begin
                if (sum_wide[DW]) sat_add = {1'b1, {(DW-1){1'b0}}};
                else              sat_add = {1'b0, {(DW-1){1'b1}}};
            end else begin
                sat_add = sum_wide[DW-1:0];
            end
        end
    endfunction


    genvar i;
    generate
        for(i = 0; i < NTAPS; i++) begin : g_taps
            fir_tap # (
                .DW(DW)
            ) u_tap (
                .clk(clk),
                .rst(rst),
                .wren_i(tap_chain_write_en_i),
                .din_i(din_pipe[i]),
                .coeff_in_i(tap_coeffs[i]),
                .din_pipe1_o(din_pipe[i+1]),
                .dout_o(tap_out_bus[i])
            );
        end
    endgenerate

    logic signed [DW-1:0] stage_data [0:AW] [0:NTAPS-1];
    
    generate 
        for(i = 0; i < NTAPS; i++) begin : g_stage1
            assign stage_data[0][i] = tap_out_bus[i];
        end
    endgenerate

    
    genvar s;
    generate 
        for(s = 0; s < AW; s++) begin : g_stages
            for(i = 0; i < (NTAPS >> (s+1)); i++) begin : g_pairs
                always_ff @(posedge clk) begin
                    if(rst) begin
                        stage_data[s+1][i] <= '0;
                    end else begin
                        stage_data[s+1][i] <= sat_add(stage_data[s][2*i], stage_data[s][2*i+1]);
                    end
                end
            end
        end 
    endgenerate


    assign dout_o = stage_data[AW][0];
    
endmodule
