module sine_qtr_rom #(
    parameter integer AW = 12, // 2^12 = 4096 entries
    parameter integer DW = 16,
    parameter        INIT_FILE = "sine_qtr_4096_16b.mem"
)(
    input  logic clk,
    input  logic [AW-1:0] addr1,
    input  logic [AW-1:0] addr2,
    output logic [DW-1:0] dout1,
    output logic [DW-1:0] dout2
);


    `ifndef NOSYNTH
        (* rom_style = "block" *) logic [DW-1:0] rom [0:(1<<AW)-1];
    `else
        logic [DW-1:0] rom [0:(1<<AW)-1];
    `endif


    integer i;
    initial begin
        for (i = 0; i < (1<<AW); i = i+1)
        rom[i] = {DW{1'b0}};

        $readmemh(INIT_FILE, rom);
    end

    //infer dual-sync BRAM here
    always_ff @(posedge clk) begin
        dout1 <= rom[addr1];
        dout2 <= rom[addr2];
    end

endmodule
