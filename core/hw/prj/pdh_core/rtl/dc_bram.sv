//dual-clock BRAM

module dc_bram #(
  parameter int DEPTH,
  parameter int AW
) (
  input  logic        wclk,
  input  logic        we,
  input  logic [AW-1:0] waddr,
  input  logic [63:0] wdata,

  input  logic        rclk,
  input  logic [AW-1:0] raddr,
  output logic [63:0] rdata
);
  (* ram_style="block" *) logic [63:0] mem [0:DEPTH-1];

  always_ff @(posedge wclk) begin
    if (we) mem[waddr] <= wdata;
  end

  always_ff @(posedge rclk) begin
    rdata <= mem[raddr];
  end
endmodule
