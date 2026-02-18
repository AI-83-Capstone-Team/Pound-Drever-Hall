module bram_controller
#(
    parameter int unsigned DEPTH = 16_384,
    parameter int unsigned AW = 31 //$clog2(DEPTH)
)
(
    input logic pdh_clk,
    input logic axi_clk,
    input logic rst_i,
    input logic [AW-1:0] addr_i, //TODO: Adress this
    input logic [63:0] din,
    output logic [63:0] dout,
    input logic enable_i,
    input logic [21:0] decimation_code_i,
    output logic bram_ready_o
);

    //(* ram_style = "block" *) reg[63:0] bram_r[0:DEPTH];
    logic bram_we_w;
    logic[AW-1:0] addr_r, next_addr_w;

    logic [21:0] decimation_code_r, next_decimation_code_w;
    logic [21:0] count_r, next_count_w;

    dc_bram 
    #(
        .DEPTH(DEPTH),
        .AW(AW)
    ) u_bram (
        .wclk(pdh_clk),
        .we(bram_we_w), 
        .waddr(addr_r),
        .wdata(din),
        .rclk(axi_clk),
        .raddr(addr_i),
        .rdata(dout)
    );


    typedef enum logic
    {
        ST_IDLE = 1'b0,
        ST_CAPTURE_DATA = 1'b1
    }   state_t;
    state_t state_r, next_state_w;


    logic enable_sync_edge_w;
    posedge_detector #(
    ) u_enable_edge_detector (
        .clk(pdh_clk),
        .rst(rst_i),
        .D  (enable_i),
        .Q  (enable_sync_edge_w)
    );

    //state transition logic
    always_comb begin
        unique case(state_r)
            ST_IDLE: next_state_w = enable_sync_edge_w? ST_CAPTURE_DATA : ST_IDLE;
            ST_CAPTURE_DATA: next_state_w = (addr_r == (DEPTH-1))? ST_IDLE : ST_CAPTURE_DATA;
            default: next_state_w = ST_IDLE;
        endcase
    end


    //ouput/counter logic
    always_comb begin
        unique case(state_r)
            ST_IDLE: begin
                bram_ready_o = 1'b1;
                next_addr_w = {AW{1'd0}};
                bram_we_w = 1'b0;
                next_decimation_code_w = enable_sync_edge_w? decimation_code_i : decimation_code_r;
                next_count_w = 26'd0;
            end

            ST_CAPTURE_DATA: begin
                bram_ready_o = 1'b0;
                next_addr_w = (count_r == 0)? addr_r + 1 : addr_r;
                bram_we_w = 1'b1;
                next_decimation_code_w = decimation_code_r;
                next_count_w = (count_r >= decimation_code_r-1)? 26'd0 : count_r + 1;
            end

            default: begin
                bram_ready_o = 1'b0;
                next_addr_w = {AW{1'd0}};
                bram_we_w = 1'b0;
                next_decimation_code_w = 26'd1;
                next_count_w = 26'd0;
            end

        endcase
    end
    
    logic rst_sync_r, rst_pipe1_r;
    always_ff @(posedge pdh_clk or posedge rst_i) begin
        if(rst_i) begin
            {rst_sync_r, rst_pipe1_r} <= {1'b1, 1'b1};
        end else begin
            {rst_sync_r, rst_pipe1_r} <= {rst_pipe1_r, 1'b0};
        end
    end

    always_ff @(posedge pdh_clk) begin
        if (rst_sync_r) begin
            state_r <= ST_IDLE;
            addr_r <= {AW{1'd0}};
            decimation_code_r <= 26'd1;
            count_r <= 26'd0;
        end else begin
            state_r <= next_state_w;
            addr_r <= next_addr_w;
            decimation_code_r <= next_decimation_code_w;
            count_r <= next_count_w;
        end
    end

 endmodule
