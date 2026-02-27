`timescale 1ns/1ps

module tb_nco;

    logic clk = 1'b0;
    always #1 clk = ~clk;

    logic enable;
    logic [11:0] stride = 12'd2;
    logic [11:0] shift = 12'd1024;
    logic invert = 1'b0;
    logic rst;
    logic sub = 1'b0;
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

    logic [13:0] out1_conv, out2_conv;

    function automatic logic [13:0] s16_to_u14(input logic signed [15:0] in);
        logic signed [15:0] t1 = -(in >> 2) + 16'sd8191;
        s16_to_u14 = t1[13:0];
    endfunction

    assign out1_conv = s16_to_u14(out1);
    assign out2_conv = s16_to_u14(out2);

    initial begin
        $dumpfile("dumps/tb_nco.vcd");
        $dumpvars(0, tb_bram_controller);
        
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
        $finish;
    end

endmodule
