#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "server.h"
#include "hw_common.h"
#include "control.h"

#define RESET_ON 1
#define RESET_OFF 0
#define LOCK_POINT_FLAG "lock_point"
#define DERIVED_SLOPE_FLAG "derived_slope"
#define RETURN_CODE "RETURN_CODE"
#define CMD "CMD_CB"

static inline pdh_callback_t pdh_execute_cmd(pdh_cmd_t cmd)
{
    cmd.strobe.val = 0;
    pdh_send_cmd(cmd);
    cmd.strobe.val = 1;
    pdh_send_cmd(cmd);

    pdh_callback_t cb = { 0 };
    pdh_get_callback(&cb);
    return cb;
}

/* Send strobe without reading callback — used by interrupt-driven _send handlers */
void pdh_strobe_cmd(pdh_cmd_t cmd)
{
    cmd.strobe.val = 0;
    pdh_send_cmd(cmd);
    cmd.strobe.val = 1;
    pdh_send_cmd(cmd);
}

static inline int push_ctx_cb(cmd_ctx_t* ctx, size_t* index, void* value, int tag, const char* name) //Value should always be an echo
{
    switch(tag)
    {
        case FLOAT_TAG:
            ctx->output.output_items[*index].data.f = *(float*)value;
            ctx->output.output_items[*index].tag = FLOAT_TAG;
            strcpy(ctx->output.output_items[*index].name, name);
            break;

        case INT_TAG:
            ctx->output.output_items[*index].data.i = *(int32_t*)value;
            ctx->output.output_items[*index].tag = INT_TAG;
            strcpy(ctx->output.output_items[*index].name, name);
            break;

        case INT16_TAG:
            ctx->output.output_items[*index].data.i = *(int16_t*)value;
            ctx->output.output_items[*index].tag = INT_TAG;
            strcpy(ctx->output.output_items[*index].name, name);
            break;

        case UINT_TAG:
            ctx->output.output_items[*index].data.u = *(uint32_t*)value;
            ctx->output.output_items[*index].tag = UINT_TAG;
            strcpy(ctx->output.output_items[*index].name, name);
            break;

        default:
            DEBUG_INFO("INVALID TAG CODE: %d", tag); 
            return 1;
    }

    ctx->output.num_outputs = ++*index;
    return 0;
}


static inline int validate_cb(void* cb, void* expected, int tag, const char* func, const char* name, int success_code, int fail_code)
{
    int return_code = success_code;

    switch(tag)
    {
        case FLOAT_TAG:
            if(*(float*)cb != *(float*)expected) 
            {
                DEBUG_INFO("%s::ERROR: Expected %f for %s but got %f", func, *(float*)expected, name, *(float*)cb);
                return_code = fail_code;
            }
            break;

        case INT_TAG:
            if(*(int32_t*)cb != *(int32_t*)expected) 
            {
                DEBUG_INFO("%s::ERROR: Expected %d for %s but got %d", func,*(int32_t*)expected, name, *(int32_t*)cb);
                return_code = fail_code;
            }
            break;

        case INT16_TAG:
            if(*(int16_t*)cb != *(int16_t*)expected) 
            {
                DEBUG_INFO("%s::ERROR: Expected %d for %s but got %d", func,*(int16_t*)expected, name, *(int16_t*)cb);
                return_code = fail_code;
            }
            break;

        case UINT_TAG:
            if(*(uint32_t*)cb != *(uint32_t*)expected) 
            {
                DEBUG_INFO("%s::ERROR: Expected %u for %s but got %u", func, *(uint32_t*)expected, name, *(uint32_t*)cb);
                return_code = fail_code;
            }
            break;

        default:
            DEBUG_INFO("INVALID TAG CODE: %d", tag); 
            return -1;
    }

    return return_code;
}


int cmd_reset_fpga(cmd_ctx_t* ctx) //TODO: Better rst cb verification
{
    int return_code = PDH_OK;

    pdh_cmd_t cmd;
    cmd.raw = 0;

    cmd.rst_cmd.rst = RESET_ON;
    pdh_send_cmd(cmd); 
    cmd.rst_cmd.rst = RESET_OFF;
    pdh_send_cmd(cmd);

    pdh_callback_t callback;
    callback.raw = 0;
    pdh_get_callback(&callback);


    uint32_t rst_raw  = callback.raw;

    size_t index = 0;
    push_ctx_cb(ctx, &index, &rst_raw, UINT_TAG, "RST_RAW_CB");

    return return_code;
}


int cmd_set_led(cmd_ctx_t* ctx)
{
    
    pdh_cmd_t cmd;
    cmd.raw = 0;

    int return_code = SET_LED_OK;
    uint32_t led_code = ctx->uint_args[0];
    
    size_t index = 0;

    if (led_code > 255)
    {
        DEBUG_INFO("ERROR EXECUTING: %s! --- Invalid LED_CODE: %d\n", __func__, led_code);
        return_code = SET_LED_INVALID_LED;
    }

    else
    {
        cmd.led_cmd.cmd = CMD_SET_LED;
        cmd.led_cmd.led_code = led_code;
        pdh_callback_t callback = pdh_execute_cmd(cmd);


        uint32_t echo_code = callback.led_cb.func_callback;
        uint32_t echo_cmd = callback.led_cb.cmd;
        push_ctx_cb(ctx, &index, &echo_code, UINT_TAG, "LED_CODE_CB");
        push_ctx_cb(ctx, &index, &echo_cmd, UINT_TAG, CMD);
        
        return_code = validate_cb(&echo_code, &led_code, UINT_TAG, __func__, "LED_CODE", return_code, SET_LED_INVALID_LED_CB);
        uint32_t cmdval = cmd.cmd.val;
        return_code = validate_cb(&echo_cmd, &cmdval, UINT_TAG, __func__, CMD, return_code, PDH_INVALID_CMD);
    }
    
    return return_code;
}


int cmd_get_adc(cmd_ctx_t *ctx)
{
    int return_code = GET_ADC_OK;

    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.adc_cmd.cmd = CMD_GET_ADC;
    pdh_callback_t callback = pdh_execute_cmd(cmd); 
    
    uint32_t echo_adc0_code = callback.adc_cb.adc_0_code; //TODO: Range checking
    uint32_t echo_adc1_code = callback.adc_cb.adc_1_code;
    float adc0_converted = -1.0f * (echo_adc0_code * (2.0f/16383.0f) - 1.0f);
    float adc1_converted = -1.0f * (echo_adc1_code * (2.0f/16383.0f) - 1.0f);
    uint32_t echo_cmd = callback.adc_cb.cmd;

    size_t index = 0;
    push_ctx_cb(ctx, &index, &echo_adc0_code, UINT_TAG, "IN1");
    push_ctx_cb(ctx, &index, &echo_adc1_code, UINT_TAG, "IN2");
    push_ctx_cb(ctx, &index, &adc0_converted, FLOAT_TAG, "IN1_V");
    push_ctx_cb(ctx, &index, &adc1_converted, FLOAT_TAG, "IN2_V");
    push_ctx_cb(ctx, &index, &echo_cmd, UINT_TAG, CMD);
    
    uint32_t cmdval = cmd.cmd.val;
    return_code = validate_cb(&echo_cmd, &cmdval, UINT_TAG, __func__, CMD, return_code, PDH_INVALID_CMD);
    return return_code;
}


int cmd_check_signed(cmd_ctx_t* ctx)
{
    int return_code = CHECK_SIGNED_OK;
    uint32_t reg_sel = ctx->uint_args[0];

    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.cs_cmd.cmd = CMD_CHECK_SIGNED;
    cmd.cs_cmd.reg_sel = reg_sel;
    pdh_callback_t callback = pdh_execute_cmd(cmd); 

    uint32_t echo_reg_sel = callback.cs_cb.reg_sel;
    int16_t echo_payload_16 = (int16_t)callback.cs_cb.payload;
    int32_t echo_payload_32 = (int32_t)echo_payload_16; //sign extend it
    uint32_t echo_cmd = callback.cs_cb.cmd;

    size_t index = 0;
    push_ctx_cb(ctx, &index, &echo_reg_sel, UINT_TAG, "REG_SEL_CB");
    push_ctx_cb(ctx, &index, &echo_payload_32, INT_TAG, "REG_VALUE_CB");
    push_ctx_cb(ctx, &index, &echo_cmd, UINT_TAG, CMD);

    return_code = validate_cb(&echo_reg_sel, &reg_sel, UINT_TAG, __func__, "REG_SEL", return_code, CHECK_SIGNED_INVALID_REG_SEL_CB);
    
    uint32_t cmdval = cmd.cmd.val;
    return_code = validate_cb(&echo_cmd, &cmdval, UINT_TAG, __func__, "CMD", return_code, PDH_INVALID_CMD);
    return return_code;
}


int cmd_set_dac(cmd_ctx_t* ctx)
{
    int return_code = SET_DAC_OK;

    float val = ctx->float_args[0];
    bool dac_sel = (bool)ctx->uint_args[0];

    val *= -1.0f;

    if (val > 1.0) val = 1.0;
    if (val < -1.0) val = -1.0;
    
    //[-1, 1] -> [0, 1] at 2x resolution
    float y = (val + 1.0f) * 0.5f;

    //[0, 1] -> [0, 16383]; 16383 is max DAC output of 1.0V, 0 is min DAC output of -1.0V. 0 at ~16383//2
    int code = (int)lrintf(y * 16383.0f);

    if (code < 0) code = 0;
    if (code > 16383) code = 16383;

    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.dac_cmd.dac_code = code;
    cmd.dac_cmd.dac_sel = dac_sel;
    cmd.dac_cmd.cmd = CMD_SET_DAC;
    pdh_callback_t callback = pdh_execute_cmd(cmd);
    
    uint32_t echo_dac1_code = callback.dac_cb.dac1_code;
    uint32_t echo_dac2_code = callback.dac_cb.dac2_code;
    uint32_t echo_cmd = callback.dac_cb.cmd;

    size_t index = 0;
    push_ctx_cb(ctx, &index, &echo_dac1_code, UINT_TAG, "DAC1_CODE_CB"); 
    push_ctx_cb(ctx, &index, &echo_dac2_code, UINT_TAG, "DAC2_CODE_CB"); 

    if(dac_sel == 0)
    {
        return_code = validate_cb(&echo_dac1_code, &code, UINT_TAG, __func__, "DAC1_CODE_CB", return_code, SET_DAC_INVALID_CODE);
    }
    
    else
    {
        return_code = validate_cb(&echo_dac2_code, &code, UINT_TAG, __func__, "DAC2_CODE_CB", return_code, SET_DAC_INVALID_CODE);
    }

    uint32_t cmdval = cmd.cmd.val;
    return_code = validate_cb(&echo_cmd, &cmdval, UINT_TAG, __func__, "CMD", return_code, PDH_INVALID_CMD);
    return return_code;
}


static inline int16_t float_to_q15(float x)
{
    if (x >= 0.999969482421875f)    return (int16_t)0x7FFF; //32767 = 0_111...1
    if (x <= -1.0f)                 return (int16_t)0x8000; //-32768 = 1_000...0

    int32_t q = (int32_t)lrintf(x * 32768.0f);
    if (q > 32767) q = 32767;
    if (q < -32768) q = -32768; 

    return (int16_t)q;
}

#define MAX_DEC 0x3FFF
#define MAX_ALPHA 15
#define MAX_SAT 31
int cmd_set_pid(cmd_ctx_t* ctx)
{
    size_t index = 0;

    float kp_f = ctx->float_args[0];
    float kd_f = ctx->float_args[1];
    float ki_f = ctx->float_args[2];
    float sp_f = ctx->float_args[3];
    float gain_f  = ctx->float_args[4];
    float bias_f  = ctx->float_args[5];  // optional, default 0.0 (zero-init ctx)
    float egain_f = ctx->float_args[6];  // Q10 input gain; Python always sends this (default 1.0)
    uint32_t dec = ctx->uint_args[0];
    uint32_t alpha = ctx->uint_args[1];
    uint32_t sat = ctx->uint_args[2];
    uint32_t en = ctx->uint_args[3];

    int16_t kp_i = float_to_q15(kp_f);
    int16_t kd_i = float_to_q15(kd_f);
    int16_t ki_i = float_to_q15(ki_f);

    int32_t sp_inter = (int32_t)lrintf(sp_f * 8192.0f);
    if(sp_inter > 8191) sp_inter = 8191;
    else if(sp_inter < -8192) sp_inter = -8192;

    int16_t sp_i = (int16_t)sp_inter;

    int32_t gain_inter = (int32_t)lrintf(gain_f * 1024.0f);
    if (gain_inter > 32767)  gain_inter = 32767;
    if (gain_inter < -32768) gain_inter = -32768;
    int16_t gain_i = (int16_t)gain_inter;

    int32_t egain_inter = (int32_t)lrintf(egain_f * 1024.0f);
    if (egain_inter > 32767)  egain_inter = 32767;
    if (egain_inter < -32768) egain_inter = -32768;
    int16_t egain_i = (int16_t)egain_inter;

    int32_t bias_inter = (int32_t)lrintf(bias_f * 8191.0f);
    if (bias_inter > 8191)  bias_inter = 8191;
    if (bias_inter < -8191) bias_inter = -8191;
    int16_t bias_i = (int16_t)bias_inter;

    if(dec > MAX_DEC) dec = MAX_DEC;
    if(alpha > MAX_ALPHA) alpha = MAX_ALPHA; 
    if(sat > MAX_SAT) sat = MAX_SAT;

    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.set_pid_cmd.cmd = CMD_SET_PID_COEFFS;
        
    cmd.set_pid_cmd.payload = kp_i;
    cmd.set_pid_cmd.coeff_sel = PID_SELECT_KP;
    pdh_callback_t cb = pdh_execute_cmd(cmd);
    int16_t echo_kp = (int16_t)cb.set_pid_cb.payload_r;
    float kp_converted = echo_kp / 32768.0f;
    int return_code = validate_cb(&echo_kp, &kp_i, INT16_TAG, __func__, "KP_CB", SET_PID_OK, SET_PID_INVALID_KP);
    push_ctx_cb(ctx, &index, &kp_converted, FLOAT_TAG, "KP_CB");

    cmd.set_pid_cmd.payload = kd_i;
    cmd.set_pid_cmd.coeff_sel = PID_SELECT_KD;
    cb = pdh_execute_cmd(cmd);
    int16_t echo_kd = (int16_t)cb.set_pid_cb.payload_r;
    float kd_converted = echo_kd / 32768.0f;
    return_code = validate_cb(&echo_kd, &kd_i, INT16_TAG, __func__, "KD_CB", return_code, SET_PID_INVALID_KD);
    push_ctx_cb(ctx, &index, &kd_converted, FLOAT_TAG, "KD_CB");

    cmd.set_pid_cmd.payload = ki_i;
    cmd.set_pid_cmd.coeff_sel = PID_SELECT_KI;
    cb = pdh_execute_cmd(cmd);
    int16_t echo_ki = (int16_t)cb.set_pid_cb.payload_r;
    float ki_converted = echo_ki / 32768.0f;
    return_code = validate_cb(&echo_ki, &ki_i, INT16_TAG, __func__, "KI_CB", return_code, SET_PID_INVALID_KI);
    push_ctx_cb(ctx, &index, &ki_converted, FLOAT_TAG, "KI_CB");

    cmd.set_pid_cmd.payload = dec;
    cmd.set_pid_cmd.coeff_sel = PID_SELECT_DEC;
    cb = pdh_execute_cmd(cmd);
    uint32_t echo_dec = cb.set_pid_cb.payload_r;
    return_code = validate_cb(&echo_dec, &dec, UINT_TAG, __func__, "DEC_CB", return_code, SET_PID_INVALID_DEC);
    push_ctx_cb(ctx, &index, &echo_dec, UINT_TAG, "DEC_CB");

    cmd.set_pid_cmd.payload = sp_i;
    cmd.set_pid_cmd.coeff_sel = PID_SELECT_SP;
    cb = pdh_execute_cmd(cmd);
    int16_t echo_sp = (int16_t)cb.set_pid_cb.payload_r;
    float sp_converted = echo_sp / 8192.0f;
    return_code = validate_cb(&echo_sp, &sp_i, INT16_TAG, __func__, "SP_CB", return_code, SET_PID_INVALID_SP);
    push_ctx_cb(ctx, &index, &sp_converted, FLOAT_TAG, "SP_CB");

    cmd.set_pid_cmd.payload = alpha;
    cmd.set_pid_cmd.coeff_sel = PID_SELECT_ALPHA;
    cb = pdh_execute_cmd(cmd);
    uint32_t echo_alpha = cb.set_pid_cb.payload_r;
    return_code = validate_cb(&echo_alpha, &alpha, UINT_TAG, __func__, "ALPHA_CB", return_code, SET_PID_INVALID_ALPHA);
    push_ctx_cb(ctx, &index, &echo_alpha, UINT_TAG, "ALPHA_CB");

    cmd.set_pid_cmd.payload = sat;
    cmd.set_pid_cmd.coeff_sel = PID_SELECT_SAT;
    cb = pdh_execute_cmd(cmd);
    uint32_t echo_sat = cb.set_pid_cb.payload_r;
    return_code = validate_cb(&echo_sat, &sat, UINT_TAG, __func__, "SAT_CB", return_code, SET_PID_INVALID_SAT);
    push_ctx_cb(ctx, &index, &echo_sat, UINT_TAG, "SAT_CB");

    cmd.set_pid_cmd.payload = en;
    cmd.set_pid_cmd.coeff_sel = PID_SELECT_EN;
    cb = pdh_execute_cmd(cmd);
    uint32_t echo_en = cb.set_pid_cb.payload_r;
    return_code = validate_cb(&echo_en, &en, UINT_TAG, __func__, "EN_CB", return_code, SET_PID_INVALID_EN);
    push_ctx_cb(ctx, &index, &echo_en, UINT_TAG, "EN_CB");

    cmd.set_pid_cmd.payload   = gain_i;
    cmd.set_pid_cmd.coeff_sel = PID_SELECT_GAIN;
    cb = pdh_execute_cmd(cmd);
    int16_t echo_gain = (int16_t)cb.set_pid_cb.payload_r;
    float gain_converted = echo_gain / 1024.0f;
    return_code = validate_cb(&echo_gain, &gain_i, INT16_TAG, __func__, "GAIN_CB",
                              return_code, SET_PID_INVALID_GAIN);
    push_ctx_cb(ctx, &index, &gain_converted, FLOAT_TAG, "GAIN_CB");

    cmd.set_pid_cmd.payload   = (uint16_t)bias_i;
    cmd.set_pid_cmd.coeff_sel = PID_SELECT_BIAS;
    cb = pdh_execute_cmd(cmd);
    int16_t echo_bias = (int16_t)cb.set_pid_cb.payload_r;
    float bias_converted = echo_bias / 8191.0f;
    return_code = validate_cb(&echo_bias, &bias_i, INT16_TAG, __func__, "BIAS_CB",
                              return_code, SET_PID_INVALID_BIAS);
    push_ctx_cb(ctx, &index, &bias_converted, FLOAT_TAG, "BIAS_CB");

    cmd.set_pid_cmd.payload   = egain_i;
    cmd.set_pid_cmd.coeff_sel = PID_SELECT_EGAIN;
    cb = pdh_execute_cmd(cmd);
    int16_t echo_egain = (int16_t)cb.set_pid_cb.payload_r;
    float egain_converted = echo_egain / 1024.0f;
    return_code = validate_cb(&echo_egain, &egain_i, INT16_TAG, __func__, "EGAIN_CB",
                              return_code, SET_PID_INVALID_EGAIN);
    push_ctx_cb(ctx, &index, &egain_converted, FLOAT_TAG, "EGAIN_CB");

    return return_code;
}





int cmd_set_fir(cmd_ctx_t* ctx)
{
    int return_code = SET_FIR_OK;
    size_t index = 0;

    uint32_t input_sel = ctx->uint_args[0];
    uint32_t num_coeffs = ctx->num_floats;

    if (input_sel > 3)
        return_code = SET_FIR_INVALID_INPUT_SEL;

    if (return_code == SET_FIR_OK)
    {
        for (uint32_t i = 0; i < num_coeffs; i++)
        {
            float v = ctx->float_args[i];
            if (v > 1.0f || v < -1.0f)
            {
                return_code = SET_FIR_INVALID_COEFF;
                break;
            }
        }
    }

    if (return_code == SET_FIR_OK)
    {
        pdh_cmd_t cmd;
        cmd.raw = 0;
        cmd.set_fir_cmd.cmd = CMD_SET_FIR;

        // Set FIR input source
        uint32_t input_sel_u32 = input_sel;
        cmd.set_fir_cmd.update_sel = FIR_SELECT_INPUT_SEL;
        cmd.set_fir_cmd.payload = input_sel_u32;
        pdh_callback_t cb = pdh_execute_cmd(cmd);
        uint32_t echo_input_sel = cb.set_fir_cb.payload_r & 0x7;
        return_code = validate_cb(&echo_input_sel, &input_sel_u32, UINT_TAG, __func__, "INPUT_SEL_CB", return_code, SET_FIR_INPUT_SEL_CB_FAIL);
        push_ctx_cb(ctx, &index, &echo_input_sel, UINT_TAG, "INPUT_SEL_CB");

        // Enable memory write before coefficient loading
        uint32_t mem_wen_1 = 1;
        cmd.set_fir_cmd.update_sel = FIR_SELECT_MEM_WRITE_EN;
        cmd.set_fir_cmd.payload = 1;
        cb = pdh_execute_cmd(cmd);
        uint32_t echo_mem_wen_en = cb.set_fir_cb.payload_r & 0x1;
        return_code = validate_cb(&echo_mem_wen_en, &mem_wen_1, UINT_TAG, __func__, "MEM_WEN_EN_CB", return_code, SET_FIR_MEM_WEN_CB_FAIL);
        push_ctx_cb(ctx, &index, &echo_mem_wen_en, UINT_TAG, "MEM_WEN_EN_CB");

        // Write each coefficient: set address then value
        for (uint32_t i = 0; i < num_coeffs; i++)
        {
            uint32_t addr = i;
            cmd.set_fir_cmd.update_sel = FIR_SELECT_ADDR;
            cmd.set_fir_cmd.payload = addr;
            cb = pdh_execute_cmd(cmd);
            uint32_t echo_addr = cb.set_fir_cb.payload_r & 0x1F;
            return_code = validate_cb(&echo_addr, &addr, UINT_TAG, __func__, "ADDR_CB", return_code, SET_FIR_ADDR_CB_FAIL);

            int16_t coeff_q15 = float_to_q15(ctx->float_args[i]);
            uint32_t coeff_u32 = (uint32_t)(uint16_t)coeff_q15;
            cmd.set_fir_cmd.update_sel = FIR_SELECT_COEFF;
            cmd.set_fir_cmd.payload = coeff_u32;
            cb = pdh_execute_cmd(cmd);
            uint32_t echo_coeff = (uint32_t)(cb.set_fir_cb.payload_r & 0xFFFF);
            return_code = validate_cb(&echo_coeff, &coeff_u32, UINT_TAG, __func__, "COEFF_CB", return_code, SET_FIR_COEFF_CB_FAIL);
        }

        // Disable memory write
        uint32_t mem_wen_0 = 0;
        cmd.set_fir_cmd.update_sel = FIR_SELECT_MEM_WRITE_EN;
        cmd.set_fir_cmd.payload = 0;
        cb = pdh_execute_cmd(cmd);
        uint32_t echo_mem_wen_dis = cb.set_fir_cb.payload_r & 0x1;
        return_code = validate_cb(&echo_mem_wen_dis, &mem_wen_0, UINT_TAG, __func__, "MEM_WEN_DIS_CB", return_code, SET_FIR_MEM_WEN_CB_FAIL);
        push_ctx_cb(ctx, &index, &echo_mem_wen_dis, UINT_TAG, "MEM_WEN_DIS_CB");

        // Enable chain write so data flows through the filter
        uint32_t chain_wen_1 = 1;
        cmd.set_fir_cmd.update_sel = FIR_SELECT_CHAIN_WRITE_EN;
        cmd.set_fir_cmd.payload = 1;
        cb = pdh_execute_cmd(cmd);
        uint32_t echo_chain_wen = cb.set_fir_cb.payload_r & 0x1;
        return_code = validate_cb(&echo_chain_wen, &chain_wen_1, UINT_TAG, __func__, "CHAIN_WEN_CB", return_code, SET_FIR_CHAIN_WEN_CB_FAIL);
        push_ctx_cb(ctx, &index, &echo_chain_wen, UINT_TAG, "CHAIN_WEN_CB");
    }

    return return_code;
}


#define SIN_SELECT 1
#define COS_SELECT 0
#define ROTATION_CONST 32768.0f


int cmd_set_rot(cmd_ctx_t* ctx)
{
    int return_code = SET_ROT_OK;
    size_t index = 0;

    float theta_rad = ctx->float_args[0];
    if (theta_rad > M_PI) theta_rad = M_PI;
    if (theta_rad < -1.0f * M_PI) theta_rad = -1.0f * M_PI;

    float c = cosf(theta_rad);
    float s = sinf(theta_rad);

    int16_t c_q15 = float_to_q15(c);
    int16_t s_q15 = float_to_q15(s);

/////////////////////////// SET COSINE TERM //////////////////////////////////////////////////
    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.set_rot_coeff_cmd.cmd = CMD_SET_ROT_COEFFS;
    cmd.set_rot_coeff_cmd.rot_payload = c_q15; 
    cmd.set_rot_coeff_cmd.rot_select = COS_SELECT;
    pdh_send_cmd(cmd);
    cmd.set_rot_coeff_cmd.strobe = 1;
    pdh_send_cmd(cmd);
    
///////////////////////// SET SINE TERM /////////////////////////////////////////////////////
    cmd.set_rot_coeff_cmd.strobe = 0;
    cmd.set_rot_coeff_cmd.rot_payload = s_q15;
    cmd.set_rot_coeff_cmd.rot_select = SIN_SELECT;
    pdh_send_cmd(cmd);
    cmd.set_rot_coeff_cmd.strobe = 1;
    pdh_send_cmd(cmd);

    pdh_callback_t cb; //TODO: These should check committed coeffs at end instead
    cb.raw = 0;
    pdh_get_callback(&cb);

    int16_t approx_cos_i = (int16_t)((((uint16_t)c_q15) >> 2) << 2);
    int16_t approx_sin_i = (int16_t)((((uint16_t)s_q15) >> 2) << 2);
    
    int16_t approx_cos_r = (int16_t)((uint16_t)cb.set_rot_coeff_cb.cos_theta_r << 2);
    int16_t approx_sin_r = (int16_t)((uint16_t)cb.set_rot_coeff_cb.sin_theta_r << 2);

    float cos_r_converted = approx_cos_r / ROTATION_CONST;
    float sin_r_converted = approx_sin_r / ROTATION_CONST;

    return_code = validate_cb(&approx_cos_r, &approx_cos_i, INT16_TAG, __func__, "COS_CB", return_code, SET_ROT_INVALID_COS);
    return_code = validate_cb(&approx_sin_r, &approx_sin_i, INT16_TAG, __func__, "SIN_CB", return_code, SET_ROT_INVALID_SIN); 

    push_ctx_cb(ctx, &index, &cos_r_converted, FLOAT_TAG, "COS_CB");
    push_ctx_cb(ctx, &index, &sin_r_converted, FLOAT_TAG, "SIN_CB");

    cmd.raw = 0;
    cmd.commit_rot_coeff_cmd.cmd = CMD_COMMIT_ROT_COEFFS;
    cb = pdh_execute_cmd(cmd);

    float i_feed_approx = ((int16_t)((uint16_t)cb.commit_rot_coeff_cb.i_feed << 2)) / 8192.0f;
    float q_feed_approx = ((int16_t)((uint16_t)cb.commit_rot_coeff_cb.q_feed << 2)) / 8192.0f;
   
    push_ctx_cb(ctx, &index, &i_feed_approx, FLOAT_TAG, "I_FEED_CB");
    push_ctx_cb(ctx, &index, &q_feed_approx, FLOAT_TAG, "Q_FEED_CB");

    return return_code;
}



#define STRIDE_CONST 7629.39453125 // 125E6/(4 * 4096)
int cmd_set_nco(cmd_ctx_t* ctx)
{
    int return_code = SET_NCO_OK;
    size_t index = 0;

    float freq = ctx->float_args[0];
    float shift_deg = ctx->float_args[1] * -1.0f;
    uint32_t en = ctx->uint_args[0];

    uint32_t stride = ABS(lrintf(freq / STRIDE_CONST));

    if (stride > 1024 || stride < 1) return_code = SET_NCO_INVALID_FREQ;

    //shift should be given as between 180 and -180 degrees
    float shift_rad = shift_deg / 180.0 * M_PI;
    if (shift_rad > M_PI || shift_rad < -M_PI) return_code = SET_NCO_INVALID_SHIFT;


    if (return_code == SET_NCO_OK)
    {
        uint32_t inv = 0; 
        uint32_t sub = 0;
        
        if (shift_rad < 0)
        {
            shift_rad += 2*M_PI;
        }

        if (shift_rad > M_PI)
        {
            shift_rad -= M_PI;
            inv = ~inv;
        }

        if (shift_rad > M_PI_2)
        {
            sub = 1;
            inv = ~inv;
            shift_rad = -(shift_rad - M_PI);
        }


        inv &= 0x00000001;

        int32_t shift_int = lrintf(shift_rad / M_PI_2 * 4095.0);
        uint32_t shift_int_unsigned = ABS(shift_int);

        pdh_cmd_t cmd;
        cmd.raw = 0;
        cmd.set_nco_cmd.cmd = CMD_SET_NCO;
        cmd.set_nco_cmd.coeff_sel = NCO_SELECT_EN;
        pdh_execute_cmd(cmd);

        cmd.set_nco_cmd.coeff_sel = NCO_SELECT_STRIDE;
        cmd.set_nco_cmd.payload = stride;
        pdh_execute_cmd(cmd);

        cmd.set_nco_cmd.coeff_sel = NCO_SELECT_SHIFT;
        cmd.set_nco_cmd.payload = shift_int_unsigned;
        pdh_execute_cmd(cmd);

        cmd.set_nco_cmd.coeff_sel = NCO_SELECT_INV;
        cmd.set_nco_cmd.payload = inv;
        pdh_execute_cmd(cmd);

        cmd.set_nco_cmd.coeff_sel = NCO_SELECT_SUB;
        cmd.set_nco_cmd.payload = sub;
        pdh_execute_cmd(cmd);

        cmd.set_nco_cmd.coeff_sel = NCO_SELECT_EN;
        cmd.set_nco_cmd.payload = en;
        pdh_callback_t cb = pdh_execute_cmd(cmd);
        

        uint32_t echo_stride = cb.set_nco_cb.nco_stride_r;
        uint32_t echo_shift = cb.set_nco_cb.nco_shift_r;
        uint32_t echo_sub = cb.set_nco_cb.nco_sub_r;
        uint32_t echo_inv = cb.set_nco_cb.nco_inv_r;
        uint32_t echo_en = cb.set_nco_cb.nco_en_r;

        return_code = validate_cb(&echo_stride, &stride, UINT_TAG, __func__, "STRIDE_CB", return_code, SET_NCO_INVALID_STRIDE_CB);
        push_ctx_cb(ctx, &index, &echo_stride, UINT_TAG, "STRIDE_CB");
        
        return_code = validate_cb(&echo_shift, &shift_int_unsigned, UINT_TAG, __func__, "SHIFT_CB", return_code, SET_NCO_INVALID_SHIFT_CB);
        push_ctx_cb(ctx, &index, &echo_shift, UINT_TAG, "SHIFT_CB");
        
        return_code = validate_cb(&echo_sub, &sub, UINT_TAG, __func__, "SUB_CB", return_code, SET_NCO_INVALID_SUB_CB);
        push_ctx_cb(ctx, &index, &echo_sub, UINT_TAG, "SUB_CB");
        
        return_code = validate_cb(&echo_inv, &inv, UINT_TAG, __func__, "INV_CB", return_code, SET_NCO_INVALID_INV_CB);
        push_ctx_cb(ctx, &index, &echo_inv, UINT_TAG, "INV_CB");

        return_code = validate_cb(&echo_en, &en, UINT_TAG, __func__, "EN_CB", return_code, SET_NCO_INVALID_EN_CB);
        push_ctx_cb(ctx, &index, &echo_en, UINT_TAG, "EN_CB");


        float echo_shift_fractional = M_PI_2 * echo_shift / 4095.0f;
        if (sub) echo_shift_fractional *= -1.0f;
        if (inv) echo_shift_fractional += M_PI;
    
        if (echo_shift_fractional > M_PI) echo_shift_fractional -= 2.0f * M_PI;

        float echo_shift_degrees = -180.0 * echo_shift_fractional / M_PI;
        float echo_shift_error = -shift_deg - echo_shift_degrees;
        float echo_freq = echo_stride * STRIDE_CONST;
        float echo_freq_error = freq - echo_freq;

        push_ctx_cb(ctx, &index, &echo_shift_degrees, FLOAT_TAG, "REGISTERED_PHASE_SHIFT");
        push_ctx_cb(ctx, &index, &echo_shift_error, FLOAT_TAG, "REGISTERED_SHIFT_ERROR");
        push_ctx_cb(ctx, &index, &echo_freq, FLOAT_TAG, "REGISTERED_FREQ");
        push_ctx_cb(ctx, &index, &echo_freq_error, FLOAT_TAG, "REGISTERED_FREQ_ERROR");
    }



    return return_code;
}









#define DMA_BURST_CONST 330 //ceil10(16384 * 2.5) / 125)
#define BRAM_DEC_CONST 140 //ceil10(16384 / 125)
int cmd_get_frame(cmd_ctx_t* ctx) //This whole thing is sort of a hacky timing exploit right now and should probably be changed later
{
    uint32_t decimation_code = ctx->uint_args[0];
    if(decimation_code < 1) decimation_code = 1; //TODO: Proper handling of invalid frame and decimation codes
    
    uint32_t frame_code = ctx->uint_args[1];

    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.cmd.val = CMD_IDLE;
    pdh_execute_cmd(cmd);

    cmd.raw = 0;
    cmd.get_frame_cmd.cmd = CMD_GET_FRAME;
    cmd.get_frame_cmd.decimation = decimation_code;
    cmd.get_frame_cmd.frame_code = frame_code;
    pdh_callback_t cb = pdh_execute_cmd(cmd);

    uint32_t echo_engaged = cb.get_frame_cb.dma_engaged;
    //uint32_t dummy_engaged = 1;

    uint32_t echo_dec = cb.get_frame_cb.decimation;
    uint32_t echo_frame = cb.get_frame_cb.frame_code;
    uint32_t echo_cmd = cb.get_frame_cb.cmd;

    uint32_t cmdval = cmd.cmd.val;

    //int return_code = validate_cb(&echo_engaged, &dummy_engaged, UINT_TAG, __func__, "DMA_ENGAGED_CB", GET_FRAME_OK, GET_FRAME_NOT_ENGAGED); 
    int return_code = validate_cb(&echo_dec, &decimation_code, UINT_TAG, __func__, "DECIMATION_CODE_CB", GET_FRAME_OK, GET_FRAME_INVALID_DEC);
    return_code = validate_cb(&echo_frame, &frame_code, UINT_TAG, __func__, "FRAME_CODE_CB", return_code, GET_FRAME_INVALID_CODE);
    return_code = validate_cb(&echo_cmd, &cmdval, UINT_TAG, __func__, CMD, return_code, PDH_INVALID_CMD);

    size_t index = 0;
    push_ctx_cb(ctx, &index, &echo_engaged, UINT_TAG, "DMA_ENGAGED_CB"); 
    push_ctx_cb(ctx, &index, &echo_dec, UINT_TAG, "DECIMATION_CODE_CB"); 
    push_ctx_cb(ctx, &index, &echo_frame, UINT_TAG, "FRAME_CODE_CB");
    push_ctx_cb(ctx, &index, &echo_cmd, UINT_TAG, CMD);
    

    usleep(DMA_BURST_CONST + (BRAM_DEC_CONST * decimation_code)); //TODO: Handle waiting for DMA finish better


    //TODO: Look into implementing this via arena stream
    FILE* f = fopen("dma_log.csv", "w");
    if (!f) return_code = DMA_FOPEN_ERR;

    //TODO: Make this more efficient
    if (return_code == DMA_OK)
    {

        for(size_t offset = 0; offset < HP0_RANGE; offset += 8)
        {
            dma_frame_t frame;
            frame.raw = dma_get_frame(offset);

            switch(frame_code) 
            {
                case ADC_DATA_IN:
                    fprintf(f, "%d, %d, %d, %d\n", (int16_t)frame.adc_data_in_frame.adc_dat_a_16s, (int16_t)frame.adc_data_in_frame.adc_dat_b_16s, (int16_t)frame.adc_data_in_frame.i_feed_w, (int16_t)frame.adc_data_in_frame.q_feed_w);
                    break;

                case PID_ERR_TAPS:
                    fprintf(f, "%d, %d, %d, %d\n", (int16_t)frame.pid_err_taps_frame.err_tap_w, (int16_t)frame.pid_err_taps_frame.perr_tap_w, (int16_t)frame.pid_err_taps_frame.derr_tap_w, (int16_t)frame.pid_err_taps_frame.ierr_tap_w);
                    break;

                case IO_SUM_ERR:
                    fprintf(f, "%d, %u, %d\n", (int16_t)frame.io_sum_err_frame.err_tap_w, (uint16_t)frame.io_sum_err_frame.pid_out_w, (int32_t)frame.io_sum_err_frame.sum_err_tap_w);
                    break;

                case OSC_INSPECT:
                    fprintf(f, "%d, %d, %u, %u\n", (int16_t)frame.osc_inspect_frame.nco_out1_r, (int16_t)frame.osc_inspect_frame.nco_out2_r, (uint16_t)frame.osc_inspect_frame.nco_feed1_r, (uint16_t)frame.osc_inspect_frame.nco_feed2_r);
                    break;

                case OSC_ADDR_CHECK:
                    fprintf(f, "%u, %u, %u, %u\n", frame.addr_check_frame.phi1_w, frame.addr_check_frame.phi2_w, frame.addr_check_frame.addr1_r, frame.addr_check_frame.addr2_r);
                    break;

                case LOOPBACK:
                    fprintf(f, "%u, %u, %u, %u\n", frame.loopback_frame.dac1_feed_w, frame.loopback_frame.dac2_feed_w, frame.loopback_frame.adc_dat_a_i, frame.loopback_frame.adc_dat_b_i);
                    break;

                case FIR_IO:
                    fprintf(f, "%d, %d\n", (int16_t)frame.fir_io_frame.fir_in_w, (int16_t)frame.fir_io_frame.fir_out_w);
                    break;

                case PID_IO:
                    fprintf(f, "%d, %d, %u\n",
                            frame.pid_io_frame.pid_in,
                            frame.pid_io_frame.err,
                            frame.pid_io_frame.pid_out & 0x3FFF);
                    break;

                default:
                    fprintf(f, "%d, %d, %d, %d\n", (int16_t)frame.adc_data_in_frame.adc_dat_a_16s, (int16_t)frame.adc_data_in_frame.adc_dat_b_16s, (int16_t)frame.adc_data_in_frame.i_feed_w, (int16_t)frame.adc_data_in_frame.q_feed_w); 
                    break;
            }
        }

        fclose(f);
    }

    ctx->output.output_items[4].data.u = return_code; 
    ctx->output.output_items[4].tag = UINT_TAG;
    strcpy(ctx->output.output_items[4].name, "return_code");

    ctx->output.num_outputs = 5;

    return return_code;
}


int cmd_config_io(cmd_ctx_t* ctx)
{
    int return_code = CONFIG_IO_OK;
    size_t index = 0; 

    uint32_t dac1_code = ctx->uint_args[0];
    uint32_t dac2_code = ctx->uint_args[1];
    uint32_t pid_code = ctx->uint_args[2];

    if(dac1_code > 3) return_code = CONFIG_IO_INVALID_DAC1;
    if(dac2_code > 3) return_code = CONFIG_IO_INVALID_DAC2;
    if(pid_code > 4) return_code = CONFIG_IO_INVALID_PID;

    if(return_code == CONFIG_IO_OK)
    {
        pdh_cmd_t cmd;
        cmd.config_io_cmd.cmd = CMD_CONFIG_IO;
        cmd.config_io_cmd.dac1_dat_sel = dac1_code;
        cmd.config_io_cmd.dac2_dat_sel = dac2_code;
        cmd.config_io_cmd.pid_dat_sel = pid_code;

        pdh_callback_t cb = pdh_execute_cmd(cmd);
        uint32_t echo_dac1 = cb.config_io_cb.dac1_dat_sel_r;
        uint32_t echo_dac2 = cb.config_io_cb.dac2_dat_sel_r;
        uint32_t echo_pid = cb.config_io_cb.pid_dat_sel_r;
        uint32_t echo_cmd = cb.config_io_cb.cmd;
        uint32_t cmdval = cmd.cmd.val;

        return_code = validate_cb(&echo_dac1, &dac1_code, UINT_TAG, __func__, "DAC1_DAT_SEL_CB", return_code, CONFIG_IO_DAC1_CB_FAIL);
        return_code = validate_cb(&echo_dac2, &dac2_code, UINT_TAG, __func__, "DAC2_DAT_SEL_CB", return_code, CONFIG_IO_DAC2_CB_FAIL);
        return_code = validate_cb(&echo_pid, &pid_code, UINT_TAG, __func__, "PID_DAT_SEL_CB", return_code, CONFIG_IO_PID_CB_FAIL);
        return_code = validate_cb(&echo_cmd, &cmdval, UINT_TAG, __func__, CMD, return_code, PDH_INVALID_CMD);
        
        push_ctx_cb(ctx, &index, &echo_dac1, UINT_TAG, "DAC1_DAT_SEL_CB"); 
        push_ctx_cb(ctx, &index, &echo_dac2, UINT_TAG, "DAC2_DAT_SEL_CB"); 
        push_ctx_cb(ctx, &index, &echo_pid, UINT_TAG, "PID_DAT_SEL_CB");
    }

    return return_code;
}


int cmd_test_frame(cmd_ctx_t* ctx)
{
    // uint32_t byte_offset = ctx->uint_args[0];
    //
    // dma_frame_t frame;
    // frame.raw = dma_get_frame(byte_offset);
    //
    // ctx->output.output_items[0].data.i = (int16_t)frame.adc_data_in_frame.sin_theta_r;
    // ctx->output.output_items[0].tag = INT_TAG;
    // strcpy(ctx->output.output_items[0].name, "sin_theta_r");
    //
    // ctx->output.output_items[1].data.i = (int16_t)frame.adc_data_in_frame.cos_theta_r;
    // ctx->output.output_items[1].tag = INT_TAG;
    // strcpy(ctx->output.output_items[1].name, "cos_theta_r");
    //
    // ctx->output.output_items[2].data.i = (int16_t)frame.adc_data_in_frame.q_feed_w;
    // ctx->output.output_items[2].tag = INT_TAG;
    // strcpy(ctx->output.output_items[2].name, "q_feed_w");
    //
    // ctx->output.output_items[3].data.i = (int16_t)frame.adc_data_in_frame.i_feed_w;
    // ctx->output.output_items[3].tag = INT_TAG;
    // strcpy(ctx->output.output_items[3].name, "i_feed_w");
    //
    // ctx->output.num_outputs = 4;
    return PDH_OK;
}


#define DAC_ZERO 8192

#define SWEEP_RAMP_OK           0
#define SWEEP_RAMP_INVALID_RANGE  1
#define SWEEP_RAMP_INVALID_POINTS 2
#define SWEEP_RAMP_FOPEN_ERR    3

int cmd_sweep_ramp(cmd_ctx_t* ctx)
{
    float    v0         = ctx->float_args[0];
    float    v1         = ctx->float_args[1];
    uint32_t num_points = ctx->uint_args[0];
    uint32_t dac_sel    = ctx->uint_args[1];
    uint32_t delay_us   = ctx->uint_args[2];

    if (v0 < -1.0f || v0 > 1.0f || v1 < -1.0f || v1 > 1.0f)
        return SWEEP_RAMP_INVALID_RANGE;
    if (num_points < 2 || num_points > 16384)
        return SWEEP_RAMP_INVALID_POINTS;

    float step = (v1 - v0) / (float)num_points;

    pdh_cmd_t set_dac_cmd;
    set_dac_cmd.raw = 0;
    set_dac_cmd.dac_cmd.cmd     = CMD_SET_DAC;
    set_dac_cmd.dac_cmd.dac_sel = dac_sel;

    pdh_cmd_t cs_cmd;
    cs_cmd.raw = 0;
    cs_cmd.cs_cmd.cmd = CMD_CHECK_SIGNED;

    static const cs_sel_e sels[4] = {
        CHECK_ADC_DAT_A_16S,
        CHECK_ADC_DAT_B_16S,
        CHECK_I_FEED,
        CHECK_Q_FEED,
    };

    // Read current IO routing and force DAC1 to register-driven mode
    pdh_cmd_t io_cmd;
    io_cmd.raw = 0;
    io_cmd.cs_cmd.cmd     = CMD_CHECK_SIGNED;
    io_cmd.cs_cmd.reg_sel = CHECK_IO;
    pdh_callback_t io_cb  = pdh_execute_cmd(io_cmd);

    uint32_t orig_dac1 = io_cb.check_io_cb.dac1_dat_sel_r;
    uint32_t orig_dac2 = io_cb.check_io_cb.dac2_dat_sel_r;
    uint32_t orig_pid  = io_cb.check_io_cb.pid_dat_sel_r;

    pdh_cmd_t cfg_cmd;
    cfg_cmd.raw = 0;
    cfg_cmd.config_io_cmd.cmd          = CMD_CONFIG_IO;
    cfg_cmd.config_io_cmd.dac1_dat_sel = SELECT_REGISTER;
    cfg_cmd.config_io_cmd.dac2_dat_sel = orig_dac2;
    cfg_cmd.config_io_cmd.pid_dat_sel  = orig_pid;
    pdh_execute_cmd(cfg_cmd);

    FILE *f = fopen("sweep_log.csv", "w");
    if (!f) return SWEEP_RAMP_FOPEN_ERR;

    for (uint32_t i = 0; i < num_points; i++)
    {
        float v = v0 + i * step;

        int32_t code_i = DAC_ZERO - (int32_t)(v * 8192);
        if (code_i < 0)     code_i = 0;
        if (code_i > 16383) code_i = 16383;
        set_dac_cmd.dac_cmd.dac_code = (uint16_t)code_i;
        pdh_execute_cmd(set_dac_cmd);

        if (delay_us > 0) usleep(delay_us);

        float vals[4] = {0};
        for (int s = 0; s < 4; s++)
        {
            cs_cmd.cs_cmd.reg_sel = sels[s];
            pdh_callback_t cb = pdh_execute_cmd(cs_cmd);
            vals[s] = (int16_t)cb.cs_cb.payload / 8192.0f;
        }

        fprintf(f, "%f,%f,%f,%f,%f\n", v, vals[0], vals[1], vals[2], vals[3]);
    }

    fclose(f);

    // Restore original IO routing
    cfg_cmd.raw = 0;
    cfg_cmd.config_io_cmd.cmd          = CMD_CONFIG_IO;
    cfg_cmd.config_io_cmd.dac1_dat_sel = orig_dac1;
    cfg_cmd.config_io_cmd.dac2_dat_sel = orig_dac2;
    cfg_cmd.config_io_cmd.pid_dat_sel  = orig_pid;
    pdh_execute_cmd(cfg_cmd);

    size_t index = 0;
    push_ctx_cb(ctx, &index, &num_points, UINT_TAG, "NUM_POINTS_CB");

    return SWEEP_RAMP_OK;
}


/* ═══════════════════════════════════════════════════════════════════════════
 * Interrupt-driven split handlers
 *
 * _send: builds the command word, calls pdh_strobe_cmd, returns initial status.
 *        Thread 1 calls this and pushes (ctx, fd) to the pending queue.
 *
 * _cb  : called by Thread 2 after the UIO interrupt fires.
 *        Receives the callback register value, validates, fills ctx output,
 *        and returns the final func_status for send_response().
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ── cmd_set_led ─────────────────────────────────────────────────────────── */

int cmd_set_led_send(cmd_ctx_t* ctx)
{
    /* Always strobe so Thread 2's interrupt fires.
     * The FPGA LED field is 8-bit; led_cmd.led_code naturally truncates.
     * Range validation is done in _cb via callback echo comparison. */
    uint32_t led_code = ctx->uint_args[0];
    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.led_cmd.cmd      = CMD_SET_LED;
    cmd.led_cmd.led_code = led_code & 0xFF;
    pdh_strobe_cmd(cmd);
    return SET_LED_OK;
}

int cmd_set_led_cb(cmd_ctx_t* ctx, pdh_callback_t cb)
{
    uint32_t led_code  = ctx->uint_args[0];
    uint32_t echo_code = cb.led_cb.func_callback;
    uint32_t echo_cmd  = cb.led_cb.cmd;

    size_t index = 0;
    push_ctx_cb(ctx, &index, &echo_code, UINT_TAG, "LED_CODE_CB");
    push_ctx_cb(ctx, &index, &echo_cmd,  UINT_TAG, CMD);

    /* If led_code > 255 the FPGA will have truncated it; echo mismatch catches this. */
    int rc = validate_cb(&echo_code, &led_code, UINT_TAG, __func__, "LED_CODE", SET_LED_OK, SET_LED_INVALID_LED_CB);
    uint32_t cmdval = CMD_SET_LED;
    return validate_cb(&echo_cmd, &cmdval, UINT_TAG, __func__, CMD, rc, PDH_INVALID_CMD);
}


/* ── cmd_set_dac ─────────────────────────────────────────────────────────── */

int cmd_set_dac_send(cmd_ctx_t* ctx)
{
    float val     = ctx->float_args[0];
    bool  dac_sel = (bool)ctx->uint_args[0];

    val *= -1.0f;
    if (val >  1.0f) val =  1.0f;
    if (val < -1.0f) val = -1.0f;

    float y    = (val + 1.0f) * 0.5f;
    int   code = (int)lrintf(y * 16383.0f);
    if (code < 0)     code = 0;
    if (code > 16383) code = 16383;

    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.dac_cmd.dac_code = code;
    cmd.dac_cmd.dac_sel  = dac_sel;
    cmd.dac_cmd.cmd      = CMD_SET_DAC;
    pdh_strobe_cmd(cmd);
    return SET_DAC_OK;
}

int cmd_set_dac_cb(cmd_ctx_t* ctx, pdh_callback_t cb)
{
    float val     = ctx->float_args[0];
    bool  dac_sel = (bool)ctx->uint_args[0];

    val *= -1.0f;
    if (val >  1.0f) val =  1.0f;
    if (val < -1.0f) val = -1.0f;
    float y    = (val + 1.0f) * 0.5f;
    int   code = (int)lrintf(y * 16383.0f);
    if (code < 0)     code = 0;
    if (code > 16383) code = 16383;

    uint32_t echo_dac1 = cb.dac_cb.dac1_code;
    uint32_t echo_dac2 = cb.dac_cb.dac2_code;
    uint32_t echo_cmd  = cb.dac_cb.cmd;

    size_t index = 0;
    push_ctx_cb(ctx, &index, &echo_dac1, UINT_TAG, "DAC1_CODE_CB");
    push_ctx_cb(ctx, &index, &echo_dac2, UINT_TAG, "DAC2_CODE_CB");

    int rc = SET_DAC_OK;
    if (dac_sel == 0)
        rc = validate_cb(&echo_dac1, &code, UINT_TAG, __func__, "DAC1_CODE_CB", rc, SET_DAC_INVALID_CODE);
    else
        rc = validate_cb(&echo_dac2, &code, UINT_TAG, __func__, "DAC2_CODE_CB", rc, SET_DAC_INVALID_CODE);

    uint32_t cmdval = CMD_SET_DAC;
    return validate_cb(&echo_cmd, &cmdval, UINT_TAG, __func__, "CMD", rc, PDH_INVALID_CMD);
}


/* ── cmd_get_adc ─────────────────────────────────────────────────────────── */

int cmd_get_adc_send(cmd_ctx_t* ctx)
{
    (void)ctx;
    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.adc_cmd.cmd = CMD_GET_ADC;
    pdh_strobe_cmd(cmd);
    return GET_ADC_OK;
}

int cmd_get_adc_cb(cmd_ctx_t* ctx, pdh_callback_t cb)
{
    uint32_t echo_adc0  = cb.adc_cb.adc_0_code;
    uint32_t echo_adc1  = cb.adc_cb.adc_1_code;
    float    adc0_v     = -1.0f * (echo_adc0 * (2.0f / 16383.0f) - 1.0f);
    float    adc1_v     = -1.0f * (echo_adc1 * (2.0f / 16383.0f) - 1.0f);
    uint32_t echo_cmd   = cb.adc_cb.cmd;

    size_t index = 0;
    push_ctx_cb(ctx, &index, &echo_adc0, UINT_TAG,  "IN1");
    push_ctx_cb(ctx, &index, &echo_adc1, UINT_TAG,  "IN2");
    push_ctx_cb(ctx, &index, &adc0_v,   FLOAT_TAG,  "IN1_V");
    push_ctx_cb(ctx, &index, &adc1_v,   FLOAT_TAG,  "IN2_V");
    push_ctx_cb(ctx, &index, &echo_cmd, UINT_TAG,   CMD);

    uint32_t cmdval = CMD_GET_ADC;
    return validate_cb(&echo_cmd, &cmdval, UINT_TAG, __func__, CMD, GET_ADC_OK, PDH_INVALID_CMD);
}


/* ── cmd_check_signed ────────────────────────────────────────────────────── */

int cmd_check_signed_send(cmd_ctx_t* ctx)
{
    uint32_t reg_sel = ctx->uint_args[0];
    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.cs_cmd.cmd     = CMD_CHECK_SIGNED;
    cmd.cs_cmd.reg_sel = reg_sel;
    pdh_strobe_cmd(cmd);
    return CHECK_SIGNED_OK;
}

int cmd_check_signed_cb(cmd_ctx_t* ctx, pdh_callback_t cb)
{
    uint32_t reg_sel      = ctx->uint_args[0];
    uint32_t echo_reg_sel = cb.cs_cb.reg_sel;
    int16_t  echo_p16     = (int16_t)cb.cs_cb.payload;
    int32_t  echo_p32     = (int32_t)echo_p16;
    uint32_t echo_cmd     = cb.cs_cb.cmd;

    size_t index = 0;
    push_ctx_cb(ctx, &index, &echo_reg_sel, UINT_TAG, "REG_SEL_CB");
    push_ctx_cb(ctx, &index, &echo_p32,     INT_TAG,  "REG_VALUE_CB");
    push_ctx_cb(ctx, &index, &echo_cmd,     UINT_TAG, CMD);

    int rc = validate_cb(&echo_reg_sel, &reg_sel, UINT_TAG, __func__, "REG_SEL", CHECK_SIGNED_OK, CHECK_SIGNED_INVALID_REG_SEL_CB);
    uint32_t cmdval = CMD_CHECK_SIGNED;
    return validate_cb(&echo_cmd, &cmdval, UINT_TAG, __func__, "CMD", rc, PDH_INVALID_CMD);
}


/* ── cmd_config_io ───────────────────────────────────────────────────────── */

int cmd_config_io_send(cmd_ctx_t* ctx)
{
    /* Always strobe. Values are 3-bit fields; FPGA masks upper bits.
     * Range validation (> 3 / > 4) is caught in _cb via echo mismatch. */
    uint32_t dac1 = ctx->uint_args[0];
    uint32_t dac2 = ctx->uint_args[1];
    uint32_t pid  = ctx->uint_args[2];

    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.config_io_cmd.cmd          = CMD_CONFIG_IO;
    cmd.config_io_cmd.dac1_dat_sel = dac1 & 0x7;
    cmd.config_io_cmd.dac2_dat_sel = dac2 & 0x7;
    cmd.config_io_cmd.pid_dat_sel  = pid  & 0x7;
    pdh_strobe_cmd(cmd);
    return CONFIG_IO_OK;
}

int cmd_config_io_cb(cmd_ctx_t* ctx, pdh_callback_t cb)
{
    uint32_t dac1 = ctx->uint_args[0];
    uint32_t dac2 = ctx->uint_args[1];
    uint32_t pid  = ctx->uint_args[2];

    /* Pre-strobe range checks (surfaced here so the error code is preserved) */
    if (dac1 > 3) return CONFIG_IO_INVALID_DAC1;
    if (dac2 > 3) return CONFIG_IO_INVALID_DAC2;
    if (pid  > 4) return CONFIG_IO_INVALID_PID;

    uint32_t echo_dac1 = cb.config_io_cb.dac1_dat_sel_r;
    uint32_t echo_dac2 = cb.config_io_cb.dac2_dat_sel_r;
    uint32_t echo_pid  = cb.config_io_cb.pid_dat_sel_r;
    uint32_t echo_cmd  = cb.config_io_cb.cmd;
    uint32_t cmdval    = CMD_CONFIG_IO;

    int rc = validate_cb(&echo_dac1, &dac1, UINT_TAG, __func__, "DAC1_DAT_SEL_CB", CONFIG_IO_OK, CONFIG_IO_DAC1_CB_FAIL);
    rc = validate_cb(&echo_dac2, &dac2, UINT_TAG, __func__, "DAC2_DAT_SEL_CB", rc, CONFIG_IO_DAC2_CB_FAIL);
    rc = validate_cb(&echo_pid,  &pid,  UINT_TAG, __func__, "PID_DAT_SEL_CB",  rc, CONFIG_IO_PID_CB_FAIL);
    rc = validate_cb(&echo_cmd, &cmdval, UINT_TAG, __func__, CMD, rc, PDH_INVALID_CMD);

    size_t index = 0;
    push_ctx_cb(ctx, &index, &echo_dac1, UINT_TAG, "DAC1_DAT_SEL_CB");
    push_ctx_cb(ctx, &index, &echo_dac2, UINT_TAG, "DAC2_DAT_SEL_CB");
    push_ctx_cb(ctx, &index, &echo_pid,  UINT_TAG, "PID_DAT_SEL_CB");
    return rc;
}


/* ── cmd_get_frame ───────────────────────────────────────────────────────── */

int cmd_get_frame_send(cmd_ctx_t* ctx)
{
    uint32_t decimation_code = ctx->uint_args[0];
    if (decimation_code < 1) decimation_code = 1;
    uint32_t frame_code_val = ctx->uint_args[1];

    /* Reset DMA state machine (CMD_IDLE generates no interrupt) */
    pdh_cmd_t idle_cmd;
    idle_cmd.raw      = 0;
    idle_cmd.cmd.val  = CMD_IDLE;
    pdh_strobe_cmd(idle_cmd);

    /* Send CMD_GET_FRAME — interrupt fires later on dma_ready_edge_w */
    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.get_frame_cmd.cmd        = CMD_GET_FRAME;
    cmd.get_frame_cmd.decimation = decimation_code;
    cmd.get_frame_cmd.frame_code = frame_code_val;
    pdh_strobe_cmd(cmd);

    /* Read the immediate callback echo (dec, frame_code, cmd) */
    pdh_callback_t cb = {0};
    pdh_get_callback(&cb);

    uint32_t echo_engaged = cb.get_frame_cb.dma_engaged;
    uint32_t echo_dec     = cb.get_frame_cb.decimation;
    uint32_t echo_frame   = cb.get_frame_cb.frame_code;
    uint32_t echo_cmd_val = cb.get_frame_cb.cmd;
    uint32_t cmdval       = CMD_GET_FRAME;

    int rc = validate_cb(&echo_dec,     &decimation_code, UINT_TAG, __func__, "DECIMATION_CODE_CB", GET_FRAME_OK, GET_FRAME_INVALID_DEC);
    rc     = validate_cb(&echo_frame,   &frame_code_val,  UINT_TAG, __func__, "FRAME_CODE_CB",     rc,            GET_FRAME_INVALID_CODE);
    rc     = validate_cb(&echo_cmd_val, &cmdval,          UINT_TAG, __func__, CMD,                 rc,            PDH_INVALID_CMD);

    size_t index = 0;
    push_ctx_cb(ctx, &index, &echo_engaged, UINT_TAG, "DMA_ENGAGED_CB");
    push_ctx_cb(ctx, &index, &echo_dec,     UINT_TAG, "DECIMATION_CODE_CB");
    push_ctx_cb(ctx, &index, &echo_frame,   UINT_TAG, "FRAME_CODE_CB");
    push_ctx_cb(ctx, &index, &echo_cmd_val, UINT_TAG, CMD);

    return rc;
}

int cmd_get_frame_cb(cmd_ctx_t* ctx, pdh_callback_t cb)
{
    (void)cb;   /* DMA data is in DDR, not the callback register */
    uint32_t frame_code_val = ctx->uint_args[1];

    FILE* f = fopen("dma_log.csv", "w");
    int return_code = f ? DMA_OK : DMA_FOPEN_ERR;

    if (return_code == DMA_OK)
    {
        for (size_t offset = 0; offset < HP0_RANGE; offset += 8)
        {
            dma_frame_t frame;
            frame.raw = dma_get_frame(offset);

            switch (frame_code_val)
            {
                case ADC_DATA_IN:
                    fprintf(f, "%d, %d, %d, %d\n",
                            (int16_t)frame.adc_data_in_frame.adc_dat_a_16s,
                            (int16_t)frame.adc_data_in_frame.adc_dat_b_16s,
                            (int16_t)frame.adc_data_in_frame.i_feed_w,
                            (int16_t)frame.adc_data_in_frame.q_feed_w);
                    break;
                case PID_ERR_TAPS:
                    fprintf(f, "%d, %d, %d, %d\n",
                            (int16_t)frame.pid_err_taps_frame.err_tap_w,
                            (int16_t)frame.pid_err_taps_frame.perr_tap_w,
                            (int16_t)frame.pid_err_taps_frame.derr_tap_w,
                            (int16_t)frame.pid_err_taps_frame.ierr_tap_w);
                    break;
                case IO_SUM_ERR:
                    fprintf(f, "%d, %u, %d\n",
                            (int16_t)frame.io_sum_err_frame.err_tap_w,
                            (uint16_t)frame.io_sum_err_frame.pid_out_w,
                            (int32_t)frame.io_sum_err_frame.sum_err_tap_w);
                    break;
                case OSC_INSPECT:
                    fprintf(f, "%d, %d, %u, %u\n",
                            (int16_t)frame.osc_inspect_frame.nco_out1_r,
                            (int16_t)frame.osc_inspect_frame.nco_out2_r,
                            (uint16_t)frame.osc_inspect_frame.nco_feed1_r,
                            (uint16_t)frame.osc_inspect_frame.nco_feed2_r);
                    break;
                case OSC_ADDR_CHECK:
                    fprintf(f, "%u, %u, %u, %u\n",
                            frame.addr_check_frame.phi1_w,
                            frame.addr_check_frame.phi2_w,
                            frame.addr_check_frame.addr1_r,
                            frame.addr_check_frame.addr2_r);
                    break;
                case LOOPBACK:
                    fprintf(f, "%u, %u, %u, %u\n",
                            frame.loopback_frame.dac1_feed_w,
                            frame.loopback_frame.dac2_feed_w,
                            frame.loopback_frame.adc_dat_a_i,
                            frame.loopback_frame.adc_dat_b_i);
                    break;
                case FIR_IO:
                    fprintf(f, "%d, %d\n",
                            (int16_t)frame.fir_io_frame.fir_in_w,
                            (int16_t)frame.fir_io_frame.fir_out_w);
                    break;
                case PID_IO:
                    fprintf(f, "%d, %d, %u\n",
                            frame.pid_io_frame.pid_in,
                            frame.pid_io_frame.err,
                            frame.pid_io_frame.pid_out & 0x3FFF);
                    break;
                default:
                    fprintf(f, "%d, %d, %d, %d\n",
                            (int16_t)frame.adc_data_in_frame.adc_dat_a_16s,
                            (int16_t)frame.adc_data_in_frame.adc_dat_b_16s,
                            (int16_t)frame.adc_data_in_frame.i_feed_w,
                            (int16_t)frame.adc_data_in_frame.q_feed_w);
                    break;
            }
        }
        fclose(f);
    }

    ctx->output.output_items[4].data.u = return_code;
    ctx->output.output_items[4].tag    = UINT_TAG;
    strcpy(ctx->output.output_items[4].name, "return_code");
    ctx->output.num_outputs = 5;

    return return_code;
}
