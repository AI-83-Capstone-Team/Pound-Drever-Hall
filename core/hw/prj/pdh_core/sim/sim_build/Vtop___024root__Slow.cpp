// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtop.h for the primary calling header

#include "Vtop__pch.h"
#include "Vtop__Syms.h"
#include "Vtop___024root.h"

// Parameter definitions for Vtop___024root
constexpr SData/*14:0*/ Vtop___024root::pdh_core__DOT__ADC_OFFSET;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__ADC_DATA_WIDTH;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__DAC_DATA_WIDTH;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__AXI_GPIO_IN_WIDTH;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__AXI_GPIO_OUT_WIDTH;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__NUM_MODULES;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__CMD_BITS;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__CMD_END;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__CMD_START;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__DATA_END;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__DATA_START;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__AW;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__DW;
constexpr VlWide<6>/*167:0*/ Vtop___024root::pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__INIT_FILE;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__u_pid__DOT__S32_W;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__u_pid__DOT__S16_W;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__u_pid__DOT__DEC_W;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__u_pid__DOT__W1S;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__u_pid__DOT__W2S;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__u_pid__DOT__W1U;
constexpr IData/*31:0*/ Vtop___024root::pdh_core__DOT__u_pid__DOT__W2U;


void Vtop___024root___ctor_var_reset(Vtop___024root* vlSelf);

Vtop___024root::Vtop___024root(Vtop__Syms* symsp, const char* v__name)
    : VerilatedModule{v__name}
    , vlSymsp{symsp}
 {
    // Reset structure values
    Vtop___024root___ctor_var_reset(this);
}

void Vtop___024root::__Vconfigure(bool first) {
    (void)first;  // Prevent unused variable warning
}

Vtop___024root::~Vtop___024root() {
}
