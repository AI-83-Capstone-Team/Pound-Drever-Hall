// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtop.h for the primary calling header

#include "Vtop__pch.h"
#include "Vtop___024root.h"

VL_ATTR_COLD void Vtop___024root___eval_static(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_static\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
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
}

VL_ATTR_COLD void Vtop___024root___eval_initial__TOP(Vtop___024root* vlSelf);

VL_ATTR_COLD void Vtop___024root___eval_initial(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_initial\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    Vtop___024root___eval_initial__TOP(vlSelf);
}

VL_ATTR_COLD void Vtop___024root___eval_initial__TOP(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_initial__TOP\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Init
    VlWide<6>/*191:0*/ __Vtemp_1;
    // Body
    vlSelfRef.pdh_core__DOT__adc_csn_o = 1U;
    vlSelfRef.pdh_core__DOT__idle_cb_w = 0U;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__i = 0U;
    while (VL_GTS_III(32, 0x1000U, vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__i)) {
        vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__rom[(0xfffU 
                                                                  & vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__i)] = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__i 
            = ((IData)(1U) + vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__i);
    }
    __Vtemp_1[0U] = 0x2e6d656dU;
    __Vtemp_1[1U] = 0x5f313662U;
    __Vtemp_1[2U] = 0x34303936U;
    __Vtemp_1[3U] = 0x7174725fU;
    __Vtemp_1[4U] = 0x696e655fU;
    __Vtemp_1[5U] = 0x73U;
    VL_READMEM_N(true, 16, 4096, 0, VL_CVT_PACK_STR_NW(6, __Vtemp_1)
                 ,  &(vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__rom)
                 , 0, ~0ULL);
}

VL_ATTR_COLD void Vtop___024root___eval_final(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_final\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__stl(Vtop___024root* vlSelf);
#endif  // VL_DEBUG
VL_ATTR_COLD bool Vtop___024root___eval_phase__stl(Vtop___024root* vlSelf);

VL_ATTR_COLD void Vtop___024root___eval_settle(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_settle\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Init
    IData/*31:0*/ __VstlIterCount;
    CData/*0:0*/ __VstlContinue;
    // Body
    __VstlIterCount = 0U;
    vlSelfRef.__VstlFirstIteration = 1U;
    __VstlContinue = 1U;
    while (__VstlContinue) {
        if (VL_UNLIKELY(((0x64U < __VstlIterCount)))) {
#ifdef VL_DEBUG
            Vtop___024root___dump_triggers__stl(vlSelf);
#endif
            VL_FATAL_MT("/home/nolan/Projects/Pound-Drever-Hall/core/hw/prj/pdh_core/rtl/pdh_core.sv", 5, "", "Settle region did not converge.");
        }
        __VstlIterCount = ((IData)(1U) + __VstlIterCount);
        __VstlContinue = 0U;
        if (Vtop___024root___eval_phase__stl(vlSelf)) {
            __VstlContinue = 1U;
        }
        vlSelfRef.__VstlFirstIteration = 0U;
    }
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__stl(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___dump_triggers__stl\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if ((1U & (~ vlSelfRef.__VstlTriggered.any()))) {
        VL_DBG_MSGF("         No triggers active\n");
    }
    if ((1ULL & vlSelfRef.__VstlTriggered.word(0U))) {
        VL_DBG_MSGF("         'stl' region trigger index 0 is active: Internal 'stl' trigger - first iteration\n");
    }
}
#endif  // VL_DEBUG

VL_ATTR_COLD void Vtop___024root___stl_sequent__TOP__0(Vtop___024root* vlSelf);

VL_ATTR_COLD void Vtop___024root___eval_stl(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_stl\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if ((1ULL & vlSelfRef.__VstlTriggered.word(0U))) {
        Vtop___024root___stl_sequent__TOP__0(vlSelf);
    }
}

VL_ATTR_COLD void Vtop___024root___stl_sequent__TOP__0(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___stl_sequent__TOP__0\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Init
    SData/*13:0*/ __Vfunc_pdh_core__DOT__s16_to_u14__0__Vfuncout;
    __Vfunc_pdh_core__DOT__s16_to_u14__0__Vfuncout = 0;
    SData/*15:0*/ __Vfunc_pdh_core__DOT__s16_to_u14__0__in;
    __Vfunc_pdh_core__DOT__s16_to_u14__0__in = 0;
    SData/*15:0*/ __Vfunc_pdh_core__DOT__s16_to_u14__0__t1;
    __Vfunc_pdh_core__DOT__s16_to_u14__0__t1 = 0;
    SData/*13:0*/ __Vfunc_pdh_core__DOT__s16_to_u14__1__Vfuncout;
    __Vfunc_pdh_core__DOT__s16_to_u14__1__Vfuncout = 0;
    SData/*15:0*/ __Vfunc_pdh_core__DOT__s16_to_u14__1__in;
    __Vfunc_pdh_core__DOT__s16_to_u14__1__in = 0;
    SData/*15:0*/ __Vfunc_pdh_core__DOT__s16_to_u14__1__t1;
    __Vfunc_pdh_core__DOT__s16_to_u14__1__t1 = 0;
    IData/*31:0*/ __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__Vfuncout;
    __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__Vfuncout = 0;
    QData/*32:0*/ __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__in;
    __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__in = 0;
    IData/*31:0*/ __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__threshold;
    __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__threshold = 0;
    SData/*13:0*/ __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__Vfuncout;
    __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__Vfuncout = 0;
    IData/*19:0*/ __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__x;
    __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__x = 0;
    // Body
    vlSelfRef.pdh_core__DOT__next_dma_enable_w = ((1U 
                                                   & ((IData)(vlSelfRef.pdh_core__DOT__dma_state_r) 
                                                      >> 1U)) 
                                                  && (1U 
                                                      & (~ (IData)(vlSelfRef.pdh_core__DOT__dma_state_r))));
    vlSelfRef.pdh_core__DOT__next_bram_enable_w = (
                                                   (1U 
                                                    & (~ 
                                                       ((IData)(vlSelfRef.pdh_core__DOT__dma_state_r) 
                                                        >> 1U))) 
                                                   && (1U 
                                                       & (IData)(vlSelfRef.pdh_core__DOT__dma_state_r)));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__enable_i = vlSelfRef.pdh_core__DOT__nco_en_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__stride_i = vlSelfRef.pdh_core__DOT__nco_stride_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__shift_i = vlSelfRef.pdh_core__DOT__nco_shift_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__invert_i = vlSelfRef.pdh_core__DOT__nco_inv_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__sub_i = vlSelfRef.pdh_core__DOT__nco_sub_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__kp_i = vlSelfRef.pdh_core__DOT__kp_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__kd_i = vlSelfRef.pdh_core__DOT__kd_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__ki_i = vlSelfRef.pdh_core__DOT__ki_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__decimate_i 
        = vlSelfRef.pdh_core__DOT__dec_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__sp_i = vlSelfRef.pdh_core__DOT__sp_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__alpha_i = vlSelfRef.pdh_core__DOT__alpha_r;
    vlSelfRef.pdh_core__DOT__next_dac_sel_w = (1U & 
                                               (~ (IData)(vlSelfRef.pdh_core__DOT__dac_sel_r)));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__addr1 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__addr2 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_r;
    vlSelfRef.adc_csn_o = vlSelfRef.pdh_core__DOT__adc_csn_o;
    vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__rst 
        = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__rst_i = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__rst = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__rst 
        = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__rst 
        = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__dma_ready_i = vlSelfRef.dma_ready_i;
    vlSelfRef.pdh_core__DOT__bram_ready_i = vlSelfRef.bram_ready_i;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__next_phi1_w 
        = (0x3fffU & ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_r) 
                      + (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__stride_r)));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__next_cnt_w 
        = (((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__cnt_r) 
            >= ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__decimate_r) 
                - (IData)(1U))) ? 0U : (0x3fffU & ((IData)(1U) 
                                                   + (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__cnt_r))));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_w = (0x3fffU 
                                                   & ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__sub_r)
                                                       ? 
                                                      ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_r) 
                                                       - (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__shift_r))
                                                       : 
                                                      ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_r) 
                                                       + (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__shift_r))));
    vlSelfRef.pdh_core__DOT__led_o = vlSelfRef.pdh_core__DOT__led_r;
    vlSelfRef.pdh_core__DOT__dma_decimation_code_o 
        = vlSelfRef.pdh_core__DOT__dma_decimation_code_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__satwidth_i 
        = vlSelfRef.pdh_core__DOT__satwidth_r;
    vlSelfRef.pdh_core__DOT__dac_sel_o = vlSelfRef.pdh_core__DOT__dac_sel_r;
    vlSelfRef.pdh_core__DOT__dac_rst_o = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__axi_to_ps_o = vlSelfRef.pdh_core__DOT__callback_r;
    vlSelfRef.pdh_core__DOT__dma_enable_o = vlSelfRef.pdh_core__DOT__dma_enable_r;
    vlSelfRef.pdh_core__DOT__bram_enable_o = vlSelfRef.pdh_core__DOT__bram_enable_r;
    vlSelfRef.pdh_core__DOT__dma_data_o = vlSelfRef.pdh_core__DOT__dma_data_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__out1_o = vlSelfRef.pdh_core__DOT__u_nco__DOT__rom1_signed_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__out2_o = vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_signed_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__enable_i = vlSelfRef.pdh_core__DOT__pid_enable_r;
    __Vfunc_pdh_core__DOT__s16_to_u14__0__in = vlSelfRef.pdh_core__DOT__nco_out1_r;
    __Vfunc_pdh_core__DOT__s16_to_u14__0__t1 = (0xffffU 
                                                & ((IData)(0x1fffU) 
                                                   + 
                                                   (- 
                                                    VL_SHIFTR_III(16,16,32, (IData)(__Vfunc_pdh_core__DOT__s16_to_u14__0__in), 2U))));
    __Vfunc_pdh_core__DOT__s16_to_u14__0__Vfuncout 
        = (0x3fffU & (IData)(__Vfunc_pdh_core__DOT__s16_to_u14__0__t1));
    vlSelfRef.pdh_core__DOT__nco_feed1_w = __Vfunc_pdh_core__DOT__s16_to_u14__0__Vfuncout;
    __Vfunc_pdh_core__DOT__s16_to_u14__1__in = vlSelfRef.pdh_core__DOT__nco_out2_r;
    __Vfunc_pdh_core__DOT__s16_to_u14__1__t1 = (0xffffU 
                                                & ((IData)(0x1fffU) 
                                                   + 
                                                   (- 
                                                    VL_SHIFTR_III(16,16,32, (IData)(__Vfunc_pdh_core__DOT__s16_to_u14__1__in), 2U))));
    __Vfunc_pdh_core__DOT__s16_to_u14__1__Vfuncout 
        = (0x3fffU & (IData)(__Vfunc_pdh_core__DOT__s16_to_u14__1__t1));
    vlSelfRef.pdh_core__DOT__nco_feed2_w = __Vfunc_pdh_core__DOT__s16_to_u14__1__Vfuncout;
    vlSelfRef.pdh_core__DOT__i_rot_w = (0x1ffffffffULL 
                                        & (VL_MULS_QQQ(33, 
                                                       (0x1ffffffffULL 
                                                        & VL_EXTENDS_QI(33,16, (IData)(vlSelfRef.pdh_core__DOT__rot_cos_theta_r))), 
                                                       (0x1ffffffffULL 
                                                        & VL_EXTENDS_QI(33,16, (IData)(vlSelfRef.pdh_core__DOT__adc_dat_a_16s_r)))) 
                                           - VL_MULS_QQQ(33, 
                                                         (0x1ffffffffULL 
                                                          & VL_EXTENDS_QI(33,16, (IData)(vlSelfRef.pdh_core__DOT__rot_sin_theta_r))), 
                                                         (0x1ffffffffULL 
                                                          & VL_EXTENDS_QI(33,16, (IData)(vlSelfRef.pdh_core__DOT__adc_dat_b_16s_r))))));
    vlSelfRef.pdh_core__DOT__i_rot_sat_w = ((1U & VL_REDXOR_64(
                                                               (0x180000000ULL 
                                                                & vlSelfRef.pdh_core__DOT__i_rot_w)))
                                             ? ((1U 
                                                 & (IData)(
                                                           (vlSelfRef.pdh_core__DOT__i_rot_w 
                                                            >> 0x1fU)))
                                                 ? 0x80000000U
                                                 : 0x7fffffffU)
                                             : (IData)(vlSelfRef.pdh_core__DOT__i_rot_w));
    vlSelfRef.pdh_core__DOT__i_feed_w = (0xffffU & 
                                         VL_SHIFTRS_III(16,32,32, vlSelfRef.pdh_core__DOT__i_rot_sat_w, 0xfU));
    vlSelfRef.pdh_core__DOT__q_rot_w = (0x1ffffffffULL 
                                        & (VL_MULS_QQQ(33, 
                                                       (0x1ffffffffULL 
                                                        & VL_EXTENDS_QI(33,16, (IData)(vlSelfRef.pdh_core__DOT__rot_sin_theta_r))), 
                                                       (0x1ffffffffULL 
                                                        & VL_EXTENDS_QI(33,16, (IData)(vlSelfRef.pdh_core__DOT__adc_dat_a_16s_r)))) 
                                           + VL_MULS_QQQ(33, 
                                                         (0x1ffffffffULL 
                                                          & VL_EXTENDS_QI(33,16, (IData)(vlSelfRef.pdh_core__DOT__rot_cos_theta_r))), 
                                                         (0x1ffffffffULL 
                                                          & VL_EXTENDS_QI(33,16, (IData)(vlSelfRef.pdh_core__DOT__adc_dat_b_16s_r))))));
    vlSelfRef.pdh_core__DOT__q_rot_sat_w = ((1U & VL_REDXOR_64(
                                                               (0x180000000ULL 
                                                                & vlSelfRef.pdh_core__DOT__q_rot_w)))
                                             ? ((1U 
                                                 & (IData)(
                                                           (vlSelfRef.pdh_core__DOT__q_rot_w 
                                                            >> 0x1fU)))
                                                 ? 0x80000000U
                                                 : 0x7fffffffU)
                                             : (IData)(vlSelfRef.pdh_core__DOT__q_rot_w));
    vlSelfRef.pdh_core__DOT__q_feed_w = (0xffffU & 
                                         VL_SHIFTRS_III(16,32,32, vlSelfRef.pdh_core__DOT__q_rot_sat_w, 0xfU));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__rom1_lookup_w 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__dout1;
    vlSelfRef.pdh_core__DOT__axi_from_ps_i = vlSelfRef.axi_from_ps_i;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_lookup_w 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__dout2;
    vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__D 
        = vlSelfRef.pdh_core__DOT__bram_ready_r;
    vlSelfRef.pdh_core__DOT__led_cb_w = (0x10000000U 
                                         | (IData)(vlSelfRef.pdh_core__DOT__led_r));
    vlSelfRef.pdh_core__DOT__strobe_bit_w = (1U & (vlSelfRef.pdh_core__DOT__axi_3ff_r 
                                                   >> 0x1eU));
    vlSelfRef.pdh_core__DOT__dac_cb_w = (0x20000000U 
                                         | (((IData)(vlSelfRef.pdh_core__DOT__dac1_dat_r) 
                                             << 0xeU) 
                                            | (IData)(vlSelfRef.pdh_core__DOT__dac2_dat_r)));
    vlSelfRef.pdh_core__DOT__set_rot_cb_w = (0x50000000U 
                                             | ((0xfffc000U 
                                                 & ((IData)(vlSelfRef.pdh_core__DOT__sin_theta_r) 
                                                    << 0xcU)) 
                                                | (0x3fffU 
                                                   & ((IData)(vlSelfRef.pdh_core__DOT__cos_theta_r) 
                                                      >> 2U))));
    vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__D 
        = vlSelfRef.pdh_core__DOT__dma_ready_3ff;
    vlSelfRef.pdh_core__DOT__commit_rot_cb_w = (0x60000000U 
                                                | ((0xfffc000U 
                                                    & ((IData)(vlSelfRef.pdh_core__DOT__q_feed_r) 
                                                       << 0xcU)) 
                                                   | (0x3fffU 
                                                      & ((IData)(vlSelfRef.pdh_core__DOT__i_feed_r) 
                                                         >> 2U))));
    vlSelfRef.pdh_core__DOT__config_io_cb_w = (0xe0000000U 
                                               | (((IData)(vlSelfRef.pdh_core__DOT__pid_sel_r) 
                                                   << 6U) 
                                                  | (((IData)(vlSelfRef.pdh_core__DOT__dac2_dat_sel_r) 
                                                      << 3U) 
                                                     | (IData)(vlSelfRef.pdh_core__DOT__dac1_dat_sel_r))));
    vlSelfRef.pdh_core__DOT__get_frame_cb_w = (0x70000000U 
                                               | (((IData)(vlSelfRef.pdh_core__DOT__frame_code_r) 
                                                   << 0x17U) 
                                                  | ((vlSelfRef.pdh_core__DOT__dma_decimation_code_r 
                                                      << 1U) 
                                                     | (IData)(vlSelfRef.pdh_core__DOT__dma_ready_3ff))));
    vlSelfRef.pdh_core__DOT__set_nco_cb_w = (0x90000000U 
                                             | (((IData)(vlSelfRef.pdh_core__DOT__nco_stride_r) 
                                                 << 0xfU) 
                                                | (((IData)(vlSelfRef.pdh_core__DOT__nco_shift_r) 
                                                    << 3U) 
                                                   | (((IData)(vlSelfRef.pdh_core__DOT__nco_sub_r) 
                                                       << 2U) 
                                                      | (((IData)(vlSelfRef.pdh_core__DOT__nco_inv_r) 
                                                          << 1U) 
                                                         | (IData)(vlSelfRef.pdh_core__DOT__nco_en_r))))));
    vlSelfRef.pdh_core__DOT__clk = vlSelfRef.clk;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_o = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_o = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__err_tap = vlSelfRef.pdh_core__DOT__u_pid__DOT__error_pipe1_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_o = vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_pipe1_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_o = vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_pipe1_r;
    vlSelfRef.pdh_core__DOT__base_bus = (0x4000U | 
                                         (0x1fU & vlSelfRef.pdh_core__DOT__axi_from_ps_r));
    vlSelfRef.pdh_core__DOT__cs_cb_w = ((0x10U & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                         ? ((8U & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                             ? ((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                << 0x10U)
                                             : ((4U 
                                                 & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                 ? 
                                                ((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                 << 0x10U)
                                                 : 
                                                ((2U 
                                                  & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                  ? 
                                                 ((1U 
                                                   & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                   ? 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__pid_sel_r))
                                                   : 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__dac2_dat_sel_r)))
                                                  : 
                                                 ((1U 
                                                   & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                   ? 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__dac1_dat_sel_r))
                                                   : 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (((IData)(vlSelfRef.pdh_core__DOT__pid_sel_r) 
                                                       << 6U) 
                                                      | (((IData)(vlSelfRef.pdh_core__DOT__dac2_dat_sel_r) 
                                                          << 3U) 
                                                         | (IData)(vlSelfRef.pdh_core__DOT__dac1_dat_sel_r))))))))
                                         : ((8U & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                             ? ((4U 
                                                 & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                 ? 
                                                ((2U 
                                                  & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                  ? 
                                                 ((1U 
                                                   & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                   ? 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (((IData)(vlSelfRef.pdh_core__DOT__satwidth_r) 
                                                       << 5U) 
                                                      | (((IData)(vlSelfRef.pdh_core__DOT__alpha_r) 
                                                          << 1U) 
                                                         | (IData)(vlSelfRef.pdh_core__DOT__pid_enable_r))))
                                                   : 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | ((0xc000U 
                                                       & ((- (IData)(
                                                                     (1U 
                                                                      & ((IData)(vlSelfRef.pdh_core__DOT__sp_r) 
                                                                         >> 0xdU)))) 
                                                          << 0xeU)) 
                                                      | (IData)(vlSelfRef.pdh_core__DOT__sp_r))))
                                                  : 
                                                 ((1U 
                                                   & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                   ? 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__dec_r))
                                                   : 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__ki_r))))
                                                 : 
                                                ((2U 
                                                  & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                  ? 
                                                 ((1U 
                                                   & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                   ? 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__kd_r))
                                                   : 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__kp_r)))
                                                  : 
                                                 ((1U 
                                                   & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                   ? 
                                                  ((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                   << 0x10U)
                                                   : 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__dma_ready_3ff)))))
                                             : ((4U 
                                                 & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                 ? 
                                                ((2U 
                                                  & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                  ? 
                                                 ((1U 
                                                   & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                   ? 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__q_feed_r))
                                                   : 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__i_feed_r)))
                                                  : 
                                                 ((1U 
                                                   & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                   ? 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__rot_sin_theta_r))
                                                   : 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__rot_cos_theta_r))))
                                                 : 
                                                ((2U 
                                                  & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                  ? 
                                                 ((1U 
                                                   & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                   ? 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__sin_theta_r))
                                                   : 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__cos_theta_r)))
                                                  : 
                                                 ((1U 
                                                   & vlSelfRef.pdh_core__DOT__axi_from_ps_r)
                                                   ? 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__adc_dat_b_16s_r))
                                                   : 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__base_bus) 
                                                    << 0x10U) 
                                                   | (IData)(vlSelfRef.pdh_core__DOT__adc_dat_a_16s_r)))))));
    vlSelfRef.pdh_core__DOT__adc_dat_a_i = vlSelfRef.adc_dat_a_i;
    vlSelfRef.pdh_core__DOT__adc_dat_b_i = vlSelfRef.adc_dat_b_i;
    vlSelfRef.pdh_core__DOT__pid_in_w = ((4U & (IData)(vlSelfRef.pdh_core__DOT__pid_sel_r))
                                          ? (IData)(vlSelfRef.pdh_core__DOT__adc_dat_a_16s_r)
                                          : ((2U & (IData)(vlSelfRef.pdh_core__DOT__pid_sel_r))
                                              ? ((1U 
                                                  & (IData)(vlSelfRef.pdh_core__DOT__pid_sel_r))
                                                  ? (IData)(vlSelfRef.pdh_core__DOT__adc_dat_b_16s_r)
                                                  : (IData)(vlSelfRef.pdh_core__DOT__adc_dat_a_16s_r))
                                              : ((1U 
                                                  & (IData)(vlSelfRef.pdh_core__DOT__pid_sel_r))
                                                  ? (IData)(vlSelfRef.pdh_core__DOT__q_feed_r)
                                                  : (IData)(vlSelfRef.pdh_core__DOT__i_feed_r))));
    vlSelfRef.pdh_core__DOT__cmd_w = (0xfU & (vlSelfRef.pdh_core__DOT__axi_from_ps_r 
                                              >> 0x1aU));
    vlSelfRef.pdh_core__DOT__data_w = (0x3ffffffU & vlSelfRef.pdh_core__DOT__axi_from_ps_r);
    vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_shifted_w 
        = (0xffffU & VL_SHIFTRS_III(16,32,32, vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_r, 0xfU));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_shifted_w 
        = (0xffffU & VL_SHIFTRS_III(16,32,32, vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_r, 0xfU));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_shifted_w 
        = (0xffffU & (IData)((0xffffffffffffULL & VL_SHIFTRS_QQI(48,48,5, vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_r, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__satwidth_r)))));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__total_error_wide_w 
        = (0xfffffU & ((VL_EXTENDS_II(20,16, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_shifted_w)) 
                        + VL_EXTENDS_II(20,16, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_shifted_w))) 
                       + VL_EXTENDS_II(20,16, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_shifted_w))));
    vlSelfRef.led_o = vlSelfRef.pdh_core__DOT__led_o;
    vlSelfRef.dma_decimation_code_o = vlSelfRef.pdh_core__DOT__dma_decimation_code_o;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__next_satwidth_w 
        = ((0xfU <= (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__satwidth_i))
            ? (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__satwidth_i)
            : 0x1fU);
    vlSelfRef.dac_sel_o = vlSelfRef.pdh_core__DOT__dac_sel_o;
    vlSelfRef.dac_rst_o = vlSelfRef.pdh_core__DOT__dac_rst_o;
    vlSelfRef.axi_to_ps_o = vlSelfRef.pdh_core__DOT__axi_to_ps_o;
    vlSelfRef.dma_enable_o = vlSelfRef.pdh_core__DOT__dma_enable_o;
    vlSelfRef.bram_enable_o = vlSelfRef.pdh_core__DOT__bram_enable_o;
    vlSelfRef.dma_data_o = vlSelfRef.pdh_core__DOT__dma_data_o;
    vlSelfRef.pdh_core__DOT__nco_out1_w = vlSelfRef.pdh_core__DOT__u_nco__DOT__out1_o;
    vlSelfRef.pdh_core__DOT__nco_out2_w = vlSelfRef.pdh_core__DOT__u_nco__DOT__out2_o;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__tick1_w = 
        ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__enable_i) 
         & (0U == (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__cnt_r)));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__state1_w = 
        (3U & ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_pipe1_r) 
               >> 0xcU));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_w = 
        (0xfffU & (((0U == (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__state1_w)) 
                    | (2U == (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__state1_w)))
                    ? (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_pipe1_r)
                    : ((IData)(0xfffU) - (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_pipe1_r))));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__rom1_signed_w 
        = (0xffffU & (((0U == (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__state1_w)) 
                       | (1U == (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__state1_w)))
                       ? (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__rom1_lookup_w)
                       : (- (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__rom1_lookup_w))));
    vlSelfRef.pdh_core__DOT__rst_i = (vlSelfRef.pdh_core__DOT__axi_from_ps_i 
                                      >> 0x1fU);
    vlSelfRef.pdh_core__DOT__u_nco__DOT__state2_w = 
        (3U & ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_pipe1_r) 
               >> 0xcU));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_w = 
        (0xfffU & (((0U == (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__state2_w)) 
                    | (2U == (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__state2_w)))
                    ? (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_pipe1_r)
                    : ((IData)(0xfffU) - (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_pipe1_r))));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_intermediate_w 
        = (0xffffU & (((0U == (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__state2_w)) 
                       | (1U == (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__state2_w)))
                       ? (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_lookup_w)
                       : (- (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_lookup_w))));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_signed_w 
        = (0xffffU & ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__invert_r)
                       ? (- (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_intermediate_w))
                       : (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_intermediate_w)));
    vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__Q 
        = ((~ (IData)(vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__prev_D)) 
           & (IData)(vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__D));
    vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__D 
        = vlSelfRef.pdh_core__DOT__strobe_bit_w;
    vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__Q 
        = ((~ (IData)(vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__prev_D)) 
           & (IData)(vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__D));
    vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__clk 
        = vlSelfRef.pdh_core__DOT__clk;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__clk = vlSelfRef.pdh_core__DOT__clk;
    vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__clk 
        = vlSelfRef.pdh_core__DOT__clk;
    vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__clk 
        = vlSelfRef.pdh_core__DOT__clk;
    vlSelfRef.pdh_core__DOT__dac_wrt_o = vlSelfRef.pdh_core__DOT__clk;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__clk = vlSelfRef.pdh_core__DOT__clk;
    vlSelfRef.pdh_core__DOT__addr1_w = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_o;
    vlSelfRef.pdh_core__DOT__addr2_w = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_o;
    vlSelfRef.pdh_core__DOT__err_tap_w = vlSelfRef.pdh_core__DOT__u_pid__DOT__err_tap;
    vlSelfRef.pdh_core__DOT__phi1_w = vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_o;
    vlSelfRef.pdh_core__DOT__phi2_w = vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_o;
    vlSelfRef.pdh_core__DOT__tmp_s_a = (0x7fffU & (- 
                                                   ((IData)(vlSelfRef.pdh_core__DOT__adc_dat_a_i) 
                                                    - (IData)(0x2000U))));
    vlSelfRef.pdh_core__DOT__adc_dat_a_16s_w = ((0x8000U 
                                                 & ((IData)(vlSelfRef.pdh_core__DOT__tmp_s_a) 
                                                    << 1U)) 
                                                | (IData)(vlSelfRef.pdh_core__DOT__tmp_s_a));
    vlSelfRef.pdh_core__DOT__tmp_s_b = (0x7fffU & (- 
                                                   ((IData)(vlSelfRef.pdh_core__DOT__adc_dat_b_i) 
                                                    - (IData)(0x2000U))));
    vlSelfRef.pdh_core__DOT__adc_dat_b_16s_w = ((0x8000U 
                                                 & ((IData)(vlSelfRef.pdh_core__DOT__tmp_s_b) 
                                                    << 1U)) 
                                                | (IData)(vlSelfRef.pdh_core__DOT__tmp_s_b));
    vlSelfRef.pdh_core__DOT__adc_cb_w = (0x30000000U 
                                         | (((IData)(vlSelfRef.pdh_core__DOT__adc_dat_b_i) 
                                             << 0xeU) 
                                            | (IData)(vlSelfRef.pdh_core__DOT__adc_dat_a_i)));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__dat_i = vlSelfRef.pdh_core__DOT__pid_in_w;
    if ((6U == (IData)(vlSelfRef.pdh_core__DOT__cmd_w))) {
        vlSelfRef.pdh_core__DOT__next_rot_sin_theta_w 
            = vlSelfRef.pdh_core__DOT__sin_theta_r;
        vlSelfRef.pdh_core__DOT__next_rot_cos_theta_w 
            = vlSelfRef.pdh_core__DOT__cos_theta_r;
    } else {
        vlSelfRef.pdh_core__DOT__next_rot_sin_theta_w 
            = vlSelfRef.pdh_core__DOT__rot_sin_theta_r;
        vlSelfRef.pdh_core__DOT__next_rot_cos_theta_w 
            = vlSelfRef.pdh_core__DOT__rot_cos_theta_r;
    }
    if ((0xeU == (IData)(vlSelfRef.pdh_core__DOT__cmd_w))) {
        vlSelfRef.pdh_core__DOT__next_dac1_dat_sel_w 
            = (7U & vlSelfRef.pdh_core__DOT__data_w);
        vlSelfRef.pdh_core__DOT__next_dac2_dat_sel_w 
            = (7U & (vlSelfRef.pdh_core__DOT__data_w 
                     >> 3U));
        vlSelfRef.pdh_core__DOT__next_pid_sel_w = (7U 
                                                   & (vlSelfRef.pdh_core__DOT__data_w 
                                                      >> 6U));
    } else {
        vlSelfRef.pdh_core__DOT__next_dac1_dat_sel_w 
            = (7U & (IData)(vlSelfRef.pdh_core__DOT__dac1_dat_sel_r));
        vlSelfRef.pdh_core__DOT__next_dac2_dat_sel_w 
            = (7U & (IData)(vlSelfRef.pdh_core__DOT__dac2_dat_sel_r));
        vlSelfRef.pdh_core__DOT__next_pid_sel_w = (7U 
                                                   & (IData)(vlSelfRef.pdh_core__DOT__pid_sel_r));
    }
    vlSelfRef.pdh_core__DOT__next_led_w = (0xffU & 
                                           ((1U == (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                             ? vlSelfRef.pdh_core__DOT__data_w
                                             : (IData)(vlSelfRef.pdh_core__DOT__led_r)));
    vlSelfRef.pdh_core__DOT__next_dac1_dat_w = (0x3fffU 
                                                & (((~ 
                                                     (vlSelfRef.pdh_core__DOT__data_w 
                                                      >> 0xeU)) 
                                                    & (2U 
                                                       == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)))
                                                    ? vlSelfRef.pdh_core__DOT__data_w
                                                    : (IData)(vlSelfRef.pdh_core__DOT__dac1_dat_r)));
    vlSelfRef.pdh_core__DOT__next_dac2_dat_w = (0x3fffU 
                                                & (((2U 
                                                     == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                    & (vlSelfRef.pdh_core__DOT__data_w 
                                                       >> 0xeU))
                                                    ? vlSelfRef.pdh_core__DOT__data_w
                                                    : (IData)(vlSelfRef.pdh_core__DOT__dac2_dat_r)));
    vlSelfRef.pdh_core__DOT__next_sin_theta_w = (0xffffU 
                                                 & (((5U 
                                                      == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                     & (vlSelfRef.pdh_core__DOT__data_w 
                                                        >> 0x10U))
                                                     ? vlSelfRef.pdh_core__DOT__data_w
                                                     : (IData)(vlSelfRef.pdh_core__DOT__sin_theta_r)));
    vlSelfRef.pdh_core__DOT__next_cos_theta_w = (0xffffU 
                                                 & (((~ 
                                                      (vlSelfRef.pdh_core__DOT__data_w 
                                                       >> 0x10U)) 
                                                     & (5U 
                                                        == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)))
                                                     ? vlSelfRef.pdh_core__DOT__data_w
                                                     : (IData)(vlSelfRef.pdh_core__DOT__cos_theta_r)));
    if ((7U == (IData)(vlSelfRef.pdh_core__DOT__cmd_w))) {
        vlSelfRef.pdh_core__DOT__next_dma_decimation_code_w 
            = (0x3fffffU & vlSelfRef.pdh_core__DOT__data_w);
        vlSelfRef.pdh_core__DOT__next_frame_code_w 
            = (0xfU & (vlSelfRef.pdh_core__DOT__data_w 
                       >> 0x16U));
    } else {
        vlSelfRef.pdh_core__DOT__next_dma_decimation_code_w 
            = (0x3fffffU & vlSelfRef.pdh_core__DOT__dma_decimation_code_r);
        vlSelfRef.pdh_core__DOT__next_frame_code_w 
            = (0xfU & (IData)(vlSelfRef.pdh_core__DOT__frame_code_r));
    }
    vlSelfRef.pdh_core__DOT__nco_coeff_select_w = (7U 
                                                   & (vlSelfRef.pdh_core__DOT__data_w 
                                                      >> 0x10U));
    vlSelfRef.pdh_core__DOT__pid_coeff_select_w = (0xfU 
                                                   & (vlSelfRef.pdh_core__DOT__data_w 
                                                      >> 0x10U));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__perr_tap = vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_shifted_w;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__derr_tap = vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_shifted_w;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__ierr_tap = vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_shifted_w;
    __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__x 
        = (0xfffffU & ((IData)(0x1fffU) + vlSelfRef.pdh_core__DOT__u_pid__DOT__total_error_wide_w));
    __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__Vfuncout 
        = (VL_LTS_III(20, 0x3fffU, __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__x)
            ? 0x3fffU : ((0x80000U & __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__x)
                          ? 0U : (0x3fffU & __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__x)));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__pid_out = __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__Vfuncout;
    vlSelfRef.pdh_core__DOT__rst_o = vlSelfRef.pdh_core__DOT__rst_i;
    vlSelfRef.pdh_core__DOT__bram_ready_edge_w = vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__Q;
    vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__Q 
        = ((~ (IData)(vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__prev_D)) 
           & (IData)(vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__D));
    vlSelfRef.pdh_core__DOT__dma_ready_edge_w = vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__Q;
    vlSelfRef.dac_wrt_o = vlSelfRef.pdh_core__DOT__dac_wrt_o;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__clk 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__clk;
    vlSelfRef.pdh_core__DOT__nco_stride_w = (0xfffU 
                                             & (((9U 
                                                  == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                 & (0U 
                                                    == (IData)(vlSelfRef.pdh_core__DOT__nco_coeff_select_w)))
                                                 ? vlSelfRef.pdh_core__DOT__data_w
                                                 : (IData)(vlSelfRef.pdh_core__DOT__nco_stride_r)));
    vlSelfRef.pdh_core__DOT__nco_shift_w = (0xfffU 
                                            & (((9U 
                                                 == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                & (1U 
                                                   == (IData)(vlSelfRef.pdh_core__DOT__nco_coeff_select_w)))
                                                ? vlSelfRef.pdh_core__DOT__data_w
                                                : (IData)(vlSelfRef.pdh_core__DOT__nco_shift_r)));
    vlSelfRef.pdh_core__DOT__nco_inv_w = (1U & (((9U 
                                                  == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                 & (2U 
                                                    == (IData)(vlSelfRef.pdh_core__DOT__nco_coeff_select_w)))
                                                 ? vlSelfRef.pdh_core__DOT__data_w
                                                 : (IData)(vlSelfRef.pdh_core__DOT__nco_inv_r)));
    vlSelfRef.pdh_core__DOT__nco_sub_w = (1U & (((9U 
                                                  == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                 & (3U 
                                                    == (IData)(vlSelfRef.pdh_core__DOT__nco_coeff_select_w)))
                                                 ? vlSelfRef.pdh_core__DOT__data_w
                                                 : (IData)(vlSelfRef.pdh_core__DOT__nco_sub_r)));
    vlSelfRef.pdh_core__DOT__nco_en_w = (1U & (((9U 
                                                 == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                & (4U 
                                                   == (IData)(vlSelfRef.pdh_core__DOT__nco_coeff_select_w)))
                                                ? vlSelfRef.pdh_core__DOT__data_w
                                                : (IData)(vlSelfRef.pdh_core__DOT__nco_en_r)));
    vlSelfRef.pdh_core__DOT__kp_w = (0xffffU & (((8U 
                                                  == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                 & (0U 
                                                    == (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w)))
                                                 ? vlSelfRef.pdh_core__DOT__data_w
                                                 : (IData)(vlSelfRef.pdh_core__DOT__kp_r)));
    vlSelfRef.pdh_core__DOT__kd_w = (0xffffU & (((8U 
                                                  == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                 & (1U 
                                                    == (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w)))
                                                 ? vlSelfRef.pdh_core__DOT__data_w
                                                 : (IData)(vlSelfRef.pdh_core__DOT__kd_r)));
    vlSelfRef.pdh_core__DOT__ki_w = (0xffffU & (((8U 
                                                  == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                 & (2U 
                                                    == (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w)))
                                                 ? vlSelfRef.pdh_core__DOT__data_w
                                                 : (IData)(vlSelfRef.pdh_core__DOT__ki_r)));
    vlSelfRef.pdh_core__DOT__dec_w = (0x3fffU & (((8U 
                                                   == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                  & (3U 
                                                     == (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w)))
                                                  ? vlSelfRef.pdh_core__DOT__data_w
                                                  : (IData)(vlSelfRef.pdh_core__DOT__dec_r)));
    vlSelfRef.pdh_core__DOT__sp_w = (0x3fffU & (((8U 
                                                  == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                 & (4U 
                                                    == (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w)))
                                                 ? vlSelfRef.pdh_core__DOT__data_w
                                                 : (IData)(vlSelfRef.pdh_core__DOT__sp_r)));
    vlSelfRef.pdh_core__DOT__alpha_w = (0xfU & (((8U 
                                                  == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                 & (5U 
                                                    == (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w)))
                                                 ? vlSelfRef.pdh_core__DOT__data_w
                                                 : (IData)(vlSelfRef.pdh_core__DOT__alpha_r)));
    vlSelfRef.pdh_core__DOT__satwidth_w = (0x1fU & 
                                           (((8U == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                             & (6U 
                                                == (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w)))
                                             ? vlSelfRef.pdh_core__DOT__data_w
                                             : (IData)(vlSelfRef.pdh_core__DOT__satwidth_r)));
    vlSelfRef.pdh_core__DOT__pid_enable_w = (1U & (
                                                   ((8U 
                                                     == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)) 
                                                    & (7U 
                                                       == (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w)))
                                                    ? vlSelfRef.pdh_core__DOT__data_w
                                                    : (IData)(vlSelfRef.pdh_core__DOT__pid_enable_r)));
    vlSelfRef.pdh_core__DOT__pid_payload_w = ((8U & (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w))
                                               ? 0U
                                               : ((4U 
                                                   & (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w))
                                                   ? 
                                                  ((2U 
                                                    & (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w))
                                                     ? (IData)(vlSelfRef.pdh_core__DOT__pid_enable_r)
                                                     : (IData)(vlSelfRef.pdh_core__DOT__satwidth_r))
                                                    : 
                                                   ((1U 
                                                     & (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w))
                                                     ? (IData)(vlSelfRef.pdh_core__DOT__alpha_r)
                                                     : 
                                                    ((0xc000U 
                                                      & ((- (IData)(
                                                                    (1U 
                                                                     & ((IData)(vlSelfRef.pdh_core__DOT__sp_r) 
                                                                        >> 0xdU)))) 
                                                         << 0xeU)) 
                                                     | (IData)(vlSelfRef.pdh_core__DOT__sp_r))))
                                                   : 
                                                  ((2U 
                                                    & (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w))
                                                     ? (IData)(vlSelfRef.pdh_core__DOT__dec_r)
                                                     : (IData)(vlSelfRef.pdh_core__DOT__ki_r))
                                                    : 
                                                   ((1U 
                                                     & (IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w))
                                                     ? (IData)(vlSelfRef.pdh_core__DOT__kd_r)
                                                     : (IData)(vlSelfRef.pdh_core__DOT__kp_r)))));
    vlSelfRef.pdh_core__DOT__perr_tap_w = vlSelfRef.pdh_core__DOT__u_pid__DOT__perr_tap;
    vlSelfRef.pdh_core__DOT__derr_tap_w = vlSelfRef.pdh_core__DOT__u_pid__DOT__derr_tap;
    vlSelfRef.pdh_core__DOT__ierr_tap_w = vlSelfRef.pdh_core__DOT__u_pid__DOT__ierr_tap;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__error_w = 
        (0xffffU & ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__dat_i) 
                    - VL_EXTENDS_II(16,14, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__sp_r))));
    if (vlSelfRef.pdh_core__DOT__u_pid__DOT__tick2_r) {
        vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_w 
            = VL_MULS_III(32, VL_EXTENDS_II(32,16, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__kp_r)), 
                          VL_EXTENDS_II(32,16, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__error_pipe1_r)));
        vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_w 
            = (0xffffffffffffULL & VL_MULS_QQQ(48, 
                                               (0xffffffffffffULL 
                                                & VL_EXTENDS_QI(48,16, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__ki_r))), 
                                               (0xffffffffffffULL 
                                                & VL_EXTENDS_QI(48,32, vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error_r))));
        vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_w 
            = VL_MULS_III(32, VL_EXTENDS_II(32,16, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__kd_r)), 
                          (VL_EXTENDS_II(32,16, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__error_w)) 
                           - VL_EXTENDS_II(32,16, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__yk_r))));
    } else {
        vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_w 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_r;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_w 
            = (0xffffffffffffULL & vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_r);
        vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_w 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_r;
    }
    vlSelfRef.pdh_core__DOT__u_pid__DOT__sat_threshold_w 
        = ((IData)(1U) << (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__satwidth_r));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error_wide_w 
        = (0x1ffffffffULL & ((((QData)((IData)((vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error_r 
                                                >> 0x1fU))) 
                               << 0x20U) | (QData)((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error_r))) 
                             + (((QData)((IData)((0x1ffffU 
                                                  & (- (IData)(
                                                               (1U 
                                                                & ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__error_w) 
                                                                   >> 0xfU))))))) 
                                 << 0x10U) | (QData)((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__error_w)))));
    __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__threshold 
        = vlSelfRef.pdh_core__DOT__u_pid__DOT__sat_threshold_w;
    __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__in 
        = vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error_wide_w;
    __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__Vfuncout 
        = (VL_GTES_IQQ(33, __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__in, (QData)((IData)(__Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__threshold)))
            ? (__Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__threshold 
               - (IData)(1U)) : (VL_LTES_IQQ(33, __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__in, 
                                             (0x100000000ULL 
                                              | (QData)((IData)(
                                                                (- __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__threshold)))))
                                  ? ((IData)(1U) + 
                                     (- __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__threshold))
                                  : (IData)(__Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__in)));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error1_w 
        = __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__Vfuncout;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error2_w 
        = ((((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__tick1_r) 
             & (~ ((0x3fffU == (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__pid_out)) 
                   & VL_GTS_III(32, vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error1_w, vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error_r)))) 
            & (~ ((0U == (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__pid_out)) 
                  & VL_LTS_III(32, vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error1_w, vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error_r))))
            ? vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error1_w
            : vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error_r);
    vlSelfRef.pdh_core__DOT__u_pid__DOT__yk_w = (0xffffU 
                                                 & ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__tick1_r)
                                                     ? 
                                                    (VL_SHIFTRS_III(16,16,4, 
                                                                    (0xffffU 
                                                                     & ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__error_w) 
                                                                        - (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__yk_r))), (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__alpha_r)) 
                                                     + (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__yk_r))
                                                     : (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__yk_r)));
    vlSelfRef.pdh_core__DOT__pid_out_w = vlSelfRef.pdh_core__DOT__u_pid__DOT__pid_out;
    vlSelfRef.rst_o = vlSelfRef.pdh_core__DOT__rst_o;
    vlSelfRef.pdh_core__DOT__next_bram_edge_acquired_w 
        = ((1U & (~ ((IData)(vlSelfRef.pdh_core__DOT__dma_state_r) 
                     >> 1U))) && ((1U & (IData)(vlSelfRef.pdh_core__DOT__dma_state_r)) 
                                  && ((IData)(vlSelfRef.pdh_core__DOT__bram_ready_edge_w) 
                                      | (IData)(vlSelfRef.pdh_core__DOT__bram_edge_acquired_r))));
    vlSelfRef.pdh_core__DOT__strobe_edge_w = vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__Q;
    vlSelfRef.pdh_core__DOT__next_dma_state_w = ((2U 
                                                  & (IData)(vlSelfRef.pdh_core__DOT__dma_state_r))
                                                  ? 
                                                 ((1U 
                                                   & (IData)(vlSelfRef.pdh_core__DOT__dma_state_r))
                                                   ? 
                                                  ((7U 
                                                    == (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                    ? 3U
                                                    : 0U)
                                                   : 
                                                  ((IData)(vlSelfRef.pdh_core__DOT__dma_ready_edge_w)
                                                    ? 3U
                                                    : 2U))
                                                  : 
                                                 ((1U 
                                                   & (IData)(vlSelfRef.pdh_core__DOT__dma_state_r))
                                                   ? 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__bram_edge_acquired_r) 
                                                    & (IData)(vlSelfRef.pdh_core__DOT__dma_ready_3ff))
                                                    ? 2U
                                                    : 1U)
                                                   : 
                                                  (((IData)(vlSelfRef.pdh_core__DOT__bram_ready_r) 
                                                    & (7U 
                                                       == (IData)(vlSelfRef.pdh_core__DOT__cmd_w)))
                                                    ? 1U
                                                    : 0U)));
    vlSelfRef.pdh_core__DOT__set_pid_cb_w = (0x80000000U 
                                             | (((IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w) 
                                                 << 0x10U) 
                                                | (IData)(vlSelfRef.pdh_core__DOT__pid_payload_w)));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_err_tap 
        = vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error2_w;
    vlSelfRef.pdh_core__DOT__dac1_feed_w = ((4U & (IData)(vlSelfRef.pdh_core__DOT__dac1_dat_sel_r))
                                             ? 0x2000U
                                             : ((2U 
                                                 & (IData)(vlSelfRef.pdh_core__DOT__dac1_dat_sel_r))
                                                 ? 
                                                ((1U 
                                                  & (IData)(vlSelfRef.pdh_core__DOT__dac1_dat_sel_r))
                                                  ? (IData)(vlSelfRef.pdh_core__DOT__nco_feed2_r)
                                                  : (IData)(vlSelfRef.pdh_core__DOT__nco_feed1_r))
                                                 : 
                                                ((1U 
                                                  & (IData)(vlSelfRef.pdh_core__DOT__dac1_dat_sel_r))
                                                  ? (IData)(vlSelfRef.pdh_core__DOT__pid_out_w)
                                                  : (IData)(vlSelfRef.pdh_core__DOT__dac1_dat_r))));
    vlSelfRef.pdh_core__DOT__dac2_feed_w = ((4U & (IData)(vlSelfRef.pdh_core__DOT__dac2_dat_sel_r))
                                             ? 0x2000U
                                             : ((2U 
                                                 & (IData)(vlSelfRef.pdh_core__DOT__dac2_dat_sel_r))
                                                 ? 
                                                ((1U 
                                                  & (IData)(vlSelfRef.pdh_core__DOT__dac2_dat_sel_r))
                                                  ? (IData)(vlSelfRef.pdh_core__DOT__nco_feed2_r)
                                                  : (IData)(vlSelfRef.pdh_core__DOT__nco_feed1_r))
                                                 : 
                                                ((1U 
                                                  & (IData)(vlSelfRef.pdh_core__DOT__dac2_dat_sel_r))
                                                  ? (IData)(vlSelfRef.pdh_core__DOT__pid_out_w)
                                                  : (IData)(vlSelfRef.pdh_core__DOT__dac2_dat_r))));
    vlSelfRef.pdh_core__DOT__next_axi_from_ps_w = ((IData)(vlSelfRef.pdh_core__DOT__strobe_edge_w)
                                                    ? vlSelfRef.pdh_core__DOT__axi_3ff_r
                                                    : vlSelfRef.pdh_core__DOT__axi_from_ps_r);
    vlSelfRef.pdh_core__DOT__next_callback_w = ((8U 
                                                 & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                 ? 
                                                ((4U 
                                                  & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                  ? 
                                                 ((2U 
                                                   & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                   ? 
                                                  ((1U 
                                                    & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                    ? 0U
                                                    : vlSelfRef.pdh_core__DOT__config_io_cb_w)
                                                   : 0U)
                                                  : 
                                                 ((2U 
                                                   & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                   ? 0U
                                                   : 
                                                  ((1U 
                                                    & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                    ? vlSelfRef.pdh_core__DOT__set_nco_cb_w
                                                    : vlSelfRef.pdh_core__DOT__set_pid_cb_w)))
                                                 : 
                                                ((4U 
                                                  & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                  ? 
                                                 ((2U 
                                                   & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                   ? 
                                                  ((1U 
                                                    & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                    ? vlSelfRef.pdh_core__DOT__get_frame_cb_w
                                                    : vlSelfRef.pdh_core__DOT__commit_rot_cb_w)
                                                   : 
                                                  ((1U 
                                                    & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                    ? vlSelfRef.pdh_core__DOT__set_rot_cb_w
                                                    : vlSelfRef.pdh_core__DOT__cs_cb_w))
                                                  : 
                                                 ((2U 
                                                   & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                   ? 
                                                  ((1U 
                                                    & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                    ? vlSelfRef.pdh_core__DOT__adc_cb_w
                                                    : vlSelfRef.pdh_core__DOT__dac_cb_w)
                                                   : 
                                                  ((1U 
                                                    & (IData)(vlSelfRef.pdh_core__DOT__cmd_w))
                                                    ? vlSelfRef.pdh_core__DOT__led_cb_w
                                                    : vlSelfRef.pdh_core__DOT__idle_cb_w))));
    vlSelfRef.pdh_core__DOT__sum_err_tap_w = vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_err_tap;
    vlSelfRef.pdh_core__DOT__dac_dat_o = ((IData)(vlSelfRef.pdh_core__DOT__dac_sel_r)
                                           ? (IData)(vlSelfRef.pdh_core__DOT__dac2_feed_w)
                                           : (IData)(vlSelfRef.pdh_core__DOT__dac1_feed_w));
    vlSelfRef.pdh_core__DOT__dma_data_w = ((8U & (IData)(vlSelfRef.pdh_core__DOT__frame_code_r))
                                            ? (((QData)((IData)(
                                                                (((IData)(vlSelfRef.pdh_core__DOT__i_feed_r) 
                                                                  << 0x10U) 
                                                                 | (IData)(vlSelfRef.pdh_core__DOT__q_feed_r)))) 
                                                << 0x20U) 
                                               | (QData)((IData)(
                                                                 (((IData)(vlSelfRef.pdh_core__DOT__adc_dat_a_16s_r) 
                                                                   << 0x10U) 
                                                                  | (IData)(vlSelfRef.pdh_core__DOT__adc_dat_b_16s_r)))))
                                            : ((4U 
                                                & (IData)(vlSelfRef.pdh_core__DOT__frame_code_r))
                                                ? (
                                                   (2U 
                                                    & (IData)(vlSelfRef.pdh_core__DOT__frame_code_r))
                                                    ? 
                                                   (((QData)((IData)(
                                                                     (((IData)(vlSelfRef.pdh_core__DOT__i_feed_r) 
                                                                       << 0x10U) 
                                                                      | (IData)(vlSelfRef.pdh_core__DOT__q_feed_r)))) 
                                                     << 0x20U) 
                                                    | (QData)((IData)(
                                                                      (((IData)(vlSelfRef.pdh_core__DOT__adc_dat_a_16s_r) 
                                                                        << 0x10U) 
                                                                       | (IData)(vlSelfRef.pdh_core__DOT__adc_dat_b_16s_r)))))
                                                    : 
                                                   ((1U 
                                                     & (IData)(vlSelfRef.pdh_core__DOT__frame_code_r))
                                                     ? 
                                                    (((QData)((IData)(
                                                                      (((IData)(vlSelfRef.pdh_core__DOT__dac1_feed_w) 
                                                                        << 0x10U) 
                                                                       | (IData)(vlSelfRef.pdh_core__DOT__dac2_feed_w)))) 
                                                      << 0x20U) 
                                                     | (QData)((IData)(
                                                                       (((IData)(vlSelfRef.pdh_core__DOT__adc_dat_a_i) 
                                                                         << 0x10U) 
                                                                        | (IData)(vlSelfRef.pdh_core__DOT__adc_dat_b_i)))))
                                                     : 
                                                    (((QData)((IData)(
                                                                      (((IData)(vlSelfRef.pdh_core__DOT__addr2_w) 
                                                                        << 0x10U) 
                                                                       | (IData)(vlSelfRef.pdh_core__DOT__addr1_w)))) 
                                                      << 0x20U) 
                                                     | (QData)((IData)(
                                                                       (((IData)(vlSelfRef.pdh_core__DOT__phi2_w) 
                                                                         << 0x10U) 
                                                                        | (IData)(vlSelfRef.pdh_core__DOT__phi1_w)))))))
                                                : (
                                                   (2U 
                                                    & (IData)(vlSelfRef.pdh_core__DOT__frame_code_r))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(vlSelfRef.pdh_core__DOT__frame_code_r))
                                                     ? 
                                                    (((QData)((IData)(
                                                                      (((IData)(vlSelfRef.pdh_core__DOT__nco_feed2_r) 
                                                                        << 0x10U) 
                                                                       | (IData)(vlSelfRef.pdh_core__DOT__nco_feed1_r)))) 
                                                      << 0x20U) 
                                                     | (QData)((IData)(
                                                                       (((IData)(vlSelfRef.pdh_core__DOT__nco_out2_r) 
                                                                         << 0x10U) 
                                                                        | (IData)(vlSelfRef.pdh_core__DOT__nco_out1_r)))))
                                                     : 
                                                    (((QData)((IData)(
                                                                      (((IData)(vlSelfRef.pdh_core__DOT__err_tap_w) 
                                                                        << 0x10U) 
                                                                       | (IData)(vlSelfRef.pdh_core__DOT__pid_out_w)))) 
                                                      << 0x20U) 
                                                     | (QData)((IData)(vlSelfRef.pdh_core__DOT__sum_err_tap_w))))
                                                    : 
                                                   ((1U 
                                                     & (IData)(vlSelfRef.pdh_core__DOT__frame_code_r))
                                                     ? 
                                                    (((QData)((IData)(
                                                                      (((IData)(vlSelfRef.pdh_core__DOT__err_tap_w) 
                                                                        << 0x10U) 
                                                                       | (IData)(vlSelfRef.pdh_core__DOT__perr_tap_w)))) 
                                                      << 0x20U) 
                                                     | (QData)((IData)(
                                                                       (((IData)(vlSelfRef.pdh_core__DOT__derr_tap_w) 
                                                                         << 0x10U) 
                                                                        | (IData)(vlSelfRef.pdh_core__DOT__ierr_tap_w)))))
                                                     : 
                                                    (((QData)((IData)(
                                                                      (((IData)(vlSelfRef.pdh_core__DOT__i_feed_r) 
                                                                        << 0x10U) 
                                                                       | (IData)(vlSelfRef.pdh_core__DOT__q_feed_r)))) 
                                                      << 0x20U) 
                                                     | (QData)((IData)(
                                                                       (((IData)(vlSelfRef.pdh_core__DOT__adc_dat_a_16s_r) 
                                                                         << 0x10U) 
                                                                        | (IData)(vlSelfRef.pdh_core__DOT__adc_dat_b_16s_r)))))))));
    vlSelfRef.dac_dat_o = vlSelfRef.pdh_core__DOT__dac_dat_o;
}

VL_ATTR_COLD void Vtop___024root___eval_triggers__stl(Vtop___024root* vlSelf);

VL_ATTR_COLD bool Vtop___024root___eval_phase__stl(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_phase__stl\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Init
    CData/*0:0*/ __VstlExecute;
    // Body
    Vtop___024root___eval_triggers__stl(vlSelf);
    __VstlExecute = vlSelfRef.__VstlTriggered.any();
    if (__VstlExecute) {
        Vtop___024root___eval_stl(vlSelf);
    }
    return (__VstlExecute);
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__ico(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___dump_triggers__ico\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if ((1U & (~ vlSelfRef.__VicoTriggered.any()))) {
        VL_DBG_MSGF("         No triggers active\n");
    }
    if ((1ULL & vlSelfRef.__VicoTriggered.word(0U))) {
        VL_DBG_MSGF("         'ico' region trigger index 0 is active: Internal 'ico' trigger - first iteration\n");
    }
}
#endif  // VL_DEBUG

#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__act(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___dump_triggers__act\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if ((1U & (~ vlSelfRef.__VactTriggered.any()))) {
        VL_DBG_MSGF("         No triggers active\n");
    }
    if ((1ULL & vlSelfRef.__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 0 is active: @(posedge pdh_core.clk)\n");
    }
    if ((2ULL & vlSelfRef.__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 1 is active: @(posedge pdh_core.rst_i)\n");
    }
    if ((4ULL & vlSelfRef.__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 2 is active: @(negedge pdh_core.clk)\n");
    }
    if ((8ULL & vlSelfRef.__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 3 is active: @(posedge pdh_core.u_strobe_edge_detector.clk)\n");
    }
    if ((0x10ULL & vlSelfRef.__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 4 is active: @(posedge pdh_core.u_strobe_edge_detector.rst)\n");
    }
    if ((0x20ULL & vlSelfRef.__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 5 is active: @(posedge pdh_core.u_nco.clk)\n");
    }
    if ((0x40ULL & vlSelfRef.__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 6 is active: @(posedge pdh_core.u_nco.u_qtr_rom.clk)\n");
    }
    if ((0x80ULL & vlSelfRef.__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 7 is active: @(posedge pdh_core.u_pid.clk)\n");
    }
    if ((0x100ULL & vlSelfRef.__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 8 is active: @(posedge pdh_core.u_dma_edge_detector.clk)\n");
    }
    if ((0x200ULL & vlSelfRef.__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 9 is active: @(posedge pdh_core.u_dma_edge_detector.rst)\n");
    }
    if ((0x400ULL & vlSelfRef.__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 10 is active: @(posedge pdh_core.u_bram_edge_detector.clk)\n");
    }
    if ((0x800ULL & vlSelfRef.__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 11 is active: @(posedge pdh_core.u_bram_edge_detector.rst)\n");
    }
}
#endif  // VL_DEBUG

#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__nba(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___dump_triggers__nba\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if ((1U & (~ vlSelfRef.__VnbaTriggered.any()))) {
        VL_DBG_MSGF("         No triggers active\n");
    }
    if ((1ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 0 is active: @(posedge pdh_core.clk)\n");
    }
    if ((2ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 1 is active: @(posedge pdh_core.rst_i)\n");
    }
    if ((4ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 2 is active: @(negedge pdh_core.clk)\n");
    }
    if ((8ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 3 is active: @(posedge pdh_core.u_strobe_edge_detector.clk)\n");
    }
    if ((0x10ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 4 is active: @(posedge pdh_core.u_strobe_edge_detector.rst)\n");
    }
    if ((0x20ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 5 is active: @(posedge pdh_core.u_nco.clk)\n");
    }
    if ((0x40ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 6 is active: @(posedge pdh_core.u_nco.u_qtr_rom.clk)\n");
    }
    if ((0x80ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 7 is active: @(posedge pdh_core.u_pid.clk)\n");
    }
    if ((0x100ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 8 is active: @(posedge pdh_core.u_dma_edge_detector.clk)\n");
    }
    if ((0x200ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 9 is active: @(posedge pdh_core.u_dma_edge_detector.rst)\n");
    }
    if ((0x400ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 10 is active: @(posedge pdh_core.u_bram_edge_detector.clk)\n");
    }
    if ((0x800ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 11 is active: @(posedge pdh_core.u_bram_edge_detector.rst)\n");
    }
}
#endif  // VL_DEBUG

VL_ATTR_COLD void Vtop___024root___ctor_var_reset(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___ctor_var_reset\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    const uint64_t __VscopeHash = VL_MURMUR64_HASH(vlSelf->name());
    vlSelf->clk = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 16707436170211756652ull);
    vlSelf->adc_dat_a_i = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 15780534314073150592ull);
    vlSelf->adc_dat_b_i = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 1031434511646992610ull);
    vlSelf->adc_csn_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 18142065489574730174ull);
    vlSelf->dac_dat_o = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 7654823914815632422ull);
    vlSelf->dac_wrt_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 12627874194369458556ull);
    vlSelf->dac_rst_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 13562677099374220734ull);
    vlSelf->dac_sel_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 5322877621851162734ull);
    vlSelf->axi_from_ps_i = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 6611040018160084418ull);
    vlSelf->axi_to_ps_o = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 11406050566169109470ull);
    vlSelf->led_o = VL_SCOPED_RAND_RESET_I(8, __VscopeHash, 8501048645136459839ull);
    vlSelf->rst_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 11572211318556052897ull);
    vlSelf->dma_enable_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 13293318731746590756ull);
    vlSelf->bram_enable_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 13800681282705625716ull);
    vlSelf->dma_ready_i = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 9464205343943451354ull);
    vlSelf->bram_ready_i = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 12094824691808237202ull);
    vlSelf->dma_decimation_code_o = VL_SCOPED_RAND_RESET_I(22, __VscopeHash, 12966714883992644525ull);
    vlSelf->dma_data_o = VL_SCOPED_RAND_RESET_Q(64, __VscopeHash, 14195677154923961246ull);
    vlSelf->pdh_core__DOT__clk = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 15634647768972726734ull);
    vlSelf->pdh_core__DOT__adc_dat_a_i = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 5556523560395399890ull);
    vlSelf->pdh_core__DOT__adc_dat_b_i = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 3130779171489333652ull);
    vlSelf->pdh_core__DOT__adc_csn_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 12665807186488738802ull);
    vlSelf->pdh_core__DOT__dac_dat_o = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 2041616415997457210ull);
    vlSelf->pdh_core__DOT__dac_wrt_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 2239482591625916974ull);
    vlSelf->pdh_core__DOT__dac_rst_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 12050194186478342921ull);
    vlSelf->pdh_core__DOT__dac_sel_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 3332648997784650ull);
    vlSelf->pdh_core__DOT__axi_from_ps_i = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 36776846270028347ull);
    vlSelf->pdh_core__DOT__axi_to_ps_o = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 17172104210830879903ull);
    vlSelf->pdh_core__DOT__led_o = VL_SCOPED_RAND_RESET_I(8, __VscopeHash, 12920131100658270249ull);
    vlSelf->pdh_core__DOT__rst_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 3233081796029433873ull);
    vlSelf->pdh_core__DOT__dma_enable_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 13537902832082708452ull);
    vlSelf->pdh_core__DOT__bram_enable_o = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 15031401938540891288ull);
    vlSelf->pdh_core__DOT__dma_ready_i = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 7415232424808762987ull);
    vlSelf->pdh_core__DOT__bram_ready_i = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 5099032361316839716ull);
    vlSelf->pdh_core__DOT__dma_decimation_code_o = VL_SCOPED_RAND_RESET_I(22, __VscopeHash, 10655744532184258625ull);
    vlSelf->pdh_core__DOT__dma_data_o = VL_SCOPED_RAND_RESET_Q(64, __VscopeHash, 9906715001746822587ull);
    vlSelf->pdh_core__DOT__rst_i = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 8785624535028254367ull);
    vlSelf->pdh_core__DOT__axi_from_ps_r = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 14022008624834514281ull);
    vlSelf->pdh_core__DOT__next_axi_from_ps_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 6328865188199936784ull);
    vlSelf->pdh_core__DOT__cmd_w = VL_SCOPED_RAND_RESET_I(4, __VscopeHash, 3048612525161453902ull);
    vlSelf->pdh_core__DOT__strobe_bit_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 10899079946634005927ull);
    vlSelf->pdh_core__DOT__strobe_edge_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 7459162889226549181ull);
    vlSelf->pdh_core__DOT__axi_1ff_r = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 1502435471664572566ull);
    vlSelf->pdh_core__DOT__axi_2ff_r = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 14729540391728515169ull);
    vlSelf->pdh_core__DOT__axi_3ff_r = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 6419902910856554741ull);
    vlSelf->pdh_core__DOT__data_w = VL_SCOPED_RAND_RESET_I(26, __VscopeHash, 17919226882662093140ull);
    vlSelf->pdh_core__DOT__led_r = VL_SCOPED_RAND_RESET_I(8, __VscopeHash, 4792072440552362391ull);
    vlSelf->pdh_core__DOT__next_led_w = VL_SCOPED_RAND_RESET_I(8, __VscopeHash, 15940714437250354965ull);
    vlSelf->pdh_core__DOT__i_feed_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 71566889649232235ull);
    vlSelf->pdh_core__DOT__q_feed_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 13624188855307876723ull);
    vlSelf->pdh_core__DOT__i_feed_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 6241644933015515571ull);
    vlSelf->pdh_core__DOT__q_feed_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 2143762484782567140ull);
    vlSelf->pdh_core__DOT__cos_theta_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 5949329192269616437ull);
    vlSelf->pdh_core__DOT__next_cos_theta_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 12863460803476051387ull);
    vlSelf->pdh_core__DOT__rot_cos_theta_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 13129394814677259370ull);
    vlSelf->pdh_core__DOT__next_rot_cos_theta_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 201681239051759581ull);
    vlSelf->pdh_core__DOT__sin_theta_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 13157272382568583593ull);
    vlSelf->pdh_core__DOT__next_sin_theta_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 1854119652320428567ull);
    vlSelf->pdh_core__DOT__rot_sin_theta_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 3901143907756446800ull);
    vlSelf->pdh_core__DOT__next_rot_sin_theta_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 11192795727178984889ull);
    vlSelf->pdh_core__DOT__kp_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 6633397652313477291ull);
    vlSelf->pdh_core__DOT__kd_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 11205499846042515360ull);
    vlSelf->pdh_core__DOT__ki_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 12595802798055378945ull);
    vlSelf->pdh_core__DOT__kp_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 6104416317648854468ull);
    vlSelf->pdh_core__DOT__kd_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 10373057898706230589ull);
    vlSelf->pdh_core__DOT__ki_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 12066821463390787428ull);
    vlSelf->pdh_core__DOT__sp_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 13857148089711849160ull);
    vlSelf->pdh_core__DOT__sp_r = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 2530597994743726111ull);
    vlSelf->pdh_core__DOT__dec_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 1179797357563879272ull);
    vlSelf->pdh_core__DOT__dec_r = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 4533779972738064093ull);
    vlSelf->pdh_core__DOT__alpha_w = VL_SCOPED_RAND_RESET_I(4, __VscopeHash, 7728020951878182065ull);
    vlSelf->pdh_core__DOT__alpha_r = VL_SCOPED_RAND_RESET_I(4, __VscopeHash, 735037204180386715ull);
    vlSelf->pdh_core__DOT__satwidth_r = VL_SCOPED_RAND_RESET_I(5, __VscopeHash, 7158566258333934773ull);
    vlSelf->pdh_core__DOT__satwidth_w = VL_SCOPED_RAND_RESET_I(5, __VscopeHash, 14569267702179069001ull);
    vlSelf->pdh_core__DOT__pid_enable_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 2590013295249309953ull);
    vlSelf->pdh_core__DOT__pid_enable_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 10765690318256936703ull);
    vlSelf->pdh_core__DOT__pid_out_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 7561937143481004536ull);
    vlSelf->pdh_core__DOT__idle_cb_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 992072369086303289ull);
    vlSelf->pdh_core__DOT__led_cb_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 17046976045640951243ull);
    vlSelf->pdh_core__DOT__dac_cb_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 6462011410638790883ull);
    vlSelf->pdh_core__DOT__adc_cb_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 17179756455972304130ull);
    vlSelf->pdh_core__DOT__cs_cb_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 12079441750233187849ull);
    vlSelf->pdh_core__DOT__set_rot_cb_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 16227148562924779797ull);
    vlSelf->pdh_core__DOT__commit_rot_cb_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 11327445940579926735ull);
    vlSelf->pdh_core__DOT__get_frame_cb_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 297553296815629667ull);
    vlSelf->pdh_core__DOT__set_pid_cb_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 8925861343164329531ull);
    vlSelf->pdh_core__DOT__set_nco_cb_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 13437615624788247877ull);
    vlSelf->pdh_core__DOT__config_io_cb_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 1100514834218443659ull);
    vlSelf->pdh_core__DOT__base_bus = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 4070452173922360839ull);
    vlSelf->pdh_core__DOT__dma_decimation_code_r = VL_SCOPED_RAND_RESET_I(22, __VscopeHash, 8782832803628600229ull);
    vlSelf->pdh_core__DOT__next_dma_decimation_code_w = VL_SCOPED_RAND_RESET_I(22, __VscopeHash, 8049098273233002854ull);
    vlSelf->pdh_core__DOT__pid_coeff_select_w = VL_SCOPED_RAND_RESET_I(4, __VscopeHash, 17830081390238514740ull);
    vlSelf->pdh_core__DOT__pid_payload_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 14732555685321916505ull);
    vlSelf->pdh_core__DOT__callback_r = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 11284639703502385376ull);
    vlSelf->pdh_core__DOT__next_callback_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 1504834451218750961ull);
    vlSelf->pdh_core__DOT__dac1_dat_r = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 16845343492583813422ull);
    vlSelf->pdh_core__DOT__next_dac1_dat_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 13812263342919681532ull);
    vlSelf->pdh_core__DOT__dac2_dat_r = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 9009416067898928941ull);
    vlSelf->pdh_core__DOT__next_dac2_dat_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 14048043727591807662ull);
    vlSelf->pdh_core__DOT__dac_sel_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 2299917187321554536ull);
    vlSelf->pdh_core__DOT__next_dac_sel_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 15618187489943467000ull);
    vlSelf->pdh_core__DOT__nco_en_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 8232815336817557095ull);
    vlSelf->pdh_core__DOT__nco_en_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 10052794725172062042ull);
    vlSelf->pdh_core__DOT__nco_inv_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 18312350295478835076ull);
    vlSelf->pdh_core__DOT__nco_inv_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 9290819704676168906ull);
    vlSelf->pdh_core__DOT__nco_sub_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 10192723247182430042ull);
    vlSelf->pdh_core__DOT__nco_sub_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 10971093977911766734ull);
    vlSelf->pdh_core__DOT__nco_stride_r = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 3035836901285698229ull);
    vlSelf->pdh_core__DOT__nco_stride_w = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 9056078291267362207ull);
    vlSelf->pdh_core__DOT__nco_shift_r = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 8110588772016286368ull);
    vlSelf->pdh_core__DOT__nco_shift_w = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 996897556133800763ull);
    vlSelf->pdh_core__DOT__nco_out1_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 6449539328664396406ull);
    vlSelf->pdh_core__DOT__nco_out2_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 10885528111711313826ull);
    vlSelf->pdh_core__DOT__nco_out1_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 2755391060793628625ull);
    vlSelf->pdh_core__DOT__nco_out2_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 10634263489962878860ull);
    vlSelf->pdh_core__DOT__nco_feed1_r = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 4488032504790612482ull);
    vlSelf->pdh_core__DOT__nco_feed2_r = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 6459204697110485143ull);
    vlSelf->pdh_core__DOT__nco_feed1_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 5687542668348883261ull);
    vlSelf->pdh_core__DOT__nco_feed2_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 15178191024583367553ull);
    vlSelf->pdh_core__DOT__addr1_w = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 16446647671119384910ull);
    vlSelf->pdh_core__DOT__addr2_w = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 2491234801618866701ull);
    vlSelf->pdh_core__DOT__phi1_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 9971184825872044264ull);
    vlSelf->pdh_core__DOT__phi2_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 4396481915142893723ull);
    vlSelf->pdh_core__DOT__pid_sel_r = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 102013608931472323ull);
    vlSelf->pdh_core__DOT__next_pid_sel_w = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 13338671507209993395ull);
    vlSelf->pdh_core__DOT__dac1_dat_sel_r = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 5355047408438230615ull);
    vlSelf->pdh_core__DOT__next_dac1_dat_sel_w = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 14766796626096099177ull);
    vlSelf->pdh_core__DOT__dac2_dat_sel_r = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 1396498158662939393ull);
    vlSelf->pdh_core__DOT__next_dac2_dat_sel_w = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 10137934891688820282ull);
    vlSelf->pdh_core__DOT__dac1_feed_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 3380983589575654620ull);
    vlSelf->pdh_core__DOT__dac2_feed_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 7962842785534771416ull);
    vlSelf->pdh_core__DOT__pid_in_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 7496349469920218870ull);
    vlSelf->pdh_core__DOT__err_tap_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 14680033443025463881ull);
    vlSelf->pdh_core__DOT__perr_tap_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 11707231579677049538ull);
    vlSelf->pdh_core__DOT__derr_tap_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 192579375745652224ull);
    vlSelf->pdh_core__DOT__ierr_tap_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 1666104093832071076ull);
    vlSelf->pdh_core__DOT__sum_err_tap_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 18385279443098660654ull);
    vlSelf->pdh_core__DOT__frame_code_r = VL_SCOPED_RAND_RESET_I(4, __VscopeHash, 8650905665335925556ull);
    vlSelf->pdh_core__DOT__next_frame_code_w = VL_SCOPED_RAND_RESET_I(4, __VscopeHash, 15624969223176452307ull);
    vlSelf->pdh_core__DOT__dma_data_r = VL_SCOPED_RAND_RESET_Q(64, __VscopeHash, 15170594844234750740ull);
    vlSelf->pdh_core__DOT__dma_data_w = VL_SCOPED_RAND_RESET_Q(64, __VscopeHash, 1281532558023088199ull);
    vlSelf->pdh_core__DOT__nco_coeff_select_w = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 7377475669181470068ull);
    vlSelf->pdh_core__DOT__adc_dat_a_16s_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 4356481537771038667ull);
    vlSelf->pdh_core__DOT__adc_dat_b_16s_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 2253047418770793775ull);
    vlSelf->pdh_core__DOT__adc_dat_a_16s_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 14265471589489465545ull);
    vlSelf->pdh_core__DOT__adc_dat_b_16s_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 9046899304331543271ull);
    vlSelf->pdh_core__DOT__tmp_s_a = VL_SCOPED_RAND_RESET_I(15, __VscopeHash, 7523887677465529236ull);
    vlSelf->pdh_core__DOT__tmp_s_b = VL_SCOPED_RAND_RESET_I(15, __VscopeHash, 209163029563305131ull);
    vlSelf->pdh_core__DOT__i_rot_w = VL_SCOPED_RAND_RESET_Q(33, __VscopeHash, 3859492320303516787ull);
    vlSelf->pdh_core__DOT__q_rot_w = VL_SCOPED_RAND_RESET_Q(33, __VscopeHash, 1093251589728089554ull);
    vlSelf->pdh_core__DOT__i_rot_sat_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 15599786448253663297ull);
    vlSelf->pdh_core__DOT__q_rot_sat_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 1433453656242077810ull);
    vlSelf->pdh_core__DOT__dma_ready_edge_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 16299075599665832913ull);
    vlSelf->pdh_core__DOT__dma_ready_1ff = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 5104151169981928194ull);
    vlSelf->pdh_core__DOT__dma_ready_2ff = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 12037609977971385341ull);
    vlSelf->pdh_core__DOT__dma_ready_3ff = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 6739453782635909866ull);
    vlSelf->pdh_core__DOT__bram_ready_edge_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 13429234410946556658ull);
    vlSelf->pdh_core__DOT__bram_ready_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 15423320824148534300ull);
    vlSelf->pdh_core__DOT__bram_edge_acquired_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 11484200311094773184ull);
    vlSelf->pdh_core__DOT__next_bram_edge_acquired_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 1947337722451066382ull);
    vlSelf->pdh_core__DOT__dma_enable_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 14631496748758325394ull);
    vlSelf->pdh_core__DOT__next_dma_enable_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 12971976021609887429ull);
    vlSelf->pdh_core__DOT__bram_enable_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 6329851938859630165ull);
    vlSelf->pdh_core__DOT__next_bram_enable_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 4371844276868471112ull);
    vlSelf->pdh_core__DOT__dma_state_r = VL_SCOPED_RAND_RESET_I(2, __VscopeHash, 17706375702812795028ull);
    vlSelf->pdh_core__DOT__next_dma_state_w = VL_SCOPED_RAND_RESET_I(2, __VscopeHash, 7475602117927306292ull);
    vlSelf->pdh_core__DOT__rst_sync_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 2815537915764905668ull);
    vlSelf->pdh_core__DOT__rst_pipe1_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 1247345736313586242ull);
    vlSelf->pdh_core__DOT__rst_sync_ne_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 6017514086544155691ull);
    vlSelf->pdh_core__DOT__rst_pipe1_ne_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 14658280278088742899ull);
    vlSelf->pdh_core__DOT__u_strobe_edge_detector__DOT__clk = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 16825570225136840240ull);
    vlSelf->pdh_core__DOT__u_strobe_edge_detector__DOT__rst = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 9081962313867197358ull);
    vlSelf->pdh_core__DOT__u_strobe_edge_detector__DOT__D = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 488402352370440364ull);
    vlSelf->pdh_core__DOT__u_strobe_edge_detector__DOT__Q = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 7828858972986823385ull);
    vlSelf->pdh_core__DOT__u_strobe_edge_detector__DOT__prev_D = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 14951581884738634108ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__clk = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 10284574788010354280ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__enable_i = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 13255372193643382673ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__rst_i = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 12011378701984722585ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__stride_i = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 1574046117298759951ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__shift_i = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 2868935468033219497ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__invert_i = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 7417037693344240364ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__sub_i = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 6160653741089075266ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__out1_o = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 4465950462638514190ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__out2_o = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 11879361149013075675ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__addr1_o = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 17262760412850133060ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__addr2_o = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 12682772720768614350ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__phi1_o = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 12601740817381266359ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__phi2_o = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 3628254307512285475ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__stride_r = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 7538119526437891996ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__shift_r = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 15010119737400134786ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__invert_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 8451103269566317147ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__sub_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 2338845591268137607ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__phi1_r = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 8075224505440812601ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__next_phi1_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 510152394260425086ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__phi2_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 16998059015939279410ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__phi1_pipe1_r = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 7290762540173354374ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__phi2_pipe1_r = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 8032725464630847117ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__addr1_w = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 5698080562826103725ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__addr2_w = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 14656457830493006285ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__addr1_r = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 5502630657160143827ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__addr2_r = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 7179861560457472459ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__state1_w = VL_SCOPED_RAND_RESET_I(2, __VscopeHash, 12888306922376769932ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__state2_w = VL_SCOPED_RAND_RESET_I(2, __VscopeHash, 15192682425758975350ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__rom1_lookup_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 6139263298169226586ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__rom2_lookup_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 10521045042564569384ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__rom1_signed_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 1597711754741588176ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__rom1_signed_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 9881669281699721808ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__rom2_intermediate_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 8581474180622477687ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__rom2_signed_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 2080838813580576709ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__rom2_signed_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 6163458796091659936ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__clk = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 1009956201934520014ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__addr1 = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 14880783711995734290ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__addr2 = VL_SCOPED_RAND_RESET_I(12, __VscopeHash, 12114244781600428505ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__dout1 = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 51312236861176458ull);
    vlSelf->pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__dout2 = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 5586030760323016892ull);
    for (int __Vi0 = 0; __Vi0 < 4096; ++__Vi0) {
        vlSelf->pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__rom[__Vi0] = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 13507175268051934870ull);
    }
    vlSelf->pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__i = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 17174375021514492619ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__clk = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 17987723865753938273ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__rst = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 6859911565064527868ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__kp_i = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 109186084785977810ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__kd_i = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 17170876016342399943ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__ki_i = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 15596616851346930685ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__decimate_i = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 17052845116394805364ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__sp_i = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 11177670158548096625ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__alpha_i = VL_SCOPED_RAND_RESET_I(4, __VscopeHash, 15590389628574034540ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__satwidth_i = VL_SCOPED_RAND_RESET_I(5, __VscopeHash, 13861141224440344538ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__dat_i = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 4688950809693037799ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__enable_i = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 5251571047225211189ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__pid_out = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 15940066208219968170ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__err_tap = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 11942398279027175381ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__perr_tap = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 14443658831577205962ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__derr_tap = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 16801012502831222387ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__ierr_tap = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 17573155644324922692ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__sum_err_tap = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 16363268669804273317ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__kp_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 5802738166026225479ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__kd_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 8768581805771851110ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__ki_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 9298120605155452598ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__sp_r = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 2241821637530172703ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__alpha_r = VL_SCOPED_RAND_RESET_I(4, __VscopeHash, 18328792883966013709ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__satwidth_r = VL_SCOPED_RAND_RESET_I(5, __VscopeHash, 16237807731707740951ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__decimate_r = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 18394455706554368259ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__cnt_r = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 4111247864544683696ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__next_cnt_w = VL_SCOPED_RAND_RESET_I(14, __VscopeHash, 16252814737630480797ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__tick1_w = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 3078942224837013832ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__tick1_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 2544805587136455121ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__tick2_r = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 9531910645156066573ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__error_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 14075795912682062897ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__error_pipe1_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 990740343404147534ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__sum_error1_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 8971293539966438900ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__sum_error2_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 3093190910107452961ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__sum_error_r = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 7324968260166902017ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__sat_threshold_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 15223707810821530906ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__sum_error_wide_w = VL_SCOPED_RAND_RESET_Q(33, __VscopeHash, 3491483637498418357ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__yk_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 11858505501328446806ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__yk_r = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 13337300875137808516ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__p_error_r = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 1027025972611252825ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__p_error_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 4212844076558276334ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__d_error_r = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 4567422398149870399ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__d_error_w = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 4431289058474485494ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__i_error_r = VL_SCOPED_RAND_RESET_Q(48, __VscopeHash, 5660592160431793459ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__i_error_w = VL_SCOPED_RAND_RESET_Q(48, __VscopeHash, 7275186075453182857ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__total_error_wide_w = VL_SCOPED_RAND_RESET_I(20, __VscopeHash, 10105172946608486615ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__p_error_shifted_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 10420664662079797882ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__d_error_shifted_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 5241003701117592715ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__i_error_shifted_w = VL_SCOPED_RAND_RESET_I(16, __VscopeHash, 16887619095405613804ull);
    vlSelf->pdh_core__DOT__u_pid__DOT__next_satwidth_w = VL_SCOPED_RAND_RESET_I(5, __VscopeHash, 14615744470953232649ull);
    vlSelf->pdh_core__DOT__u_dma_edge_detector__DOT__clk = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 10527235731711203081ull);
    vlSelf->pdh_core__DOT__u_dma_edge_detector__DOT__rst = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 782647447685307632ull);
    vlSelf->pdh_core__DOT__u_dma_edge_detector__DOT__D = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 10441810498451908181ull);
    vlSelf->pdh_core__DOT__u_dma_edge_detector__DOT__Q = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 67392869245540160ull);
    vlSelf->pdh_core__DOT__u_dma_edge_detector__DOT__prev_D = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 8323957828999702627ull);
    vlSelf->pdh_core__DOT__u_bram_edge_detector__DOT__clk = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 14205271244375333234ull);
    vlSelf->pdh_core__DOT__u_bram_edge_detector__DOT__rst = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 11061190036231468707ull);
    vlSelf->pdh_core__DOT__u_bram_edge_detector__DOT__D = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 10111988705459059541ull);
    vlSelf->pdh_core__DOT__u_bram_edge_detector__DOT__Q = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 9759904660209775837ull);
    vlSelf->pdh_core__DOT__u_bram_edge_detector__DOT__prev_D = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 5182490331562254314ull);
    vlSelf->__Vtrigprevexpr___TOP__pdh_core__DOT__clk__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 9624288118129914440ull);
    vlSelf->__Vtrigprevexpr___TOP__pdh_core__DOT__rst_i__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 693681130968975082ull);
    vlSelf->__Vtrigprevexpr___TOP__pdh_core__DOT__u_strobe_edge_detector__DOT__clk__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 2249965783037420855ull);
    vlSelf->__Vtrigprevexpr___TOP__pdh_core__DOT__u_strobe_edge_detector__DOT__rst__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 16497156514868204967ull);
    vlSelf->__Vtrigprevexpr___TOP__pdh_core__DOT__u_nco__DOT__clk__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 14512496787514585449ull);
    vlSelf->__Vtrigprevexpr___TOP__pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__clk__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 7503944318857911644ull);
    vlSelf->__Vtrigprevexpr___TOP__pdh_core__DOT__u_pid__DOT__clk__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 9093825880591346069ull);
    vlSelf->__Vtrigprevexpr___TOP__pdh_core__DOT__u_dma_edge_detector__DOT__clk__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 7671256852761244639ull);
    vlSelf->__Vtrigprevexpr___TOP__pdh_core__DOT__u_dma_edge_detector__DOT__rst__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 5972454758452474756ull);
    vlSelf->__Vtrigprevexpr___TOP__pdh_core__DOT__u_bram_edge_detector__DOT__clk__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 2779780955970473455ull);
    vlSelf->__Vtrigprevexpr___TOP__pdh_core__DOT__u_bram_edge_detector__DOT__rst__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 9542752312898717183ull);
}
