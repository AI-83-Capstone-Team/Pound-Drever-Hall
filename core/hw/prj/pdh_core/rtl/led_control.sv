`timescale 1 ns / 1 ps

module led_control #
(
    parameter CALLBACK_WIDTH = 8, //This module should have 8-bit callback width
    parameter DATA_WIDTH = 8,
    parameter LED_WIDTH = 8
)
(
    input logic [3:0] cmd_i,
    input logic [DATA_WIDTH-1:0] led_payload_i,

    input logic en_i,
    input logic clk,
    input logic rst_i,

    output logic [CALLBACK_WIDTH-1 : 0] callback_o,
    output logic [LED_WIDTH-1 : 0] led_o
);

    typedef enum logic [1:0] {
        CLEAR = 2'b00,
        SET_LED = 2'b01,
        HOLD = 2'b10
    } state_t;
    state_t state_r, next_state_w;

    logic [LED_WIDTH-1 : 0] next_led_w, led_r;
    assign led_o = led_r;

    logic [CALLBACK_WIDTH-1: 0] next_callback_w, callback_r;
    assign callback_o = callback_r;

    logic en_r;


    always_comb begin
        case(state_r)
            CLEAR: begin
                next_state_w = en_r? SET_LED : CLEAR;
                next_led_w = 0;
                next_callback_w = 0;
            end

            SET_LED: begin
                next_state_w = HOLD;
                next_led_w =  led_payload_r;
                next_callback_w = 0;
            end

            HOLD: begin 
                next_state_w = en_r? SET_LED : HOLD;
                next_led_w = led_r;
                next_callback_w = led_r;
            end

            default: begin
                next_state_w = CLEAR;
                next_led_w = 0;
                next_callback_w = 0;
            end
        endcase
    end

    logic[7:0] led_payload_r;
    always_ff @(posedge clk) begin

        if(rst_i) begin
            state_r <= CLEAR;
            led_r <= 0;
            en_r <= 0;
            callback_r <= 0;
            led_payload_r <= 0;
        end

        else begin
            if(cmd_i == 2'b01) begin
                led_payload_r <= led_payload_i;
            end else begin
                led_payload_r <= led_payload_r;
            end

            state_r <= next_state_w;
            led_r <= next_led_w;
            en_r <= en_i;
            callback_r <= next_callback_w;
        end
    end




            


endmodule
