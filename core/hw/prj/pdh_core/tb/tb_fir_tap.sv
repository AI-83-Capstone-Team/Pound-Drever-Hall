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

        // ── Q15 multiply: 0.5 × 0.5 = 0.25 ──────────────────────────────────
        // coeff=0x4000 (0.5 Q15), din=0x4000 (0.5 Q15)
        // 32-bit product = 0x10000000; bits[30:15] = 0x2000 (0.25 Q15)
        @(negedge clk); coeff_in_i = 16'sh4000; din_i = 16'sh4000; wren_i = 1'b1;
        @(posedge clk); #1; wren_i = 1'b0;
        @(posedge clk); #1;
        check("Q15 mul: 0.5*0.5=0.25", dout_o, 16'sh2000);

        // ── din pipe passthrough ──────────────────────────────────────────────
        @(negedge clk); din_i = 16'sd7;
        @(posedge clk); #1;
        check("din_pipe: delay 1 cycle", din_pipe1_o, 16'sd7);

        // ── Q15 multiply: -0.5 × 0.5 = -0.25 ────────────────────────────────
        // coeff=0xC000 (-0.5 Q15), din=0x4000 (0.5 Q15)
        // 32-bit product = 0xF0000000; bits[30:15] = 0xE000 (-0.25 Q15)
        @(negedge clk); coeff_in_i = 16'shC000; din_i = 16'sh4000; wren_i = 1'b1;
        @(posedge clk); #1; wren_i = 1'b0;
        @(posedge clk); #1;
        check("Q15 mul: -0.5*0.5=-0.25", dout_o, 16'shE000);

        // ── Positive saturation: (-1.0) × (-1.0) = +1.0 → 0x7FFF ────────────
        // coeff=0x8000 (-1.0 Q15), din=0x8000 (-1.0 Q15)
        // 32-bit product = 0x40000000; bits[31]=0, bits[30]=1 → overflow → 0x7FFF
        @(negedge clk); coeff_in_i = 16'sh8000; din_i = 16'sh8000; wren_i = 1'b1;
        @(posedge clk); #1; wren_i = 1'b0;
        @(posedge clk); #1;
        check("sat+: (-1.0)*(-1.0) → 0x7FFF", dout_o, 16'sh7FFF);

        // ── coeff=0 zeroes output ─────────────────────────────────────────────
        @(negedge clk); coeff_in_i = 16'sd0; din_i = 16'sh4000; wren_i = 1'b1;
        @(posedge clk); #1; wren_i = 1'b0;
        @(posedge clk); #1;
        check("zero coeff: any din → 0", dout_o, 16'sd0);

        #20;
        $display("\n%0d/%0d checks passed", pass_count, pass_count + fail_count);
        if (fail_count != 0) $fatal(1, "%0d check(s) FAILED", fail_count);
        $finish;
    end

endmodule
