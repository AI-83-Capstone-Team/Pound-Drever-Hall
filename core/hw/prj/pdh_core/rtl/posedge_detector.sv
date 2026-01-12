module posedge_detector
(
    input logic clk,
    input logic rst,
    input logic D,
    output logic Q
);

    logic prev_D;
    assign Q = D & ~prev_D;
    
    always_ff @(posedge clk) begin
        if(rst) begin
            prev_D <= 0;
        end else begin
            prev_D <= D;
        end
    end
endmodule
