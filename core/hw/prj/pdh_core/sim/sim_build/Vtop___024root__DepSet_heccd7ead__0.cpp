// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtop.h for the primary calling header

#include "Vtop__pch.h"
#include "Vtop___024root.h"

void Vtop___024root___ico_sequent__TOP__0(Vtop___024root* vlSelf);

void Vtop___024root___eval_ico(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_ico\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if ((1ULL & vlSelfRef.__VicoTriggered.word(0U))) {
        Vtop___024root___ico_sequent__TOP__0(vlSelf);
    }
}

VL_INLINE_OPT void Vtop___024root___ico_sequent__TOP__0(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___ico_sequent__TOP__0\n"); );
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
    vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__rst 
        = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__rst 
        = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__rst 
        = vlSelfRef.pdh_core__DOT__rst_sync_r;
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
    vlSelfRef.pdh_core__DOT__u_nco__DOT__rst_i = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__rst = vlSelfRef.pdh_core__DOT__rst_sync_r;
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
    vlSelfRef.pdh_core__DOT__u_nco__DOT__clk = vlSelfRef.pdh_core__DOT__clk;
    vlSelfRef.pdh_core__DOT__dac_wrt_o = vlSelfRef.pdh_core__DOT__clk;
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
    vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__clk 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__clk;
    vlSelfRef.dac_wrt_o = vlSelfRef.pdh_core__DOT__dac_wrt_o;
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

void Vtop___024root___eval_triggers__ico(Vtop___024root* vlSelf);

bool Vtop___024root___eval_phase__ico(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_phase__ico\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Init
    CData/*0:0*/ __VicoExecute;
    // Body
    Vtop___024root___eval_triggers__ico(vlSelf);
    __VicoExecute = vlSelfRef.__VicoTriggered.any();
    if (__VicoExecute) {
        Vtop___024root___eval_ico(vlSelf);
    }
    return (__VicoExecute);
}

void Vtop___024root___eval_act(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_act\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
}

void Vtop___024root___nba_sequent__TOP__0(Vtop___024root* vlSelf);
void Vtop___024root___nba_sequent__TOP__1(Vtop___024root* vlSelf);
void Vtop___024root___nba_sequent__TOP__2(Vtop___024root* vlSelf);
void Vtop___024root___nba_sequent__TOP__3(Vtop___024root* vlSelf);
void Vtop___024root___nba_sequent__TOP__4(Vtop___024root* vlSelf);
void Vtop___024root___nba_sequent__TOP__5(Vtop___024root* vlSelf);
void Vtop___024root___nba_sequent__TOP__6(Vtop___024root* vlSelf);
void Vtop___024root___nba_sequent__TOP__7(Vtop___024root* vlSelf);
void Vtop___024root___nba_comb__TOP__0(Vtop___024root* vlSelf);
void Vtop___024root___nba_sequent__TOP__8(Vtop___024root* vlSelf);
void Vtop___024root___nba_sequent__TOP__9(Vtop___024root* vlSelf);
void Vtop___024root___nba_comb__TOP__1(Vtop___024root* vlSelf);
void Vtop___024root___nba_comb__TOP__2(Vtop___024root* vlSelf);
void Vtop___024root___nba_comb__TOP__3(Vtop___024root* vlSelf);
void Vtop___024root___nba_comb__TOP__4(Vtop___024root* vlSelf);
void Vtop___024root___nba_comb__TOP__5(Vtop___024root* vlSelf);
void Vtop___024root___nba_comb__TOP__6(Vtop___024root* vlSelf);
void Vtop___024root___nba_sequent__TOP__10(Vtop___024root* vlSelf);
void Vtop___024root___nba_comb__TOP__7(Vtop___024root* vlSelf);

void Vtop___024root___eval_nba(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_nba\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if ((0x80ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_sequent__TOP__0(vlSelf);
    }
    if ((0x40ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_sequent__TOP__1(vlSelf);
    }
    if ((0x18ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_sequent__TOP__2(vlSelf);
    }
    if ((0xc00ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_sequent__TOP__3(vlSelf);
    }
    if ((0x300ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_sequent__TOP__4(vlSelf);
    }
    if ((0x20ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_sequent__TOP__5(vlSelf);
    }
    if ((4ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_sequent__TOP__6(vlSelf);
    }
    if ((1ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_sequent__TOP__7(vlSelf);
    }
    if ((0x60ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_comb__TOP__0(vlSelf);
    }
    if ((6ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_sequent__TOP__8(vlSelf);
    }
    if ((0x20ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_sequent__TOP__9(vlSelf);
    }
    if ((0x85ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_comb__TOP__1(vlSelf);
    }
    if ((0xc01ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_comb__TOP__2(vlSelf);
    }
    if ((0x81ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_comb__TOP__3(vlSelf);
    }
    if ((0x301ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_comb__TOP__4(vlSelf);
    }
    if ((5ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_comb__TOP__5(vlSelf);
    }
    if ((0x19ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_comb__TOP__6(vlSelf);
    }
    if ((3ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_sequent__TOP__10(vlSelf);
    }
    if ((0xa5ULL & vlSelfRef.__VnbaTriggered.word(0U))) {
        Vtop___024root___nba_comb__TOP__7(vlSelf);
    }
}

VL_INLINE_OPT void Vtop___024root___nba_sequent__TOP__0(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_sequent__TOP__0\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Init
    SData/*13:0*/ __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__Vfuncout;
    __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__Vfuncout = 0;
    IData/*19:0*/ __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__x;
    __Vfunc_pdh_core__DOT__u_pid__DOT__sat_unsigned_from_signed__3__x = 0;
    // Body
    if (vlSelfRef.pdh_core__DOT__u_pid__DOT__rst) {
        vlSelfRef.pdh_core__DOT__u_pid__DOT__decimate_r = 1U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__ki_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__kp_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__alpha_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__sp_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__kd_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__satwidth_r = 0x1fU;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__cnt_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__error_pipe1_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__yk_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_r = 0ULL;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_r = 0U;
    } else if (vlSelfRef.pdh_core__DOT__u_pid__DOT__enable_i) {
        vlSelfRef.pdh_core__DOT__u_pid__DOT__decimate_r 
            = (0x3fffU & ((1U > (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__decimate_i))
                           ? 1U : (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__decimate_i)));
        vlSelfRef.pdh_core__DOT__u_pid__DOT__ki_r = vlSelfRef.pdh_core__DOT__u_pid__DOT__ki_i;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__kp_r = vlSelfRef.pdh_core__DOT__u_pid__DOT__kp_i;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__alpha_r 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__alpha_i;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__sp_r = vlSelfRef.pdh_core__DOT__u_pid__DOT__sp_i;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__kd_r = vlSelfRef.pdh_core__DOT__u_pid__DOT__kd_i;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__satwidth_r 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__next_satwidth_w;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__cnt_r 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__next_cnt_w;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__error_pipe1_r 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__error_w;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__yk_r = vlSelfRef.pdh_core__DOT__u_pid__DOT__yk_w;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error_r 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error2_w;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_r 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_w;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_r 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_w;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_r 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_w;
    } else {
        vlSelfRef.pdh_core__DOT__u_pid__DOT__decimate_r 
            = (0x3fffU & (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__decimate_r));
        vlSelfRef.pdh_core__DOT__u_pid__DOT__ki_r = vlSelfRef.pdh_core__DOT__u_pid__DOT__ki_r;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__kp_r = vlSelfRef.pdh_core__DOT__u_pid__DOT__kp_r;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__alpha_r 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__alpha_r;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__sp_r = vlSelfRef.pdh_core__DOT__u_pid__DOT__sp_r;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__kd_r = vlSelfRef.pdh_core__DOT__u_pid__DOT__kd_r;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__satwidth_r 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__satwidth_r;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__cnt_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__error_pipe1_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__yk_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_r = 0U;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_r = 0ULL;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_r = 0U;
    }
    vlSelfRef.pdh_core__DOT__u_pid__DOT__tick2_r = 
        ((1U & (~ (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__rst))) 
         && ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__enable_i) 
             && (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__tick1_r)));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__next_cnt_w 
        = (((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__cnt_r) 
            >= ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__decimate_r) 
                - (IData)(1U))) ? 0U : (0x3fffU & ((IData)(1U) 
                                                   + (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__cnt_r))));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__err_tap = vlSelfRef.pdh_core__DOT__u_pid__DOT__error_pipe1_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__tick1_r = 
        ((1U & (~ (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__rst))) 
         && ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__enable_i) 
             && (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__tick1_w)));
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
    } else {
        vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_w 
            = vlSelfRef.pdh_core__DOT__u_pid__DOT__p_error_r;
        vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_w 
            = (0xffffffffffffULL & vlSelfRef.pdh_core__DOT__u_pid__DOT__i_error_r);
    }
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
    vlSelfRef.pdh_core__DOT__err_tap_w = vlSelfRef.pdh_core__DOT__u_pid__DOT__err_tap;
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
    vlSelfRef.pdh_core__DOT__perr_tap_w = vlSelfRef.pdh_core__DOT__u_pid__DOT__perr_tap;
    vlSelfRef.pdh_core__DOT__derr_tap_w = vlSelfRef.pdh_core__DOT__u_pid__DOT__derr_tap;
    vlSelfRef.pdh_core__DOT__ierr_tap_w = vlSelfRef.pdh_core__DOT__u_pid__DOT__ierr_tap;
    vlSelfRef.pdh_core__DOT__pid_out_w = vlSelfRef.pdh_core__DOT__u_pid__DOT__pid_out;
}

VL_INLINE_OPT void Vtop___024root___nba_sequent__TOP__1(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_sequent__TOP__1\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__dout1 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__rom
        [vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__addr1];
    vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__dout2 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__rom
        [vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__addr2];
    vlSelfRef.pdh_core__DOT__u_nco__DOT__rom1_lookup_w 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__dout1;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_lookup_w 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__dout2;
}

VL_INLINE_OPT void Vtop___024root___nba_sequent__TOP__2(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_sequent__TOP__2\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__prev_D 
        = ((1U & (~ (IData)(vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__rst))) 
           && (IData)(vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__D));
}

VL_INLINE_OPT void Vtop___024root___nba_sequent__TOP__3(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_sequent__TOP__3\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__prev_D 
        = ((1U & (~ (IData)(vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__rst))) 
           && (IData)(vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__D));
}

VL_INLINE_OPT void Vtop___024root___nba_sequent__TOP__4(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_sequent__TOP__4\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__prev_D 
        = ((1U & (~ (IData)(vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__rst))) 
           && (IData)(vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__D));
}

VL_INLINE_OPT void Vtop___024root___nba_sequent__TOP__5(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_sequent__TOP__5\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if (vlSelfRef.pdh_core__DOT__u_nco__DOT__rst_i) {
        vlSelfRef.pdh_core__DOT__u_nco__DOT__stride_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__shift_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__rom1_signed_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_signed_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_pipe1_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_pipe1_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_r = 0U;
    } else if (vlSelfRef.pdh_core__DOT__u_nco__DOT__enable_i) {
        vlSelfRef.pdh_core__DOT__u_nco__DOT__stride_r 
            = vlSelfRef.pdh_core__DOT__u_nco__DOT__stride_i;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__shift_r 
            = vlSelfRef.pdh_core__DOT__u_nco__DOT__shift_i;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__rom1_signed_r 
            = vlSelfRef.pdh_core__DOT__u_nco__DOT__rom1_signed_w;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_signed_r 
            = vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_signed_w;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_r 
            = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_w;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_r 
            = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_w;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_pipe1_r 
            = vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_w;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_pipe1_r 
            = vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_r;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_r 
            = vlSelfRef.pdh_core__DOT__u_nco__DOT__next_phi1_w;
    } else {
        vlSelfRef.pdh_core__DOT__u_nco__DOT__stride_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__shift_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__rom1_signed_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_signed_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_pipe1_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_pipe1_r = 0U;
        vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_r = 0U;
    }
    vlSelfRef.pdh_core__DOT__u_nco__DOT__sub_r = ((1U 
                                                   & (~ (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__rst_i))) 
                                                  && ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__enable_i) 
                                                      && (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__sub_i)));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__invert_r = 
        ((1U & (~ (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__rst_i))) 
         && ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__enable_i) 
             && (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__invert_i)));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__out1_o = vlSelfRef.pdh_core__DOT__u_nco__DOT__rom1_signed_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__out2_o = vlSelfRef.pdh_core__DOT__u_nco__DOT__rom2_signed_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__addr1 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_o = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__u_qtr_rom__DOT__addr2 
        = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_o = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_o = vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_pipe1_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_o = vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_pipe1_r;
    vlSelfRef.pdh_core__DOT__addr1_w = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr1_o;
    vlSelfRef.pdh_core__DOT__addr2_w = vlSelfRef.pdh_core__DOT__u_nco__DOT__addr2_o;
    vlSelfRef.pdh_core__DOT__phi2_w = vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_o;
    vlSelfRef.pdh_core__DOT__phi1_w = vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_o;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__next_phi1_w 
        = (0x3fffU & ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_r) 
                      + (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__stride_r)));
    vlSelfRef.pdh_core__DOT__u_nco__DOT__phi2_w = (0x3fffU 
                                                   & ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__sub_r)
                                                       ? 
                                                      ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_r) 
                                                       - (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__shift_r))
                                                       : 
                                                      ((IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__phi1_r) 
                                                       + (IData)(vlSelfRef.pdh_core__DOT__u_nco__DOT__shift_r))));
}

VL_INLINE_OPT void Vtop___024root___nba_sequent__TOP__6(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_sequent__TOP__6\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.pdh_core__DOT__dac_sel_r = ((1U & (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_ne_r))) 
                                          && (IData)(vlSelfRef.pdh_core__DOT__next_dac_sel_w));
    if (vlSelfRef.pdh_core__DOT__rst_sync_ne_r) {
        vlSelfRef.pdh_core__DOT__dac1_dat_r = 0x2000U;
        vlSelfRef.pdh_core__DOT__dac2_dat_r = 0x2000U;
    } else {
        vlSelfRef.pdh_core__DOT__dac1_dat_r = vlSelfRef.pdh_core__DOT__next_dac1_dat_w;
        vlSelfRef.pdh_core__DOT__dac2_dat_r = vlSelfRef.pdh_core__DOT__next_dac2_dat_w;
    }
    vlSelfRef.pdh_core__DOT__next_dac_sel_w = (1U & 
                                               (~ (IData)(vlSelfRef.pdh_core__DOT__dac_sel_r)));
    vlSelfRef.pdh_core__DOT__dac_sel_o = vlSelfRef.pdh_core__DOT__dac_sel_r;
    vlSelfRef.pdh_core__DOT__dac_cb_w = (0x20000000U 
                                         | (((IData)(vlSelfRef.pdh_core__DOT__dac1_dat_r) 
                                             << 0xeU) 
                                            | (IData)(vlSelfRef.pdh_core__DOT__dac2_dat_r)));
    vlSelfRef.dac_sel_o = vlSelfRef.pdh_core__DOT__dac_sel_o;
}

VL_INLINE_OPT void Vtop___024root___nba_sequent__TOP__7(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_sequent__TOP__7\n"); );
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
    // Body
    vlSelfRef.pdh_core__DOT__bram_enable_r = ((1U & 
                                               (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_r))) 
                                              && (IData)(vlSelfRef.pdh_core__DOT__next_bram_enable_w));
    vlSelfRef.pdh_core__DOT__dma_enable_r = ((1U & 
                                              (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_r))) 
                                             && (IData)(vlSelfRef.pdh_core__DOT__next_dma_enable_w));
    if (vlSelfRef.pdh_core__DOT__rst_sync_r) {
        vlSelfRef.pdh_core__DOT__callback_r = 0U;
        vlSelfRef.pdh_core__DOT__dma_data_r = 0ULL;
        vlSelfRef.pdh_core__DOT__dma_state_r = 0U;
        vlSelfRef.pdh_core__DOT__led_r = 0U;
        vlSelfRef.pdh_core__DOT__dma_decimation_code_r = 1U;
        vlSelfRef.pdh_core__DOT__nco_shift_r = 0U;
        vlSelfRef.pdh_core__DOT__nco_stride_r = 1U;
        vlSelfRef.pdh_core__DOT__nco_out1_r = 0U;
        vlSelfRef.pdh_core__DOT__nco_out1_r = 0U;
        vlSelfRef.pdh_core__DOT__frame_code_r = 0U;
        vlSelfRef.pdh_core__DOT__rot_cos_theta_r = 0x7fffU;
        vlSelfRef.pdh_core__DOT__rot_sin_theta_r = 0U;
        vlSelfRef.pdh_core__DOT__sin_theta_r = 0U;
        vlSelfRef.pdh_core__DOT__cos_theta_r = 0x7fffU;
        vlSelfRef.pdh_core__DOT__sp_r = 0U;
        vlSelfRef.pdh_core__DOT__dec_r = 1U;
        vlSelfRef.pdh_core__DOT__ki_r = 0U;
        vlSelfRef.pdh_core__DOT__kd_r = 0U;
        vlSelfRef.pdh_core__DOT__kp_r = 0U;
        vlSelfRef.pdh_core__DOT__alpha_r = 4U;
        vlSelfRef.pdh_core__DOT__satwidth_r = 0x1fU;
        vlSelfRef.pdh_core__DOT__dac1_dat_sel_r = 0U;
        vlSelfRef.pdh_core__DOT__dac2_dat_sel_r = 0U;
        vlSelfRef.pdh_core__DOT__nco_feed1_r = 0U;
        vlSelfRef.pdh_core__DOT__nco_feed2_r = 0U;
        vlSelfRef.pdh_core__DOT__pid_sel_r = 0U;
        vlSelfRef.pdh_core__DOT__adc_dat_a_16s_r = 0U;
        vlSelfRef.pdh_core__DOT__adc_dat_b_16s_r = 0U;
        vlSelfRef.pdh_core__DOT__q_feed_r = 0U;
        vlSelfRef.pdh_core__DOT__i_feed_r = 0U;
        vlSelfRef.pdh_core__DOT__axi_3ff_r = 0U;
        vlSelfRef.pdh_core__DOT__axi_from_ps_r = 0U;
        vlSelfRef.pdh_core__DOT__axi_2ff_r = 0U;
        vlSelfRef.pdh_core__DOT__axi_1ff_r = 0U;
    } else {
        vlSelfRef.pdh_core__DOT__callback_r = vlSelfRef.pdh_core__DOT__next_callback_w;
        vlSelfRef.pdh_core__DOT__dma_data_r = vlSelfRef.pdh_core__DOT__dma_data_w;
        vlSelfRef.pdh_core__DOT__dma_state_r = vlSelfRef.pdh_core__DOT__next_dma_state_w;
        vlSelfRef.pdh_core__DOT__led_r = vlSelfRef.pdh_core__DOT__next_led_w;
        vlSelfRef.pdh_core__DOT__dma_decimation_code_r 
            = vlSelfRef.pdh_core__DOT__next_dma_decimation_code_w;
        vlSelfRef.pdh_core__DOT__nco_shift_r = vlSelfRef.pdh_core__DOT__nco_shift_w;
        vlSelfRef.pdh_core__DOT__nco_stride_r = vlSelfRef.pdh_core__DOT__nco_stride_w;
        vlSelfRef.pdh_core__DOT__nco_out1_r = vlSelfRef.pdh_core__DOT__nco_out1_w;
        vlSelfRef.pdh_core__DOT__frame_code_r = vlSelfRef.pdh_core__DOT__next_frame_code_w;
        vlSelfRef.pdh_core__DOT__rot_cos_theta_r = vlSelfRef.pdh_core__DOT__next_rot_cos_theta_w;
        vlSelfRef.pdh_core__DOT__rot_sin_theta_r = vlSelfRef.pdh_core__DOT__next_rot_sin_theta_w;
        vlSelfRef.pdh_core__DOT__sin_theta_r = vlSelfRef.pdh_core__DOT__next_sin_theta_w;
        vlSelfRef.pdh_core__DOT__cos_theta_r = vlSelfRef.pdh_core__DOT__next_cos_theta_w;
        vlSelfRef.pdh_core__DOT__sp_r = vlSelfRef.pdh_core__DOT__sp_w;
        vlSelfRef.pdh_core__DOT__dec_r = vlSelfRef.pdh_core__DOT__dec_w;
        vlSelfRef.pdh_core__DOT__ki_r = vlSelfRef.pdh_core__DOT__ki_w;
        vlSelfRef.pdh_core__DOT__kd_r = vlSelfRef.pdh_core__DOT__kd_w;
        vlSelfRef.pdh_core__DOT__kp_r = vlSelfRef.pdh_core__DOT__kp_w;
        vlSelfRef.pdh_core__DOT__alpha_r = vlSelfRef.pdh_core__DOT__alpha_w;
        vlSelfRef.pdh_core__DOT__satwidth_r = vlSelfRef.pdh_core__DOT__satwidth_w;
        vlSelfRef.pdh_core__DOT__dac1_dat_sel_r = vlSelfRef.pdh_core__DOT__next_dac1_dat_sel_w;
        vlSelfRef.pdh_core__DOT__dac2_dat_sel_r = vlSelfRef.pdh_core__DOT__next_dac2_dat_sel_w;
        vlSelfRef.pdh_core__DOT__nco_feed1_r = vlSelfRef.pdh_core__DOT__nco_feed1_w;
        vlSelfRef.pdh_core__DOT__nco_feed2_r = vlSelfRef.pdh_core__DOT__nco_feed2_w;
        vlSelfRef.pdh_core__DOT__pid_sel_r = vlSelfRef.pdh_core__DOT__next_pid_sel_w;
        vlSelfRef.pdh_core__DOT__adc_dat_a_16s_r = vlSelfRef.pdh_core__DOT__adc_dat_a_16s_w;
        vlSelfRef.pdh_core__DOT__adc_dat_b_16s_r = vlSelfRef.pdh_core__DOT__adc_dat_b_16s_w;
        vlSelfRef.pdh_core__DOT__q_feed_r = vlSelfRef.pdh_core__DOT__q_feed_w;
        vlSelfRef.pdh_core__DOT__i_feed_r = vlSelfRef.pdh_core__DOT__i_feed_w;
        vlSelfRef.pdh_core__DOT__axi_3ff_r = vlSelfRef.pdh_core__DOT__axi_2ff_r;
        vlSelfRef.pdh_core__DOT__axi_from_ps_r = vlSelfRef.pdh_core__DOT__next_axi_from_ps_w;
        vlSelfRef.pdh_core__DOT__axi_2ff_r = vlSelfRef.pdh_core__DOT__axi_1ff_r;
        vlSelfRef.pdh_core__DOT__axi_1ff_r = vlSelfRef.pdh_core__DOT__axi_from_ps_i;
    }
    vlSelfRef.pdh_core__DOT__bram_edge_acquired_r = 
        ((1U & (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_r))) 
         && (IData)(vlSelfRef.pdh_core__DOT__next_bram_edge_acquired_w));
    vlSelfRef.pdh_core__DOT__bram_ready_r = ((1U & 
                                              (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_r))) 
                                             && (IData)(vlSelfRef.pdh_core__DOT__bram_ready_i));
    vlSelfRef.pdh_core__DOT__nco_en_r = ((1U & (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_r))) 
                                         && (IData)(vlSelfRef.pdh_core__DOT__nco_en_w));
    vlSelfRef.pdh_core__DOT__nco_inv_r = ((1U & (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_r))) 
                                          && (IData)(vlSelfRef.pdh_core__DOT__nco_inv_w));
    vlSelfRef.pdh_core__DOT__nco_sub_r = ((1U & (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_r))) 
                                          && (IData)(vlSelfRef.pdh_core__DOT__nco_sub_w));
    if ((1U & (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_r)))) {
        vlSelfRef.pdh_core__DOT__nco_out2_r = vlSelfRef.pdh_core__DOT__nco_out2_w;
    }
    vlSelfRef.pdh_core__DOT__pid_enable_r = ((1U & 
                                              (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_r))) 
                                             && (IData)(vlSelfRef.pdh_core__DOT__pid_enable_w));
    vlSelfRef.pdh_core__DOT__dma_ready_3ff = ((1U & 
                                               (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_r))) 
                                              && (IData)(vlSelfRef.pdh_core__DOT__dma_ready_2ff));
    vlSelfRef.pdh_core__DOT__bram_enable_o = vlSelfRef.pdh_core__DOT__bram_enable_r;
    vlSelfRef.pdh_core__DOT__dma_enable_o = vlSelfRef.pdh_core__DOT__dma_enable_r;
    vlSelfRef.pdh_core__DOT__axi_to_ps_o = vlSelfRef.pdh_core__DOT__callback_r;
    vlSelfRef.pdh_core__DOT__dma_data_o = vlSelfRef.pdh_core__DOT__dma_data_r;
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
    vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__D 
        = vlSelfRef.pdh_core__DOT__bram_ready_r;
    vlSelfRef.pdh_core__DOT__led_o = vlSelfRef.pdh_core__DOT__led_r;
    vlSelfRef.pdh_core__DOT__led_cb_w = (0x10000000U 
                                         | (IData)(vlSelfRef.pdh_core__DOT__led_r));
    vlSelfRef.pdh_core__DOT__dma_decimation_code_o 
        = vlSelfRef.pdh_core__DOT__dma_decimation_code_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__shift_i = vlSelfRef.pdh_core__DOT__nco_shift_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__stride_i = vlSelfRef.pdh_core__DOT__nco_stride_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__enable_i = vlSelfRef.pdh_core__DOT__nco_en_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__invert_i = vlSelfRef.pdh_core__DOT__nco_inv_r;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__sub_i = vlSelfRef.pdh_core__DOT__nco_sub_r;
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
    vlSelfRef.pdh_core__DOT__set_rot_cb_w = (0x50000000U 
                                             | ((0xfffc000U 
                                                 & ((IData)(vlSelfRef.pdh_core__DOT__sin_theta_r) 
                                                    << 0xcU)) 
                                                | (0x3fffU 
                                                   & ((IData)(vlSelfRef.pdh_core__DOT__cos_theta_r) 
                                                      >> 2U))));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__sp_i = vlSelfRef.pdh_core__DOT__sp_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__decimate_i 
        = vlSelfRef.pdh_core__DOT__dec_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__ki_i = vlSelfRef.pdh_core__DOT__ki_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__kd_i = vlSelfRef.pdh_core__DOT__kd_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__kp_i = vlSelfRef.pdh_core__DOT__kp_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__alpha_i = vlSelfRef.pdh_core__DOT__alpha_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__satwidth_i 
        = vlSelfRef.pdh_core__DOT__satwidth_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__enable_i = vlSelfRef.pdh_core__DOT__pid_enable_r;
    vlSelfRef.pdh_core__DOT__config_io_cb_w = (0xe0000000U 
                                               | (((IData)(vlSelfRef.pdh_core__DOT__pid_sel_r) 
                                                   << 6U) 
                                                  | (((IData)(vlSelfRef.pdh_core__DOT__dac2_dat_sel_r) 
                                                      << 3U) 
                                                     | (IData)(vlSelfRef.pdh_core__DOT__dac1_dat_sel_r))));
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
    vlSelfRef.pdh_core__DOT__commit_rot_cb_w = (0x60000000U 
                                                | ((0xfffc000U 
                                                    & ((IData)(vlSelfRef.pdh_core__DOT__q_feed_r) 
                                                       << 0xcU)) 
                                                   | (0x3fffU 
                                                      & ((IData)(vlSelfRef.pdh_core__DOT__i_feed_r) 
                                                         >> 2U))));
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
    vlSelfRef.pdh_core__DOT__strobe_bit_w = (1U & (vlSelfRef.pdh_core__DOT__axi_3ff_r 
                                                   >> 0x1eU));
    vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__D 
        = vlSelfRef.pdh_core__DOT__dma_ready_3ff;
    vlSelfRef.pdh_core__DOT__get_frame_cb_w = (0x70000000U 
                                               | (((IData)(vlSelfRef.pdh_core__DOT__frame_code_r) 
                                                   << 0x17U) 
                                                  | ((vlSelfRef.pdh_core__DOT__dma_decimation_code_r 
                                                      << 1U) 
                                                     | (IData)(vlSelfRef.pdh_core__DOT__dma_ready_3ff))));
    vlSelfRef.pdh_core__DOT__dma_ready_2ff = ((1U & 
                                               (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_r))) 
                                              && (IData)(vlSelfRef.pdh_core__DOT__dma_ready_1ff));
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
    vlSelfRef.pdh_core__DOT__cmd_w = (0xfU & (vlSelfRef.pdh_core__DOT__axi_from_ps_r 
                                              >> 0x1aU));
    vlSelfRef.pdh_core__DOT__data_w = (0x3ffffffU & vlSelfRef.pdh_core__DOT__axi_from_ps_r);
    vlSelfRef.bram_enable_o = vlSelfRef.pdh_core__DOT__bram_enable_o;
    vlSelfRef.dma_enable_o = vlSelfRef.pdh_core__DOT__dma_enable_o;
    vlSelfRef.axi_to_ps_o = vlSelfRef.pdh_core__DOT__axi_to_ps_o;
    vlSelfRef.dma_data_o = vlSelfRef.pdh_core__DOT__dma_data_o;
    vlSelfRef.led_o = vlSelfRef.pdh_core__DOT__led_o;
    vlSelfRef.dma_decimation_code_o = vlSelfRef.pdh_core__DOT__dma_decimation_code_o;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__next_satwidth_w 
        = ((0xfU <= (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__satwidth_i))
            ? (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__satwidth_i)
            : 0x1fU);
    vlSelfRef.pdh_core__DOT__u_pid__DOT__dat_i = vlSelfRef.pdh_core__DOT__pid_in_w;
    vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__D 
        = vlSelfRef.pdh_core__DOT__strobe_bit_w;
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
    vlSelfRef.pdh_core__DOT__dma_ready_1ff = ((1U & 
                                               (~ (IData)(vlSelfRef.pdh_core__DOT__rst_sync_r))) 
                                              && (IData)(vlSelfRef.pdh_core__DOT__dma_ready_i));
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
    vlSelfRef.pdh_core__DOT__set_pid_cb_w = (0x80000000U 
                                             | (((IData)(vlSelfRef.pdh_core__DOT__pid_coeff_select_w) 
                                                 << 0x10U) 
                                                | (IData)(vlSelfRef.pdh_core__DOT__pid_payload_w)));
}

VL_INLINE_OPT void Vtop___024root___nba_comb__TOP__0(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_comb__TOP__0\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
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
}

VL_INLINE_OPT void Vtop___024root___nba_sequent__TOP__8(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_sequent__TOP__8\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.pdh_core__DOT__rst_sync_ne_r = ((IData)(vlSelfRef.pdh_core__DOT__rst_i) 
                                              || (IData)(vlSelfRef.pdh_core__DOT__rst_pipe1_ne_r));
    vlSelfRef.pdh_core__DOT__rst_pipe1_ne_r = vlSelfRef.pdh_core__DOT__rst_i;
}

VL_INLINE_OPT void Vtop___024root___nba_sequent__TOP__9(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_sequent__TOP__9\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.pdh_core__DOT__nco_out1_w = vlSelfRef.pdh_core__DOT__u_nco__DOT__out1_o;
    vlSelfRef.pdh_core__DOT__nco_out2_w = vlSelfRef.pdh_core__DOT__u_nco__DOT__out2_o;
}

VL_INLINE_OPT void Vtop___024root___nba_comb__TOP__1(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_comb__TOP__1\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
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
    vlSelfRef.pdh_core__DOT__dac_dat_o = ((IData)(vlSelfRef.pdh_core__DOT__dac_sel_r)
                                           ? (IData)(vlSelfRef.pdh_core__DOT__dac2_feed_w)
                                           : (IData)(vlSelfRef.pdh_core__DOT__dac1_feed_w));
    vlSelfRef.dac_dat_o = vlSelfRef.pdh_core__DOT__dac_dat_o;
}

VL_INLINE_OPT void Vtop___024root___nba_comb__TOP__2(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_comb__TOP__2\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__Q 
        = ((~ (IData)(vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__prev_D)) 
           & (IData)(vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__D));
    vlSelfRef.pdh_core__DOT__bram_ready_edge_w = vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__Q;
    vlSelfRef.pdh_core__DOT__next_bram_edge_acquired_w 
        = ((1U & (~ ((IData)(vlSelfRef.pdh_core__DOT__dma_state_r) 
                     >> 1U))) && ((1U & (IData)(vlSelfRef.pdh_core__DOT__dma_state_r)) 
                                  && ((IData)(vlSelfRef.pdh_core__DOT__bram_ready_edge_w) 
                                      | (IData)(vlSelfRef.pdh_core__DOT__bram_edge_acquired_r))));
}

VL_INLINE_OPT void Vtop___024root___nba_comb__TOP__3(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_comb__TOP__3\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Init
    IData/*31:0*/ __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__Vfuncout;
    __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__Vfuncout = 0;
    QData/*32:0*/ __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__in;
    __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__in = 0;
    IData/*31:0*/ __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__threshold;
    __Vfunc_pdh_core__DOT__u_pid__DOT__apply_satwidth_truncation__2__threshold = 0;
    // Body
    vlSelfRef.pdh_core__DOT__u_pid__DOT__tick1_w = 
        ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__enable_i) 
         & (0U == (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__cnt_r)));
    vlSelfRef.pdh_core__DOT__u_pid__DOT__error_w = 
        (0xffffU & ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__dat_i) 
                    - VL_EXTENDS_II(16,14, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__sp_r))));
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
    vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_w 
        = ((IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__tick2_r)
            ? VL_MULS_III(32, VL_EXTENDS_II(32,16, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__kd_r)), 
                          (VL_EXTENDS_II(32,16, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__error_w)) 
                           - VL_EXTENDS_II(32,16, (IData)(vlSelfRef.pdh_core__DOT__u_pid__DOT__yk_r))))
            : vlSelfRef.pdh_core__DOT__u_pid__DOT__d_error_r);
    vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_err_tap 
        = vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_error2_w;
    vlSelfRef.pdh_core__DOT__sum_err_tap_w = vlSelfRef.pdh_core__DOT__u_pid__DOT__sum_err_tap;
}

VL_INLINE_OPT void Vtop___024root___nba_comb__TOP__4(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_comb__TOP__4\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__Q 
        = ((~ (IData)(vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__prev_D)) 
           & (IData)(vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__D));
    vlSelfRef.pdh_core__DOT__dma_ready_edge_w = vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__Q;
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
}

VL_INLINE_OPT void Vtop___024root___nba_comb__TOP__5(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_comb__TOP__5\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
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
}

VL_INLINE_OPT void Vtop___024root___nba_comb__TOP__6(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_comb__TOP__6\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__Q 
        = ((~ (IData)(vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__prev_D)) 
           & (IData)(vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__D));
    vlSelfRef.pdh_core__DOT__strobe_edge_w = vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__Q;
    vlSelfRef.pdh_core__DOT__next_axi_from_ps_w = ((IData)(vlSelfRef.pdh_core__DOT__strobe_edge_w)
                                                    ? vlSelfRef.pdh_core__DOT__axi_3ff_r
                                                    : vlSelfRef.pdh_core__DOT__axi_from_ps_r);
}

VL_INLINE_OPT void Vtop___024root___nba_sequent__TOP__10(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_sequent__TOP__10\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.pdh_core__DOT__rst_sync_r = ((IData)(vlSelfRef.pdh_core__DOT__rst_i) 
                                           || (IData)(vlSelfRef.pdh_core__DOT__rst_pipe1_r));
    vlSelfRef.pdh_core__DOT__rst_pipe1_r = vlSelfRef.pdh_core__DOT__rst_i;
    vlSelfRef.pdh_core__DOT__u_nco__DOT__rst_i = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__u_pid__DOT__rst = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__u_strobe_edge_detector__DOT__rst 
        = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__u_dma_edge_detector__DOT__rst 
        = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__u_bram_edge_detector__DOT__rst 
        = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.pdh_core__DOT__dac_rst_o = vlSelfRef.pdh_core__DOT__rst_sync_r;
    vlSelfRef.dac_rst_o = vlSelfRef.pdh_core__DOT__dac_rst_o;
}

VL_INLINE_OPT void Vtop___024root___nba_comb__TOP__7(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___nba_comb__TOP__7\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
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
}

void Vtop___024root___eval_triggers__act(Vtop___024root* vlSelf);

bool Vtop___024root___eval_phase__act(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_phase__act\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Init
    VlTriggerVec<12> __VpreTriggered;
    CData/*0:0*/ __VactExecute;
    // Body
    Vtop___024root___eval_triggers__act(vlSelf);
    __VactExecute = vlSelfRef.__VactTriggered.any();
    if (__VactExecute) {
        __VpreTriggered.andNot(vlSelfRef.__VactTriggered, vlSelfRef.__VnbaTriggered);
        vlSelfRef.__VnbaTriggered.thisOr(vlSelfRef.__VactTriggered);
        Vtop___024root___eval_act(vlSelf);
    }
    return (__VactExecute);
}

bool Vtop___024root___eval_phase__nba(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_phase__nba\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Init
    CData/*0:0*/ __VnbaExecute;
    // Body
    __VnbaExecute = vlSelfRef.__VnbaTriggered.any();
    if (__VnbaExecute) {
        Vtop___024root___eval_nba(vlSelf);
        vlSelfRef.__VnbaTriggered.clear();
    }
    return (__VnbaExecute);
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__ico(Vtop___024root* vlSelf);
#endif  // VL_DEBUG
#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__nba(Vtop___024root* vlSelf);
#endif  // VL_DEBUG
#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__act(Vtop___024root* vlSelf);
#endif  // VL_DEBUG

void Vtop___024root___eval(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Init
    IData/*31:0*/ __VicoIterCount;
    CData/*0:0*/ __VicoContinue;
    IData/*31:0*/ __VnbaIterCount;
    CData/*0:0*/ __VnbaContinue;
    // Body
    __VicoIterCount = 0U;
    vlSelfRef.__VicoFirstIteration = 1U;
    __VicoContinue = 1U;
    while (__VicoContinue) {
        if (VL_UNLIKELY(((0x64U < __VicoIterCount)))) {
#ifdef VL_DEBUG
            Vtop___024root___dump_triggers__ico(vlSelf);
#endif
            VL_FATAL_MT("/home/nolan/Projects/Pound-Drever-Hall/core/hw/prj/pdh_core/rtl/pdh_core.sv", 5, "", "Input combinational region did not converge.");
        }
        __VicoIterCount = ((IData)(1U) + __VicoIterCount);
        __VicoContinue = 0U;
        if (Vtop___024root___eval_phase__ico(vlSelf)) {
            __VicoContinue = 1U;
        }
        vlSelfRef.__VicoFirstIteration = 0U;
    }
    __VnbaIterCount = 0U;
    __VnbaContinue = 1U;
    while (__VnbaContinue) {
        if (VL_UNLIKELY(((0x64U < __VnbaIterCount)))) {
#ifdef VL_DEBUG
            Vtop___024root___dump_triggers__nba(vlSelf);
#endif
            VL_FATAL_MT("/home/nolan/Projects/Pound-Drever-Hall/core/hw/prj/pdh_core/rtl/pdh_core.sv", 5, "", "NBA region did not converge.");
        }
        __VnbaIterCount = ((IData)(1U) + __VnbaIterCount);
        __VnbaContinue = 0U;
        vlSelfRef.__VactIterCount = 0U;
        vlSelfRef.__VactContinue = 1U;
        while (vlSelfRef.__VactContinue) {
            if (VL_UNLIKELY(((0x64U < vlSelfRef.__VactIterCount)))) {
#ifdef VL_DEBUG
                Vtop___024root___dump_triggers__act(vlSelf);
#endif
                VL_FATAL_MT("/home/nolan/Projects/Pound-Drever-Hall/core/hw/prj/pdh_core/rtl/pdh_core.sv", 5, "", "Active region did not converge.");
            }
            vlSelfRef.__VactIterCount = ((IData)(1U) 
                                         + vlSelfRef.__VactIterCount);
            vlSelfRef.__VactContinue = 0U;
            if (Vtop___024root___eval_phase__act(vlSelf)) {
                vlSelfRef.__VactContinue = 1U;
            }
        }
        if (Vtop___024root___eval_phase__nba(vlSelf)) {
            __VnbaContinue = 1U;
        }
    }
}

#ifdef VL_DEBUG
void Vtop___024root___eval_debug_assertions(Vtop___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_debug_assertions\n"); );
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if (VL_UNLIKELY(((vlSelfRef.clk & 0xfeU)))) {
        Verilated::overWidthError("clk");}
    if (VL_UNLIKELY(((vlSelfRef.adc_dat_a_i & 0xc000U)))) {
        Verilated::overWidthError("adc_dat_a_i");}
    if (VL_UNLIKELY(((vlSelfRef.adc_dat_b_i & 0xc000U)))) {
        Verilated::overWidthError("adc_dat_b_i");}
    if (VL_UNLIKELY(((vlSelfRef.dma_ready_i & 0xfeU)))) {
        Verilated::overWidthError("dma_ready_i");}
    if (VL_UNLIKELY(((vlSelfRef.bram_ready_i & 0xfeU)))) {
        Verilated::overWidthError("bram_ready_i");}
}
#endif  // VL_DEBUG
