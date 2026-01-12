`timescale 1ns/1ps

module tb_pdh_core;

    logic clk = 1'b0;
    always #4 clk = ~clk;
    
    logic [31:0] axi_from_ps_i;
    logic [31:0] axi_to_ps_o;
    logic [7:0]  led_o;
    
    logic [31:0] S_AXIS_tdata_i = 32'h0;
    logic        S_AXIS_tvalid_i = 1'b0;
    logic [31:0] dac_tdata_o;
    logic        dac_tvalid_o;

    typedef enum logic [3:0] {
        CMD_IDLE    = 4'b0000,
        CMD_SET_LED = 4'b0001,
        CMD_SET_DAC = 4'b0010
    } cmd_t;

    pdh_core dut (
      .clk(clk),
      .adc_tdata_i(S_AXIS_tdata_i),
      .adc_tvalid_i(S_AXIS_tvalid_i),
      .axi_from_ps_i(axi_from_ps_i),
      .axi_to_ps_o(axi_to_ps_o),
      .led_o(led_o),
      .dac_tdata_o(dac_tdata_o),
      .dac_tvalid_o(dac_tvalid_o)
    );

    logic [3:0] cmd_cb_tap;
    logic strobe_tap;
    logic [7:0] led_cb_tap;
    assign cmd_cb_tap = axi_to_ps_o[31:28];
    assign cmd_strobe_tap = axi_to_ps_o[27]; 
    assign led_cb_tap = axi_to_ps_o[7:0];
        
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
        input logic dac_sel,          // 0 -> lower 14b, 1 -> upper 14b
        input logic [13:0] code
    );
        logic [25:0] p;
        begin
            p = 26'd0;
            p[14]    = dac_sel;
            p[13:0]  = code;
            return p;
        end
    endfunction

    task automatic expect_dac_word(input logic [31:0] exp, input string tag="");
    begin
        // wait a couple cycles for registers to settle
        repeat (2) @(posedge clk);
        if (dac_tdata_o === exp)
            $display("PASS: %s dac_tdata_o = 0x%08h", tag, dac_tdata_o);
        else
            $display("FAIL: %s dac_tdata_o = 0x%08h (expected 0x%08h)", tag, dac_tdata_o, exp);
    end
    endtask

    task automatic expect_tvalid_single_pulse(input int window_cycles, input string tag="");
        int pulses;
    begin
        pulses = 0;
        repeat (window_cycles) begin
            @(posedge clk);
            if (dac_tvalid_o) pulses++;
        end
        if (pulses == 1)
            $display("PASS: %s dac_tvalid_o pulsed exactly once", tag);
        else
            $display("FAIL: %s dac_tvalid_o pulses=%0d (expected 1)", tag, pulses);
    end
    endtask

    task automatic send_set_dac_two_step(input logic dac_sel, input logic [13:0] code);
        logic [25:0] payload;
    begin
        payload = make_dac_payload(dac_sel, code);
        send_cmd_two_step(CMD_SET_DAC, payload);
    end
    endtask



    initial begin
        $dumpfile("wave.fst");
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

        // Expect: {00, upper14, 00, lower14} = upper still 0
        expect_dac_word({2'b00, 14'h0000, 2'b00, 14'h0123}, "DAC0 write");
        // tvalid should pulse once within the command window
        expect_tvalid_single_pulse(20, "DAC0 tvalid");

        $display("---- Set DAC1 (upper 14b) to 0x1ABC ----");
        send_set_dac_two_step(1'b1, 14'h1ABC);

        // Expect upper updated, lower preserved (0x0123)
        expect_dac_word({2'b00, 14'h1ABC, 2'b00, 14'h0123}, "DAC1 write");
        expect_tvalid_single_pulse(20, "DAC1 tvalid");

        $display("---- Update DAC0 again to 0x0005 (DAC1 must stay) ----");
        send_set_dac_two_step(1'b0, 14'h0005);

        // Expect lower updated, upper preserved (0x1ABC)
        expect_dac_word({2'b00, 14'h1ABC, 2'b00, 14'h0005}, "DAC0 rewrite");
        expect_tvalid_single_pulse(20, "DAC0 rewrite tvalid");


        $finish;
    end

endmodule
