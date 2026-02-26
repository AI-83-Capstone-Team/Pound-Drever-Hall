`timescale 1ns/1ps

module tb_nco;

    logic clk = 1'b0;
    always #1 clk = ~clk;

    logic enable;
    logic [11:0] stride = 12'd100;
    logic [11:0] shift = 12'd1024;
    logic invert = 1'b1;
    logic rst;

    nco dut (
        .clk(clk),
        .stride_i(stride),
        .enable_i(enable),
        .rst_i(rst),
        .shift_i(shift),
        .invert_i(invert)
    );



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
        $finish;
    end

endmodule
