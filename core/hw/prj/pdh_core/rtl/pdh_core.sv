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
    input logic clk, //FCLK_CLK0

    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input logic [AXIS_TDATA_WIDTH-1:0] S_AXIS_tdata_i,
    input logic S_AXIS_tvalid_i,

    input logic [AXI_GPIO_IN_WIDTH-1:0] axi_from_ps_i,
    output logic [AXI_GPIO_OUT_WIDTH-1:0] axi_to_ps_o,
    output logic [7:0] led_o,

    ///////////// TEST //////////////////////////////////
    output logic [3:0] cmd_tap,
    output logic [3:0] state_tap,
    output logic [7:0] led_payload_tap,
    output logic rst_r_tap
);
/////////////////////  LOCAL PARAMS   //////////////////////////////////
    localparam int unsigned NUM_MODULES = 1;
    localparam int unsigned CMD_BITS = 4;
    localparam int unsigned DATA_BITS = 27;

    localparam int unsigned CMD_END = 30;
    localparam int unsigned CMD_START = 27;
    localparam int unsigned DATA_END = 26;
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
        CMD_IDLE = 4'b0000,
        CMD_SET_LED = 4'b0001,
        CMD_STROBE = 4'b1110
    } cmd_t;

    typedef enum logic [CMD_BITS-1:0] {
        ST_IDLE = 4'b0000,
        ST_SET_LED = 4'b0001,
        ST_STROBE = 4'b1110,
        ST_INVALID_STATE = 4'b1111
    } state_t;


    cmd_t cmd_sig_r, prev_cmd_sig_r, cmd_sig_w, last_cmd_r;
    state_t next_state_w, state_r, prev_state_r;

    assign cmd_sig_w = cmd_t'(axi_from_ps_i[CMD_END : CMD_START]);

    logic [DATA_BITS-1 : 0] data_sig_w;
    assign data_sig_w = axi_from_ps_i[DATA_END : DATA_START];
    logic [DATA_BITS-1 : 0] data_sig_r;

    logic rst_i;
    assign rst_i = axi_from_ps_i[31];
    
    logic [NUM_MODULES-1 : 0] en_bus_r;
    logic [NUM_MODULES-1 : 0] next_en_bus_w;


    //cmd -> next_state_cmd logic
    always_comb begin
        case(cmd_sig_r)
            CMD_IDLE: next_state_w = ST_IDLE;
            CMD_SET_LED: next_state_w = ST_SET_LED;
            CMD_STROBE: next_state_w = ST_STROBE;
            default: next_state_w = ST_INVALID_STATE;
        endcase
    end

    localparam logic DISABLE_ALL = 1'b0;
    localparam logic ENABLE_LED = 1'b1;

    always_comb begin
        case(prev_state_r)
            ST_IDLE: next_en_bus_w = DISABLE_ALL;
            ST_SET_LED: next_en_bus_w = ENABLE_LED;
            ST_STROBE: next_en_bus_w = DISABLE_ALL;
            default: next_en_bus_w = DISABLE_ALL;
        endcase
    end


    logic rst_r, strobe_edge;
    assign strobe_edge = (cmd_sig_r == CMD_STROBE) && (prev_cmd_sig_r != CMD_STROBE);

    always_ff @(posedge clk) begin
        rst_r <= rst_i;

        if(rst_r) begin
            cmd_sig_r <= CMD_IDLE; //Register cmds and data signals for CDC safety
            prev_cmd_sig_r <= CMD_IDLE;
            data_sig_r <= 0;
            prev_state_r <= ST_IDLE;
            state_r <= ST_IDLE;
            en_bus_r <= DISABLE_ALL;
            last_cmd_r <= CMD_IDLE;
        end else begin
            cmd_sig_r <= cmd_sig_w;
            prev_cmd_sig_r <= cmd_sig_r;
            data_sig_r <= data_sig_w;
            prev_state_r <= state_r;
            state_r <= next_state_w;
        end
        
        if(strobe_edge) begin 
            en_bus_r <= next_en_bus_w;
            last_cmd_r <= prev_cmd_sig_r;
        end else begin
            en_bus_r <= DISABLE_ALL;
            last_cmd_r <= last_cmd_r;
        end
    end

    


//////////////////////// LED Controller ///////////////////
    logic[7:0] led_payload_r;
   
    always_ff @(posedge clk) begin
        if(rst_r) begin
            led_payload_r <= 0;
        end else if(cmd_sig_r == CMD_SET_LED) begin
            led_payload_r <= data_sig_r[7:0];
        end else begin
            led_payload_r <= led_payload_r;
        end
    end


    logic[7:0] led_controller_callback_w;


    led_control led_control_u(
        .en_i(en_bus_r[0]),
        .clk(clk),
        .rst_i(rst_r),
        .data_in_w(led_payload_r),
        .callback_o(led_controller_callback_w),
        .led_o(led_o)
    );

    //TODO: Full 32-bit usage scheme here
    logic [7:0] func_callback_w;
    always_comb begin
        case(last_cmd_r)
            CMD_IDLE: func_callback_w = 0;
            CMD_SET_LED: func_callback_w = led_controller_callback_w;
            CMD_STROBE: func_callback_w = 0;
            default: func_callback_w = 0;
        endcase
    end
    
    always_comb begin
        axi_to_ps_o = '0;
        axi_to_ps_o[7:0] = func_callback_w;
        axi_to_ps_o[11:8] = last_cmd_r;
        axi_to_ps_o[15:12] = cmd_sig_r;
    end







//////////////////// TEST ////////////////////////////////
    assign cmd_tap = cmd_sig_r;
    assign state_tap = state_r;
    assign led_payload_tap = led_payload_r;
    assign rst_r_tap = rst_r;


endmodule
