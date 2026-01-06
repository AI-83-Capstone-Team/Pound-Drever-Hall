
`timescale 1ns/1ps

module tb_pdh_core;

  // 125 MHz clock (8 ns period)
  logic clk = 1'b0;
  always #4 clk = ~clk;

  // DUT ports
  logic [31:0] axi_from_ps_i;
  logic [31:0] axi_to_ps_o;
  logic [7:0]  led_o;
  logic [3:0] cmd_tap;
  logic [3:0] state_tap;


  logic [31:0] S_AXIS_tdata_i;
  logic        S_AXIS_tvalid_i;

  logic [7:0] led_payload_tap;
  logic rst_r_tap;

  logic [7:0] led_in_tap;
  logic [7:0] rst_in_tap;

  logic [3:0] last_axi_cmd;
  logic [3:0] last_module_cmd;
  logic [7:0] func_callback;

  logic [3:0] incoming_cmd;

  assign led_in_tap = axi_from_ps_i[7:0];
  assign rst_in_tap = axi_from_ps_i[31];


  assign last_axi_cmd = axi_to_ps_o[15:12];
  assign last_module_cmd = axi_to_ps_o[11:8];
  assign func_callback = axi_to_ps_o[7:0];

  assign incoming_cmd = axi_from_ps_i[30:27];

  pdh_core dut (
    .clk(clk),
    .S_AXIS_tdata_i(S_AXIS_tdata_i),
    .S_AXIS_tvalid_i(S_AXIS_tvalid_i),
    .axi_from_ps_i(axi_from_ps_i),
    .axi_to_ps_o(axi_to_ps_o),
    .led_o(led_o),
    .cmd_tap(cmd_tap),
    .state_tap(state_tap),
    .led_payload_tap(led_payload_tap),
    .rst_r_tap(rst_r_tap)
  );

  // Wave dump (FST)
  initial begin
    $dumpfile("wave.fst");
    $dumpvars(0, tb_pdh_core);
  end

  // Bitfields in axi_from_ps_i
  localparam int CMD_START = 27;
  localparam int CMD_END   = 30;

  localparam logic [3:0] CMD_IDLE    = 4'h0;
  localparam logic [3:0] CMD_SET_LED = 4'h1;
  localparam logic [3:0] CMD_STROBE  = 4'hE;

  function automatic [31:0] make_word(
    input logic        rst,      // bit 31
    input logic [3:0]  cmd,      // bits 30:27
    input logic [7:0]  led_value // data bits 7:0
  );
    logic [31:0] w;
    begin
      w = 32'h0;
      w[31] = rst;
      w[CMD_END:CMD_START] = cmd;
      w[7:0] = led_value;
      make_word = w;
    end
  endfunction

  task automatic hold_word(input [31:0] w, input int cycles);
    begin
      axi_from_ps_i = w;
      repeat (cycles) @(posedge clk);
    end
  endtask

  task automatic soft_reset();
    begin
      // RTL resets when rst_n (registered) == 1
      // Hold rst=1 a few cycles, then drop to 0.
      hold_word(make_word(1'b1, CMD_IDLE, 8'h00), 6);
      hold_word(make_word(1'b0, CMD_IDLE, 8'h00), 6);
    end
  endtask

  task automatic do_set_led_then_strobe(input logic [7:0] val);
    begin
      // payload write
      hold_word(make_word(1'b0, CMD_SET_LED, val), 10);
      // strobe write (overwrites reg)
      hold_word(make_word(1'b0, CMD_STROBE,  8'h00), 10);
    end
  endtask

  task automatic expect_eq8(input string name, input logic [7:0] got, input logic [7:0] exp);
    begin
      if (got !== exp) begin
        $display("FAIL: %s got=0x%02x exp=0x%02x @ t=%0t", name, got, exp, $time);
        $display(1);
      end else begin
        $display("PASS: %s = 0x%02x @ t=%0t", name, got, $time);
      end
    end
  endtask

  task automatic expect_eq4(input string name, input logic [3:0] got, input logic [3:0] exp);
    begin
      if (got !== exp) begin
        $display("FAIL: %s got=0x%1x exp=0x%1x @ t=%0t", name, got, exp, $time);
        $display(1);
      end else begin
        $display("PASS: %s = 0x%1x @ t=%0t", name, got, $time);
      end
    end
  endtask

  initial begin
    axi_from_ps_i   = 32'h0;
    S_AXIS_tdata_i  = 32'h0;
    S_AXIS_tvalid_i = 1'b0;
    repeat (3) @(posedge clk);


    $display("---- soft reset ----");
    soft_reset();
    repeat (5) @(posedge clk);
    expect_eq8("led_o after reset", led_o, 8'h00);


    $display("---- set led to 0xA5 then strobe ----");
    do_set_led_then_strobe(8'hA5);
    repeat (30) @(posedge clk);
    expect_eq8("led_o after set", led_o, 8'hA5);
    expect_eq4("last_cmd_r", axi_to_ps_o[11:8], CMD_SET_LED);
    expect_eq4("cmd_sig_r", axi_to_ps_o[15:12], CMD_STROBE);
    expect_eq8("callback", axi_to_ps_o[7:0], 8'hA5);


    $display("---- set led to 0x00 then strobe (turn off) ----");
    do_set_led_then_strobe(8'h00);
    repeat (30) @(posedge clk);
    expect_eq8("led_o after off", led_o, 8'h00);
    expect_eq4("last_cmd_r after off", axi_to_ps_o[11:8], CMD_SET_LED);
    expect_eq4("cmd_sig_r after off", axi_to_ps_o[15:12], CMD_STROBE);
    expect_eq8("callback after off", axi_to_ps_o[7:0], 8'h00);


    $finish;
  end

endmodule
