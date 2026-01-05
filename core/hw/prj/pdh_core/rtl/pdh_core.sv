`timescale 1 ns / 1 ps

module pdh_core #
(
    parameter ADC_DATA_WIDTH = 16, //To account for padding
    parameter DAC_DATA_WIDTH = 14,
    parameter AXIS_TDATA_WIDTH = 32, //Delivered as 2 16-bit unsigned ints packed together
    parameter AXI_GPIO_IN_WIDTH = 32,
    parameter AXI_GPIO_OUT_WIDTH = 32
)
(
    input logic rst_n, //xlc_reset.dout
    input logic clk, //FCLK_CLK0

    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input logic [AXIS_TDATA_WIDTH-1:0] S_AXIS_tdata_i,
    input logic S_AXIS_tvalid_i,

    input logic [AXI_GPIO_IN_WIDTH-1:0] axi_from_ps_i,
    output logic [AXI_GPIO_OUT_WIDTH-1:0] axi_to_ps_o,
    output logic [7:0] led_o
);
/////////////////////  LOCAL PARAMS   //////////////////////////////////
    localparam int unsigned NUM_MODULES = 1;
    localparam int unsigned CMD_BITS = 4;
    localparam int unsigned DATA_BITS = 28;

    localparam int unsigned CMD_END = CMD_BITS + DATA_BITS - 1;
    localparam int unsigned CMD_START = DATA_BITS;
    localparam int unsigned DATA_END = DATA_BITS-1;
    localparam int unsigned DATA_START = 0;


//////////////////////  DATA UNPACK    /////////////////////////////////
    logic [ADC_DATA_WIDTH-1 : 0] adc_1_tdata_w;
    logic adc_1_tvalid_w;
    
    logic [ADC_DATA_WIDTH-1 : 0] adc_2_tdata_w;
    logic adc_2_tvalid_w;

    assign adc_1_tdata_w = S_AXIS_tdata_i[ADC_DATA_WIDTH-1 : 0];
    assign adc_2_tdata_w = S_AXIS_tdata_i[AXIS_TDATA_WIDTH-1:ADC_DATA_WIDTH];
    
    assign adc_1_tvalid_w = S_AXIS_tvalid_i;
    assign adc_2_tvalid_w = S_AXIS_tvalid_i;
//////////////////////////////////////////////////////////



    typedef enum logic [CMD_BITS-1:0] {
        IDLE = 4'b0000,
        SET_LED = 4'b0001,
        STROBE = 4'b1110,
        INVALID_STATE = 4'b1111
    } cmd_t;
    cmd_t cmd_sig_r, cmd_sig_w, next_state_w, state_r, prev_state_r;
    logic [DATA_BITS-1 : 0] data_sig_w;
  
    assign cmd_sig_w = cmd_t'(axi_from_ps_i[CMD_END : CMD_START]);
    assign data_sig_w = axi_from_ps_i[DATA_END : DATA_START];
    logic [DATA_BITS-1 : 0] data_sig_r;
    
    logic [NUM_MODULES-1 : 0] en_bus_r;
    logic [NUM_MODULES-1 : 0] next_en_bus_w;


    //cmd -> next_state_cmd logic
    always_comb begin
        case(cmd_sig_r)
            IDLE: next_state_w = IDLE;
            SET_LED: next_state_w = SET_LED;
            STROBE: next_state_w = STROBE;
            default: next_state_w = INVALID_STATE;
        endcase
    end

    localparam logic DISABLE_ALL = 1'b0;
    localparam logic ENABLE_LED = 1'b1;

    always_comb begin
        case(prev_state_r)
            IDLE: next_en_bus_w = DISABLE_ALL;
            SET_LED: next_en_bus_w = ENABLE_LED;
            STROBE: next_en_bus_w = DISABLE_ALL;
            default: next_en_bus_w = DISABLE_ALL;
        endcase
    end



    always_ff @(posedge clk) begin
        if(~rst_n) begin
            cmd_sig_r <= IDLE; //Register cmds and data signals for CDC safety
            data_sig_r <= 0;
            prev_state_r <= IDLE;
            state_r <= IDLE;
            en_bus_r <= DISABLE_ALL;
        end

        else begin
            cmd_sig_r <= cmd_sig_w;
            data_sig_r <= data_sig_w;
            prev_state_r <= state_r;
            state_r <= next_state_w;

            if(state_r == STROBE) en_bus_r <= next_en_bus_w;
            else en_bus_r <= DISABLE_ALL; 
        end
    end



endmodule
