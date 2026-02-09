`timescale 1ns/1ps

module tb_pdh_core;

    logic clk = 1'b0;
    always #4 clk = ~clk;
    
    logic [31:0] axi_from_ps_i;
    logic [31:0] axi_to_ps_o;
    logic [7:0]  led_o;
    
    logic [31:0] S_AXIS_tdata_i = 32'h0;
    logic        S_AXIS_tvalid_i = 1'b0;
    logic [13:0] dac_dat_o;
    logic        dac_wrt_o;


    typedef enum logic [3:0] 
    {
        CMD_IDLE = 4'b0000,
        CMD_SET_LED = 4'b0001,
        CMD_SET_DAC = 4'b0010,
        CMD_GET_ADC = 4'b0011,
        CMD_CHECK_SIGNED = 4'b0100,
        CMD_SET_ROT_COEFFS = 4'b0101,
        CMD_COMMIT_ROT_COEFFS = 4'b0110,
        CMD_GET_FRAME = 4'b0111,
        CMD_SET_KP = 4'b1000,
        CMD_SET_KD = 4'b1001,
        CMD_SET_KI = 4'b1010,
        CMD_SET_DEC = 4'b1011,
        CMD_SET_SP = 4'b1100,
        CMD_SET_ALPHA_SAT_EN = 4'b1101
    } cmd_t;


    logic [13:0] adc_dat_a_w;
    pdh_core dut (
      .clk(clk),
      .axi_from_ps_i(axi_from_ps_i),
      .axi_to_ps_o(axi_to_ps_o),
      .led_o(led_o),
      .dac_dat_o(dac_dat_o),
      .dac_wrt_o(dac_wrt_o),
      .adc_dat_a_i(adc_dat_a_w) //dont worry about rotating in this tb for now given that its been verified to work on FPGA
    );

        
    function automatic [31:0] make_word(
        input logic        rst,      
        input logic        strobe,
        input logic [3:0]  cmd,      
        input logic [25:0] data 
    );
        logic [31:0] w;
        begin
            w = 32'h0;
            w[31]    = rst;
            w[30]    = strobe;
            w[29:26] = cmd;
            w[25:0]  = data;
            return w;
        end
    endfunction


    task automatic hold_word(input [31:0] w, input int cycles);
        begin
            axi_from_ps_i = w;
            repeat (cycles) @(posedge clk);
        end
    endtask


    task automatic send_cmd_two_step(input logic [3:0] cmd, input logic [25:0] data);
        begin
            hold_word(make_word(1'b0, 1'b0, cmd, data), 5);
            hold_word(make_word(1'b0, 1'b1, cmd, data), 5);
            hold_word(make_word(1'b0, 1'b0, cmd, data), 5);
        end
    endtask


    function automatic [25:0] make_dac_payload(
        input logic dac_enable,
        input logic dac_sel,          // 0 -> lower 14b, 1 -> upper 14b
        input logic [13:0] code
    );
        logic [25:0] p;
        begin
            p = 25'd0;
            p[15] = dac_enable;
            p[14] = dac_sel;
            p[13:0] = code;
            return p;
        end
    endfunction


    task automatic expect_dac_word(input logic [27:0] exp, input string tag="");
        begin
            #20;
            if (dac_dat_o === exp)
                $display("PASS: %s dac_dat_o = 0x%08h", tag, dac_dat_o);
            else
                $display("FAIL: %s dac_dat_o = 0x%08h (expected 0x%08h)", tag, dac_dat_o, exp);
        end
    endtask


    task automatic send_set_dac_two_step(input logic dac_sel, input logic [13:0] code);
        logic [25:0] payload;
        begin
            payload = make_dac_payload(.dac_sel(dac_sel), .code(code), .dac_enable(1'b1));
            send_cmd_two_step(CMD_SET_DAC, payload);
        end
    endtask


    task automatic config_pid(
        input logic signed [15:0] kp, 
        input logic signed [15:0] kd, 
        input logic signed [15:0] ki, 
        input logic [13:0] dec, 
        input logic signed [13:0] sp, 
        input logic [3:0] alpha, 
        input logic [4:0] sat, 
        input logic enable
    );
        begin   
            send_cmd_two_step(CMD_SET_KP, {10'b0, kp}); 
            send_cmd_two_step(CMD_SET_KD, {10'b0, kd}); 
            send_cmd_two_step(CMD_SET_KI, {10'b0, ki});
            send_cmd_two_step(CMD_SET_DEC, {12'b0, dec});
            send_cmd_two_step(CMD_SET_SP, {12'b0, sp});
            send_cmd_two_step(CMD_SET_ALPHA_SAT_EN, {16'b0, alpha, sat, enable});
        end
    endtask


    logic signed [15:0] kp_w; 
    logic signed [15:0] kd_w; 
    logic signed [15:0] ki_w; 
    logic [13:0] dec_w; 
    logic signed [13:0] sp_w; 
    logic [3:0] alpha_w; 
    logic [4:0] sat_w;
    logic enable_w;


    initial begin
        $dumpfile("dumps/tb_pdh.vcd");
        $dumpvars(0, tb_pdh_core);
        
        axi_from_ps_i = 32'h0;
        
        $display("---- Resetting ----");
        hold_word(make_word(1'b1, 1'b0, CMD_IDLE, 26'h0), 10);
        hold_word(make_word(1'b0, 1'b0, CMD_IDLE, 26'h0), 10);

        $display("---- Set LED to 0x55 (Two-Step) ----");
        send_cmd_two_step(CMD_SET_LED, 26'h55);
        
        repeat(5) @(posedge clk);
        if (led_o === 8'h55) 
            $display("PASS: LED IS 0x55");
        else 
            $display("FAIL: LED IS 0x%h", led_o);


        if (axi_to_ps_o === {CMD_SET_LED, 1'b0, 19'd0, 8'h55})
            $display("PASS: Callback Correct");
        else
            $display("FAIL: Callback was 0x%h", axi_to_ps_o);


        // ---------------- DAC TESTS ----------------
        $display("---- Set DAC0 (lower 14b) to 0x0123 ----");
        send_set_dac_two_step(1'b0, 14'h0123);
        expect_dac_word({14'h0000, 14'h0123}, "DAC0 write");

        $display("---- Set DAC1 (upper 14b) to 0x1ABC ----");
        send_set_dac_two_step(1'b1, 14'h1ABC);
        expect_dac_word({14'h1ABC}, "DAC1 write");

        $display("---- Update DAC0 again to 0x0005 (DAC1 must stay) ----");
        send_set_dac_two_step(1'b0, 14'h0005);
        expect_dac_word({14'h0005}, "DAC0 rewrite");

        send_cmd_two_step(CMD_GET_ADC, 26'b0);
        repeat(5) @(posedge clk);

        #100;
        //START TEST PID BLOCK

        //SAME AS FIRST TESTCASE FROM PID TB but with 18-bit satwidth
        kp_w = {2'b0, {14{1'b1}}};
        kd_w = {3'b0, {13{1'b1}}};
        ki_w =  {3'b0, {13{1'b1}}};
        alpha_w = 4'd2;
        enable_w = 1'b1;
        dec_w = 14'd2; 
        sp_w = 14'sd0; 
        sat_w = 5'd18;
        adc_dat_a_w = '0; //ADC is reading +1V, SP is set to 0V
        config_pid(
            .kp(kp_w),
            .kd(kd_w),
            .ki(ki_w),
            .dec(dec_w),
            .sp(sp_w),
            .alpha(alpha_w),
            .sat(sat_w),
            .enable(enable_w)
        );
        #2000;

        $finish;
    end

endmodule
