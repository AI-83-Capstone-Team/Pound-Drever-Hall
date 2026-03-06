`timescale 1ns/1ps

module tb_bram_controller;

    logic clk = 1'b0;
    always #4 clk = ~clk;

    logic enable;
    logic [21:0] divcode;
    logic rst;

    bram_controller dut (
        .pdh_clk(clk),
        .decimation_code_i(divcode),
        .enable_i(enable),
        .rst_i(rst)
    );


    initial begin
        $dumpfile("dumps/tb_bram_controller.vcd");
        $dumpvars(0, tb_bram_controller);
        
        #4;
        rst = 1'b1;
        #2;
        rst = 1'b0;
        #100;
        divcode = 32'd3;
        enable = 1'b1;
        #800;
        $finish;
    end

endmodule
