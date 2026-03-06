`timescale 1ns/1ps

// NTAPS must be a power of two (AW = $clog2(NTAPS) reduction stages).
module tb_fir;

    localparam NTAPS = 4;
    localparam DW    = 16;
    localparam AW    = $clog2(NTAPS);  // 2

    // Pipeline latency: 1 (tap multiply FF) + AW (adder tree FFs) = 3 cycles.
    // Steady-state settling (constant input, all taps loaded):
    //   tap[3] sees din_i after 3 din_pipe delays + 1 tap FF = 4 cycles.
    //   Then 2 more adder stages = 6 cycles total. Use 15 for margin.
    localparam SETTLE = 15;

    int pass_count = 0;
    int fail_count = 0;

    task automatic check(
        input string         name,
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

    logic                 rst;
    logic [AW-1:0]        tap_addr_i;
    logic signed [DW-1:0] tap_coeff_i;
    logic                 tap_mem_write_en_i;
    logic                 write_tap_chain_i;
    logic signed [DW-1:0] din_i;
    logic signed [DW-1:0] dout_o;

    fir #(.NTAPS(NTAPS), .DW(DW)) u_dut (
        .clk               (clk),
        .rst               (rst),
        .tap_addr_i        (tap_addr_i),
        .tap_coeff_i       (tap_coeff_i),
        .tap_mem_write_en_i(tap_mem_write_en_i),
        .write_tap_chain_i (write_tap_chain_i),
        .din_i             (din_i),
        .dout_o            (dout_o)
    );

    // Write one entry into tap_coeffs[] memory.
    task automatic write_tap_coeff(
        input [AW-1:0]        addr,
        input signed [DW-1:0] coeff
    );
        @(negedge clk);
        tap_addr_i         = addr;
        tap_coeff_i        = coeff;
        tap_mem_write_en_i = 1'b1;
        @(posedge clk); #1;
        tap_mem_write_en_i = 1'b0;
    endtask

    // Pulse write_tap_chain_i for one cycle to latch tap_coeffs → each tap's coeff_r.
    task automatic latch_tap_chain();
        @(negedge clk);
        write_tap_chain_i = 1'b1;
        @(posedge clk); #1;
        write_tap_chain_i = 1'b0;
    endtask

    initial begin
        $dumpfile("dumps/tb_fir.vcd");
        $dumpvars(0, tb_fir);

        // ── Reset ──────────────────────────────────────────────────────────────
        rst                = 1'b1;
        tap_addr_i         = '0;
        tap_coeff_i        = '0;
        tap_mem_write_en_i = 1'b0;
        write_tap_chain_i  = 1'b0;
        din_i              = '0;
        #16;
        rst = 1'b0;
        #1;
        check("rst: dout_o cleared", dout_o, 16'sd0);

        // ── All coeff=1, constant input=4 → steady-state dout=16 ─────────────
        // Each tap computes 1*4=4; NTAPS=4 taps sum to 16 via the adder tree.
        write_tap_coeff(0, 16'sd1);
        write_tap_coeff(1, 16'sd1);
        write_tap_coeff(2, 16'sd1);
        write_tap_coeff(3, 16'sd1);
        latch_tap_chain();

        @(negedge clk); din_i = 16'sd4;
        repeat(SETTLE) @(posedge clk);
        #1;
        check("steady-state: NTAPS*coeff*din = 4*1*4 = 16", dout_o, 16'sd16);

        // ── Input returns to 0 → dout returns to 0 ────────────────────────────
        @(negedge clk); din_i = 16'sd0;
        repeat(SETTLE) @(posedge clk);
        #1;
        check("drain: din=0 → dout=0", dout_o, 16'sd0);

        // ── Weighted taps: coeff=[1,2,3,4], constant din=1 → dout=10 ──────────
        // sum = 1+2+3+4 = 10
        write_tap_coeff(0, 16'sd1);
        write_tap_coeff(1, 16'sd2);
        write_tap_coeff(2, 16'sd3);
        write_tap_coeff(3, 16'sd4);
        latch_tap_chain();

        @(negedge clk); din_i = 16'sd1;
        repeat(SETTLE) @(posedge clk);
        #1;
        check("weighted: coeff=[1,2,3,4] din=1 → 10", dout_o, 16'sd10);

        // ── Zero coefficients silence output ──────────────────────────────────
        write_tap_coeff(0, 16'sd0);
        write_tap_coeff(1, 16'sd0);
        write_tap_coeff(2, 16'sd0);
        write_tap_coeff(3, 16'sd0);
        latch_tap_chain();

        @(negedge clk); din_i = 16'sd32767;
        repeat(SETTLE) @(posedge clk);
        #1;
        check("zero coeff: any din → 0", dout_o, 16'sd0);

        #20;
        $display("\n%0d/%0d checks passed", pass_count, pass_count + fail_count);
        if (fail_count != 0) $fatal(1, "%0d check(s) FAILED", fail_count);
        $finish;
    end

endmodule
