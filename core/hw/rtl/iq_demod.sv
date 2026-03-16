`timescale 1 ns / 1 ps

// IQ Demodulator — instantaneous product of two 16-bit signed signals.
//
// ref_i and in_i are treated as Q15 (±1.0 scale).  The 32-bit signed product
// is arithmetic-right-shifted by 15 to produce a Q15 output, matching the
// convention used for i_feed/q_feed in pdh_core.sv.
//
// A 16 × 16 signed multiply cannot overflow a 32-bit signed result, so no
// saturation is needed before the shift.  Output is registered to keep it
// synchronous with the rest of the pipeline.

module iq_demod (
    input  logic clk,
    input  logic rst,
    input  logic signed [15:0] ref_i,
    input  logic signed [15:0] in_i,
    output logic signed [15:0] out_o
);

    logic signed [31:0] product_w;

    always_comb begin
        product_w = $signed(ref_i) * $signed(in_i);
    end

    always_ff @(posedge clk) begin
        if (rst)
            out_o <= '0;
        else
            out_o <= product_w[30:15];
    end

endmodule
