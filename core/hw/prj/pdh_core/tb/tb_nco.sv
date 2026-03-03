`timescale 1ns/1ps

module tb_nco;

    logic clk = 1'b0;
    always #1 clk = ~clk;   // 500 MHz sim clock (2ns period). Change if you want.

    logic enable;
    logic [11:0] stride;
    logic [11:0] shift;
    logic invert;
    logic rst;
    logic sub;
    logic signed [15:0] out1, out2;

    nco dut (
        .clk(clk),
        .stride_i(stride),
        .enable_i(enable),
        .rst_i(rst),
        .shift_i(shift),
        .invert_i(invert),
        .out1_o(out1),
        .out2_o(out2),
        .sub_i(sub)
    );

    // Same conversion you use elsewhere
    function automatic logic [13:0] s16_to_u14(input logic signed [15:0] in);
        logic signed [15:0] t1;
        begin
            t1 = -(in >>> 2) + 16'sd8191;
            s16_to_u14 = t1[13:0];
        end
    endfunction

    logic [13:0] out1_conv, out2_conv;
    assign out1_conv = s16_to_u14(out1);
    assign out2_conv = s16_to_u14(out2);

    integer f;
    integer sample_count;

    // CSV logging: one line per posedge clk
    always_ff @(posedge clk) begin
        if (rst) begin
            sample_count <= 0;
        end else begin
            // If you only want samples while enabled, uncomment:
            // if (!enable) begin end else begin ... end

            $fwrite(f, "%0t,%0d,%0d,%0d,%0d,%0d\n",
                    $time,
                    sample_count,
                    out1, out2,
                    out1_conv, out2_conv);

            sample_count <= sample_count + 1;
        end
    end

    initial begin

        $dumpfile("dumps/tb_nco.vcd");
        $dumpvars(0, tb_nco);
               // init (avoid X)
        rst    = 1'b0;
        enable = 1'b0;
        sub    = 1'b1;
        invert = 1'b0;
        stride = 12'd262;
        shift  = 12'd1024;

        // open CSV
        f = $fopen("nco_out.csv", "w");
        if (f == 0) begin
            $fatal(1, "Failed to open nco_out.csv");
        end

        // header
        $fwrite(f, "time,sample,out1_s16,out2_s16,out1_u14,out2_u14\n");

        // reset + stimulus
        #4;
        rst = 1'b1;
        #2;
        rst = 1'b0;

        #100;
        enable = 1'b1;

        #80000;
        sub = 1'b1;
        #10;
        enable = 1'b0;
        #10;
        enable = 1'b1;

        #40000;
        enable = 1'b0;
        shift = 12'd4095;
        #10;
        enable = 1'b1;

        #40000;

        $fclose(f);
        $finish;
    end

endmodule
