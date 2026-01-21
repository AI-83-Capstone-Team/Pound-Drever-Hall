`timescale 1 ns / 1 ps



module dma_controller
(
    input   logic        aclk,
    input   logic        rst_i,

    output  logic [31:0] m_axi_awaddr,
    output  logic        m_axi_awvalid,
    input   logic        m_axi_awready,
    output  logic [3:0]  m_axi_awlen,   //num beats in burst
    output  logic [2:0]  m_axi_awsize,  //num bytes in beat
    output  logic [1:0]  m_axi_awburst, //burst code

    output  logic [63:0] m_axi_wdata,
    output  logic        m_axi_wvalid,
    input   logic        m_axi_wready,
    input   logic        m_axi_bvalid,
    output  logic        m_axi_bready,
    input   logic [1:0]  m_axi_bresp, 
    output  logic [7:0]  m_axi_wstrb,
    output  logic        m_axi_wlast,

    input   logic enable_i,
    input   logic [63:0] data_i, //TODO: FIFO on this
    output  logic finished_o,
    output  logic dma_engaged_o
);

    // localparam int unsigned HP0_BASE_ADDR = 32'h1F00_0000;
    localparam int unsigned HP0_BASE_ADDR = 32'h4300_0000;
    localparam int unsigned DMA_SIZE = 32'h000C3500;
    localparam int unsigned FINAL_WRITE_ADDR = HP0_BASE_ADDR + DMA_SIZE - 1;
    localparam int unsigned BYTES_PER_BEAT = 8; //each address points to a location in DDR holding this many bits
    localparam int unsigned ADDR_INC = 16*8; // beats/burst * bytes/beat

    localparam logic [3:0] BEAT_BASE = 4'd0;

    localparam logic [1:0] BURST_INC = 2'b01;
    localparam logic [3:0] BURST_LEN = 4'd15;
    localparam logic [2:0] BEAT_SIZE = 3'd3;
    localparam logic [7:0] STROBE_FULL = 8'hFF;


    

    logic enable_meta_w, enable_sync_r, enable_sync_edge_w;
    assign enable_meta_w = enable_i;

    posedge_detector #(
    ) u_enable_edge_detector (
        .clk(aclk),
        .rst(rst_i),
        .D  (enable_sync_r),
        .Q  (enable_sync_edge_w)
    );


    typedef enum logic[2:0]
    {
        ST_IDLE = 3'b000,
        ST_SET_ADDR_AWAIT_ACK = 3'b001,
        ST_SET_DATA_AWAIT_ACK = 3'b010,
        ST_AWAIT_RESP = 3'b011,
        ST_DONE = 3'b100
    } state_t;
    state_t state_r, next_state_w;


    logic [31:0] addr_r, next_addr_w;
    logic [4:0] beat_r, next_beat_w;
    logic [63:0] data_r, next_data_w;
    logic awvalid_w, wvalid_w, wlast_w, bready_w, finished_w, dma_engaged_w;

    //State transition logic
    always_comb begin
        unique case(state_r)
            ST_IDLE: next_state_w = enable_sync_edge_w? ST_SET_ADDR_AWAIT_ACK : ST_IDLE;
            ST_SET_ADDR_AWAIT_ACK: next_state_w = m_axi_awready? ST_SET_DATA_AWAIT_ACK : ST_SET_ADDR_AWAIT_ACK;
            ST_SET_DATA_AWAIT_ACK: next_state_w = (m_axi_wready && (beat_r == BURST_LEN))? ST_AWAIT_RESP : ST_SET_DATA_AWAIT_ACK;
            //ST_AWAIT_RESP: next_state_w = m_axi_bvalid? ((m_axi_bresp == 2'b00)? ((addr_r + ADDR_INC > FINAL_WRITE_ADDR)? ST_DONE : ST_SET_ADDR_AWAIT_ACK) : ST_SET_ADDR_AWAIT_ACK) : ST_AWAIT_RESP; <-ignoring response status for now to see if we can observe corrupted writes
            ST_AWAIT_RESP: next_state_w = m_axi_bvalid? ((addr_r + ADDR_INC > FINAL_WRITE_ADDR)? ST_DONE : ST_SET_ADDR_AWAIT_ACK) : ST_AWAIT_RESP;
            ST_DONE: next_state_w = enable_sync_edge_w? ST_SET_ADDR_AWAIT_ACK : ST_DONE;
            default: next_state_w = ST_IDLE;
        endcase
    end

    //Signal logic
    always_comb begin
        unique case(state_r)
            ST_IDLE: begin
                next_addr_w = HP0_BASE_ADDR;
                next_beat_w = BEAT_BASE;
                next_data_w = 64'b0;
                awvalid_w = 1'b0;
                wvalid_w = 1'b0;
                wlast_w = 1'b0;
                bready_w = 1'b0;
                finished_w = 1'b0;
                dma_engaged_w = 1'b0;
            end

            ST_SET_ADDR_AWAIT_ACK: begin
                next_addr_w = addr_r;
                next_beat_w = BEAT_BASE;
                next_data_w = data_i;
                awvalid_w = 1'b1;
                wvalid_w = 1'b0;
                wlast_w = 1'b0;
                bready_w = 1'b0;
                finished_w = 1'b0;
                dma_engaged_w = 1'b1;
            end

            ST_SET_DATA_AWAIT_ACK: begin
                next_addr_w = addr_r;
                next_beat_w = m_axi_wready? beat_r + 1 : beat_r;
                next_data_w = m_axi_wready? data_i : data_r;
                awvalid_w = 1'b0;
                wvalid_w = 1'b1;
                wlast_w = (beat_r == BURST_LEN);
                bready_w = 1'b0;
                finished_w = 1'b0;
                dma_engaged_w = 1'b1;
            end

            ST_AWAIT_RESP: begin
               // next_addr_w = (m_axi_bvalid & (m_axi_bresp == 2'b00))? addr_r + ADDR_INC : addr_r;
                next_addr_w = (m_axi_bvalid)? addr_r + ADDR_INC : addr_r;
                next_beat_w = BEAT_BASE; //should wrap to base anyway via overflow
                next_data_w = 64'b0;
                awvalid_w = 1'b0;
                wvalid_w = 1'b0;
                wlast_w = 1'b0;
                bready_w = 1'b1;
                finished_w = 1'b0;
                dma_engaged_w = 1'b1;
            end

            ST_DONE: begin
                next_addr_w = HP0_BASE_ADDR;
                next_beat_w = BEAT_BASE;
                next_data_w = 64'b0;
                awvalid_w = 1'b0;
                wvalid_w = 1'b0;
                wlast_w = 1'b0;
                bready_w = 1'b0;
                finished_w = 1'b1;
                dma_engaged_w = 1'b0;
            end
            
            default: begin
                next_addr_w = HP0_BASE_ADDR;
                next_beat_w = BEAT_BASE;
                next_data_w = 64'b0;
                awvalid_w = 1'b0;
                wvalid_w = 1'b0;
                wlast_w = 1'b0;
                bready_w = 1'b0;
                finished_w = 1'b0;
                dma_engaged_w = 1'b0;
            end
        endcase
    end


    //constant values
    assign m_axi_awburst = BURST_INC; 
    assign m_axi_awlen = BURST_LEN; //15 + 1 = 16 beats
    assign m_axi_awsize = BEAT_SIZE; //2^3 = 8 bytes = 64 bits/beat
    assign m_axi_wstrb = STROBE_FULL; //all 8 bytes valid
    
    //outputs
    assign m_axi_awaddr = addr_r;
    assign m_axi_wdata = data_r;
    assign m_axi_awvalid = awvalid_w;
    assign m_axi_wvalid = wvalid_w;
    assign m_axi_bready = bready_w;
    assign m_axi_wlast = wlast_w;

    assign finished_o = finished_w;
    assign dma_engaged_o = dma_engaged_w;

    always_ff @(posedge aclk or posedge rst_i) begin
        if (rst_i) begin
            enable_sync_r <= 1'b0;
            state_r <= ST_IDLE;
            addr_r <= HP0_BASE_ADDR;
            beat_r <= BEAT_BASE;
            data_r <= 64'd0;
        end else begin
            enable_sync_r <= enable_meta_w;
            state_r <= next_state_w;
            addr_r <= next_addr_w;
            beat_r <= next_beat_w;
            data_r <= next_data_w;
        end
    end

endmodule

