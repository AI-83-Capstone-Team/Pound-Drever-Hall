`timescale 1ns/1ps

module tb_pid_controller;

    logic clk = 1'b0;
    always #4 clk = ~clk;
    logic rst;

    logic enable_i;
    logic strobe_i;
    logic signed [15:0] dat_i;
    
    logic signed [15:0] kp_i;
    logic signed [15:0] kd_i;
    logic signed [15:0] ki_i;
    logic signed [13:0] sp_i;
    logic [3:0] alpha_i;
    logic [4:0] satwidth_i;


    logic [13:0] decimate_i;

    pid_core u_pid (
        .clk(clk),
        .rst(rst),
        .enable_i(enable_i),
        .kp_i(kp_i),
        .kd_i(kd_i),
        .ki_i(ki_i),
        .alpha_i(alpha_i),
        .dat_i(dat_i),
        .sp_i(sp_i),
        .decimate_i(decimate_i),
        .satwidth_i(satwidth_i)
    );

    initial begin
        $dumpfile("dumps/tb_pid_controller.vcd");
        $dumpvars(0, tb_pid_controller);

        #4;
        rst= 1'b1;
        enable_i = 1'b0;
        #7;
        rst = 1'b0;
        #1;
        dat_i = 16'sd8192;
        kp_i = {2'b0, {14{1'b1}}};
        kd_i = {3'b0, {13{1'b1}}};
        ki_i =  {3'b0, {13{1'b1}}};
        alpha_i = 4'd2;
        enable_i = 1'b1;
        #4
        decimate_i = 14'd2; 

        #400;
        sp_i = 14'sd10;
        #4;

        #400;
        dat_i = {{3{1'b1}}, {13{1'b0}}};
        #400;

        sp_i = -14'sd10;
        #10;
        #400;
        satwidth_i = 5'd15;
        #1000;
        dat_i = 16'sd8192;
        #2000;
        
        rst= 1'b1;
        enable_i = 1'b0;
        #7;
        rst = 1'b0;
        #1;
        dat_i = 16'sd8192;

        satwidth_i = 5'd0;
        kp_i = {1'b0, {15{1'b1}}};
        sp_i = {14'sd0};
        kd_i = {3'b0, {13{1'b1}}};
        ki_i =  {3'b0, {13{1'b1}}};
        alpha_i = 4'd2;
        enable_i = 1'b1;
        #400;
        $finish;
    end

endmodule
