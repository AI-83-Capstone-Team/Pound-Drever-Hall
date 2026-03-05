`timescale 1ns/1ps

module tb_fir_tap;

    localparam DW = 16;

    int pass_count = 0;
    int fail_count = 0;

    task automatic check(
        input string          name,
        input logic signed [DW-1:0] got,
        input logic signed [DW-1:0] exp
    );
        if (got !== exp) begin
            $error("[FAIL] %s: got=%0d  exp=%0d", name, $signed(got), $signed(exp));
            fail_count++;
        end else begin
            $display("[PASS] %s: %0d", name, $signed(got));
            pass_count++;
        end
    endtask

    logic clk = 1'b0;
    always #4 clk = ~clk;

    logic rst, wren_i;
    logic signed [DW-1:0] din_i, coeff_in_i;
    logic signed [DW-1:0] din_pipe1_o, dout_o;

    fir_tap #(.DW(DW)) u_dut (
        .clk        (clk),
        .rst        (rst),
        .wren_i     (wren_i),
        .din_i      (din_i),
        .coeff_in_i (coeff_in_i),
        .din_pipe1_o(din_pipe1_o),
        .dout_o     (dout_o)
    );

    initial begin
        $dumpfile("dumps/tb_fir_tap.vcd");
        $dumpvars(0, tb_fir_tap);

        // ── Reset ──────────────────────────────────────────────────────────────
        rst = 1'b1; wren_i = 1'b0; din_i = '0; coeff_in_i = '0;
        #16;
        rst = 1'b0;
        #1;
        check("rst: dout_o",      dout_o,      16'sd0);
        check("rst: din_pipe1_o", din_pipe1_o, 16'sd0);

        // ── Load coeff=3, din=4 → dout=12 ────────────────────────────────────
        // Posedge 1: coeff_r ← 3 (via wren_i), out_r ← old_coeff*4 = 0
        // Posedge 2: out_r ← 3*4 = 12
        @(negedge clk); coeff_in_i = 16'sd3; din_i = 16'sd4; wren_i = 1'b1;
        @(posedge clk); #1; wren_i = 1'b0;
        @(posedge clk); #1;
        check("multiply: coeff=3 din=4 → 12", dout_o, 16'sd12);

        // ── din pipe passthrough ──────────────────────────────────────────────
        @(negedge clk); din_i = 16'sd7;
        @(posedge clk); #1;
        check("din_pipe: delay 1 cycle", din_pipe1_o, 16'sd7);

        // ── Positive saturation ───────────────────────────────────────────────
        // coeff=0x4000 (16384), din=2 → prod_wide = 32768 = 17'b0_1000...
        // in[16]=0, in[15]=1 → overflow → saturate to 0x7FFF
        @(negedge clk); coeff_in_i = 16'sh4000; din_i = 16'sd2; wren_i = 1'b1;
        @(posedge clk); #1; wren_i = 1'b0;
        @(posedge clk); #1;
        check("sat+: coeff=0x4000 din=2 → 0x7FFF", dout_o, 16'sh7FFF);

        // ── Negative output (no saturation) ──────────────────────────────────
        // coeff=-1, din=5 → prod=-5, fits in 17 bits, no saturation
        @(negedge clk); coeff_in_i = -16'sd1; din_i = 16'sd5; wren_i = 1'b1;
        @(posedge clk); #1; wren_i = 1'b0;
        @(posedge clk); #1;
        check("negative: coeff=-1 din=5 → -5", dout_o, -16'sd5);

        // ── Negative saturation ───────────────────────────────────────────────
        // coeff=0x8001 (-32767), din=2 → prod=-65534
        // 17-bit: 1_0000_0000_0000_0010 → in[16]=1, in[15]=0 → overflow → 0x8000
        @(negedge clk); coeff_in_i = 16'sh8001; din_i = 16'sd2; wren_i = 1'b1;
        @(posedge clk); #1; wren_i = 1'b0;
        @(posedge clk); #1;
        check("sat-: coeff=0x8001 din=2 → 0x8000", dout_o, 16'sh8000);

        // ── coeff=0 zeroes output ─────────────────────────────────────────────
        @(negedge clk); coeff_in_i = 16'sd0; din_i = 16'sd12345; wren_i = 1'b1;
        @(posedge clk); #1; wren_i = 1'b0;
        @(posedge clk); #1;
        check("zero coeff: any din → 0", dout_o, 16'sd0);

        #20;
        $display("\n%0d/%0d checks passed", pass_count, pass_count + fail_count);
        if (fail_count != 0) $fatal(1, "%0d check(s) FAILED", fail_count);
        $finish;
    end

endmodule
