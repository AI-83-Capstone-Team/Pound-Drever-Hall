// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtop.h for the primary calling header

#include "Vtop__pch.h"
#include "Vtop__Syms.h"
#include "Vtop___024root.h"

#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__ico(Vtop___024root* vlSelf);
#endif  // VL_DEBUG

void Vtop___024root___eval_triggers__ico(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_triggers__ico\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.__VicoTriggered.setBit(0U, (IData)(vlSelfRef.__VicoFirstIteration));
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vtop___024root___dump_triggers__ico(vlSelf);
    }
#endif
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__act(Vtop___024root* vlSelf);
#endif  // VL_DEBUG

void Vtop___024root___eval_triggers__act(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_triggers__act\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.__VactTriggered.setBit(0U, ((IData)(vlSelfRef.pdh_core__DOT__clk) 
                                          & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__clk__0))));
    vlSelfRef.__VactTriggered.setBit(1U, ((IData)(vlSelfRef.pdh_core__DOT__rst_i) 
                                          & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__rst_i__0))));
    vlSelfRef.__VactTriggered.setBit(2U, ((~ (IData)(vlSelfRef.pdh_core__DOT__clk)) 
                                          & (IData)(vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__clk__0)));
    vlSelfRef.__VactTriggered.setBit(3U, ((IData)(vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__clk) 
                                          & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_strobe_edge_detector__DOT__clk__0))));
    vlSelfRef.__VactTriggered.setBit(4U, ((IData)(vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__rst) 
                                          & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_strobe_edge_detector__DOT__rst__0))));
    vlSelfRef.__VactTriggered.setBit(5U, ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__clk) 
                                          & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_nco__DOT__clk__0))));
    vlSelfRef.__VactTriggered.setBit(6U, ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__clk) 
                                          & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__clk__0))));
    vlSelfRef.__VactTriggered.setBit(7U, ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__clk) 
                                          & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_pid__DOT__clk__0))));
    vlSelfRef.__VactTriggered.setBit(8U, ((IData)(vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__clk) 
                                          & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_dma_edge_detector__DOT__clk__0))));
    vlSelfRef.__VactTriggered.setBit(9U, ((IData)(vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__rst) 
                                          & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_dma_edge_detector__DOT__rst__0))));
    vlSelfRef.__VactTriggered.setBit(0xaU, ((IData)(vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__clk) 
                                            & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_bram_edge_detector__DOT__clk__0))));
    vlSelfRef.__VactTriggered.setBit(0xbU, ((IData)(vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__rst) 
                                            & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_bram_edge_detector__DOT__rst__0))));
    vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__clk__0 
        = vlSelfRef.pdh_core__DOT__clk;
    vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__rst_i__0 
        = vlSelfRef.pdh_core__DOT__rst_i;
    vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_strobe_edge_detector__DOT__clk__0 
        = vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__clk;
    vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_strobe_edge_detector__DOT__rst__0 
        = vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__rst;
    vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_nco__DOT__clk__0 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__clk;
    vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__clk__0 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__clk;
    vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_pid__DOT__clk__0 
        = vlSelfRef.pdh_core__DOT__u_pid__DOT__clk;
    vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_dma_edge_detector__DOT__clk__0 
        = vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__clk;
    vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_dma_edge_detector__DOT__rst__0 
        = vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__rst;
    vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_bram_edge_detector__DOT__clk__0 
        = vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__clk;
    vlSelfRef.__Vtrigprevexpr___TOP__pdh_core__DOT__u_bram_edge_detector__DOT__rst__0 
        = vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__rst;
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vtop___024root___dump_triggers__act(vlSelf);
    }
#endif
}
