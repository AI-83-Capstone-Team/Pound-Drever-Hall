module bram_controller
#(
    parameter int unsigned DEPTH = 16_384,
    parameter int unsigned AW = $clog2(DEPTH)
)
(
    input logic pdh_clk,
    input logic axi_clk,
    input logic rst_i,
    input logic [AW-1:0] addr_i,
    input logic [63:0] din,
    output logic [63:0] dout,
    input logic enable_i,
    output logic dma_enable,
    input logic dma_termination_sig,
    input logic [21:0] decimation_code_i,
    output logic transaction_complete
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


    typedef enum logic[2:0]
    {
        ST_IDLE = 3'b000,
        ST_CAPTURE_DATA = 3'b001,
        ST_AWAIT_DMA_TERMINATE = 3'b010,
        ST_DONE = 3'b011
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
            ST_CAPTURE_DATA: next_state_w = (addr_r == (DEPTH-1))? ST_AWAIT_DMA_TERMINATE : ST_CAPTURE_DATA;
            ST_AWAIT_DMA_TERMINATE: next_state_w = dma_termination_sig? ST_DONE : ST_AWAIT_DMA_TERMINATE;
            ST_DONE: next_state_w = enable_sync_edge_w? ST_CAPTURE_DATA : ST_DONE;             
            default: next_state_w = ST_IDLE;
        endcase
    end


    //ouput/counter logic
    always_comb begin
        unique case(state_r)
            ST_IDLE: begin
                dma_enable = 1'b0;
                transaction_complete = 1'b0;
                next_addr_w = {AW{1'd0}};
                bram_we_w = 1'b0;
                next_decimation_code_w = enable_sync_edge_w? decimation_code_i : decimation_code_r;
                next_count_w = 26'd0;
            end

            ST_CAPTURE_DATA: begin
                dma_enable = 1'b0;
                transaction_complete = 1'b0;
                next_addr_w = (count_r == 0)? addr_r + 1 : addr_r;
                bram_we_w = 1'b1;
                next_decimation_code_w = decimation_code_r;
                next_count_w = (count_r >= decimation_code_r-1)? 26'd0 : count_r + 1;
            end

            ST_AWAIT_DMA_TERMINATE: begin
                dma_enable = 1'b1;
                transaction_complete = 1'b0;
                next_addr_w = {AW{1'b1}};
                bram_we_w = 1'b0;
                next_decimation_code_w = decimation_code_r;
                next_count_w = 26'd0;
            end

            ST_DONE: begin
                dma_enable = 1'b0;
                transaction_complete = 1'b1;
                next_addr_w = {AW{1'd0}};
                bram_we_w = 1'b0;
                next_decimation_code_w = enable_sync_edge_w? decimation_code_i : decimation_code_r;
                next_count_w = 26'd0;
            end

            default: begin
                dma_enable = 1'b0;
                transaction_complete = 1'b0;
                next_addr_w = {AW{1'd0}};
                bram_we_w = 1'b0;
                next_decimation_code_w = 26'd1;
                next_count_w = 26'd0;
            end

        endcase
    end
    

    always_ff @(posedge pdh_clk or posedge rst_i) begin
        if (rst_i) begin
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
