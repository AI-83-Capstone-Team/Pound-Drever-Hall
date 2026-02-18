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

    if(dec > MAX_DEC) dec = MAX_DEC;
    if(alpha > MAX_ALPHA) alpha = MAX_ALPHA; 
    if(sat > MAX_SAT) alpha = MAX_SAT;

    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.set_kp_cmd.cmd = CMD_SET_KP;
    cmd.set_kp_cmd.kp = kp_i;
    pdh_callback_t cb = pdh_execute_cmd(cmd);
    int16_t echo_kp = (int16_t)cb.set_kp_cb.kp_r;
    float kp_converted = echo_kp / 32768.0f;
    int return_code = validate_cb(&echo_kp, &kp_i, INT16_TAG, __func__, "KP_CB", SET_PID_OK, SET_PID_INVALID_KP);
    push_ctx_cb(ctx, &index, &kp_converted, FLOAT_TAG, "KP_CB");

    cmd.raw = 0;
    cmd.set_kd_cmd.cmd = CMD_SET_KD;
    cmd.set_kd_cmd.kd = kd_i;
    cb = pdh_execute_cmd(cmd);
    int16_t echo_kd = (int16_t)cb.set_kd_cb.kd_r;
    float kd_converted = echo_kd / 32768.0f;
    return_code = validate_cb(&echo_kd, &kd_i, INT16_TAG, __func__, "KD_CB", return_code, SET_PID_INVALID_KD);
    push_ctx_cb(ctx, &index, &kd_converted, FLOAT_TAG, "KD_CB");

    cmd.raw = 0;
    cmd.set_ki_cmd.cmd = CMD_SET_KI;
    cmd.set_ki_cmd.ki = ki_i;
    cb = pdh_execute_cmd(cmd);
    int16_t echo_ki = (int16_t)cb.set_ki_cb.ki_r;
    float ki_converted = echo_ki / 32768.0f;
    return_code = validate_cb(&echo_ki, &ki_i, INT16_TAG, __func__, "KI_CB", return_code, SET_PID_INVALID_KI);
    push_ctx_cb(ctx, &index, &ki_converted, FLOAT_TAG, "KI_CB");

    cmd.raw = 0;
    cmd.set_dec_cmd.cmd = CMD_SET_DEC;
    cmd.set_dec_cmd.dec = dec;
    cb = pdh_execute_cmd(cmd);
    uint32_t echo_dec = cmd.set_dec_cmd.dec;
    return_code = validate_cb(&echo_dec, &dec, UINT_TAG, __func__, "DEC_CB", return_code, SET_PID_INVALID_DEC);
    push_ctx_cb(ctx, &index, &echo_dec, UINT_TAG, "DEC_CB");

    cmd.raw = 0;
    cmd.set_sp_cmd.cmd = CMD_SET_SP;
    cmd.set_sp_cmd.sp = sp_i;
    cb = pdh_execute_cmd(cmd);
    int16_t echo_sp = (int16_t)(((uint16_t)cb.set_sp_cb.sp_r) << 2)>>2;
    float sp_converted = echo_sp / 8192.0f;
    return_code = validate_cb(&echo_sp, &sp_i, INT16_TAG, __func__, "SP_CB", return_code, SET_PID_INVALID_SP);
    push_ctx_cb(ctx, &index, &sp_converted, FLOAT_TAG, "SP_CB");

    cmd.raw = 0;
    cmd.set_alpha_sat_en_cmd.cmd = CMD_SET_ALPHA_SAT_EN;
    cmd.set_alpha_sat_en_cmd.alpha = alpha;
    cmd.set_alpha_sat_en_cmd.sat = sat;
    cmd.set_alpha_sat_en_cmd.en = en;
    cb = pdh_execute_cmd(cmd);

    uint32_t echo_alpha = cb.set_alpha_sat_en_cb.alpha_r;
    uint32_t echo_sat = cb.set_alpha_sat_en_cb.sat_r;
    uint32_t echo_en = cb.set_alpha_sat_en_cb.en_r;

    return_code = validate_cb(&echo_alpha, &alpha, UINT_TAG, __func__, "ALPHA_CB", return_code, SET_PID_INVALID_ALPHA);
    return_code = validate_cb(&echo_sat, &sat, UINT_TAG, __func__, "SAT_CB", return_code, SET_PID_INVALID_SAT);
    return_code = validate_cb(&echo_en, &en, UINT_TAG, __func__, "EN_CB", return_code, SET_PID_INVALID_EN);

    push_ctx_cb(ctx, &index, &echo_alpha, UINT_TAG, "ALPHA_CB");
    push_ctx_cb(ctx, &index, &echo_sat, UINT_TAG, "SAT_CB");
    push_ctx_cb(ctx, &index, &echo_en, UINT_TAG, "EN_CB");

    return PDH_OK;
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
    if (theta_rad < -1.0f*M_PI) theta_rad = -1.0f*M_PI;

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
                case ANGLES_AND_ESIGS:
                    fprintf(f, "%f, %f, %d, %d\n", ((int16_t)frame.angles_and_esigs_frame.cos_theta_r)/ROTATION_CONST, ((int16_t)frame.angles_and_esigs_frame.sin_theta_r)/ROTATION_CONST, (int16_t)frame.angles_and_esigs_frame.i_feed_w, (int16_t)frame.angles_and_esigs_frame.q_feed_w); 
                    break;

                case PID_ERR_TAPS:
                    fprintf(f, "%d, %d, %d, %d\n", (int16_t)frame.pid_err_taps_frame.err_tap_w, (int16_t)frame.pid_err_taps_frame.perr_tap_w, (int16_t)frame.pid_err_taps_frame.derr_tap_w, (int16_t)frame.pid_err_taps_frame.ierr_tap_w);
                    break;

                case IO_SUM_ERR:
                    fprintf(f, "%d, %u, %d\n", (int16_t)frame.io_sum_err_frame.err_tap_w, (uint16_t)frame.io_sum_err_frame.pid_out_w, (int32_t)frame.io_sum_err_frame.sum_err_tap_w);
                    break;

                case GATE_CHECK:
                    fprintf(f, "%u, %u, %u, %u\n", (uint16_t)frame.gate_check_frame.dac1_gate_r, (uint16_t)frame.gate_check_frame.dac1_dat_r, (uint16_t)frame.gate_check_frame.dac2_gate_r, (uint16_t)frame.gate_check_frame.dac2_dat_r);
                    break;

                default:
                    fprintf(f, "%f, %f, %d, %d\n", ((int16_t)frame.angles_and_esigs_frame.cos_theta_r)/ROTATION_CONST, ((int16_t)frame.angles_and_esigs_frame.sin_theta_r)/ROTATION_CONST, frame.angles_and_esigs_frame.i_feed_w, frame.angles_and_esigs_frame.q_feed_w); 
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

    if(dac1_code > 1) return_code = CONFIG_IO_INVALID_DAC1;
    if(dac2_code > 1) return_code = CONFIG_IO_INVALID_DAC2;
    if(pid_code > 3) return_code = CONFIG_IO_INVALID_PID;

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
    uint32_t byte_offset = ctx->uint_args[0];
    
    dma_frame_t frame;
    frame.raw = dma_get_frame(byte_offset);

    ctx->output.output_items[0].data.i = (int16_t)frame.angles_and_esigs_frame.sin_theta_r;
    ctx->output.output_items[0].tag = INT_TAG;
    strcpy(ctx->output.output_items[0].name, "sin_theta_r");
    
    ctx->output.output_items[1].data.i = (int16_t)frame.angles_and_esigs_frame.cos_theta_r;
    ctx->output.output_items[1].tag = INT_TAG;
    strcpy(ctx->output.output_items[1].name, "cos_theta_r");

    ctx->output.output_items[2].data.i = (int16_t)frame.angles_and_esigs_frame.q_feed_w;
    ctx->output.output_items[2].tag = INT_TAG;
    strcpy(ctx->output.output_items[2].name, "q_feed_w");
    
    ctx->output.output_items[3].data.i = (int16_t)frame.angles_and_esigs_frame.i_feed_w;
    ctx->output.output_items[3].tag = INT_TAG;
    strcpy(ctx->output.output_items[3].name, "i_feed_w");

    ctx->output.num_outputs = 4;
    return PDH_OK;
}


static int validate_params(const lock_in_ctx_t *ctx)
{
	if (!ctx)
    {
		return NO_CONTEXT;
	}

    if (((ctx->dac_end - ctx->dac_start) / ctx->dac_step) <= 0)
	{
		return INVALID_STEP;
	}

	if (ctx->dac_start == ctx->dac_end)
	{
		return NO_RANGE;
	}

	return DAC_OK;
}

int lock_in(lock_in_ctx_t* ctx)
{
    DEBUG_INFO("Validating parameters\n");
	int ret = validate_params(ctx);
    if (ret != DAC_OK)
    {
		return ret;
	}
/*
	float curr_out = ctx->dac_start;

	uint32_t num_readings = (uint32_t)((ctx->dac_end - ctx->dac_start) / ctx->dac_step);
	if (num_readings > SWEEP_BUFFER_SIZE) return STEP_TOO_SMALL;

	DEBUG_INFO("Allocating: %d readings\n", num_readings);
	float* readings = (float*)malloc(num_readings * sizeof(float));
	
	DEBUG_INFO("Begin RF sweep: %f, %f, %f\n", ctx->dac_start, ctx->dac_end, ctx->dac_step);
	for(uint32_t index = 0; index < num_readings; index++)
	{
		rf_write(ctx->chout, curr_out > 0? RP_WAVEFORM_DC : RP_WAVEFORM_DC_NEG, ABS(curr_out), 0, 0, true);
		usleep(WR_DELAY_US);
		rf_read(ctx->chin, 1);	
	
		readings[index] = gAdcMirror[0];
		curr_out += ctx->dac_step;
	}
	
	uint32_t hi = num_readings-1;
	uint32_t lo = 0;
	float slope = 0;
	while(hi > lo)
	{
		slope += (readings[hi] - readings[lo]) / (hi - lo);
		hi--;
		lo++;
	}
	slope /= (num_readings / 2);

	float best_in = readings[0];
	float best_out = ctx->dac_start;

	for(uint32_t index = 1; index < num_readings; index++)
	{
		float curr_in = readings[index] - (index * slope);
		if (curr_in < best_in)
		{
			best_in = curr_in;
			best_out = ctx->dac_start + (index * ctx->dac_step);
		}
	}

	if (ctx->log_data)
	{
		DEBUG_INFO("Logging sweep data...\n");
		FILE* f = fopen("lockin_log.csv", "w");
		if (!f) return CANNOT_LOG;
		for(uint32_t index = 0; index < num_readings; index++)
		{
			float normalized;
			if (ctx->dac_step > 0) normalized = readings[index] - index*slope + best_out;
			else normalized = readings[index] - index*slope - best_out;
			
			fprintf(f, "%f, %f, %f\n", ctx->dac_start + index*ctx->dac_step, readings[index], normalized);
		}
		fclose(f);
	}

	else
	{
		for(uint32_t index = 0; index < num_readings; index++)
		{
			float normalized;
			if (ctx->dac_step > 0) normalized = readings[index] - index*slope + best_out;
			else normalized = readings[index] - index*slope - best_out;
			sweep_entry_t entry = {(ctx->dac_start+index*ctx->dac_step), (readings[index]), normalized};
			gSweepBuff[index] = entry;
		}
		ctx->num_readings = num_readings;
	}

	free(readings);

	ctx->lock_point = best_out;

	ctx->derived_slope = slope;
	if (ctx->dac_step < 0) ctx->derived_slope *= -1;

	rf_write(ctx->chout, best_out > 0? RP_WAVEFORM_DC : RP_WAVEFORM_DC_NEG, ABS(best_out), 0, 0, true);
*/
	return LOCKED_IN;
}

int cmd_lock_in(cmd_ctx_t* ctx)
{
	DEBUG_INFO("Loading command context...\n");
	lock_in_ctx_t lock_ctx = {
		false,
		(uint32_t)ctx->uint_args[0], 	//chin
		(uint32_t)ctx->uint_args[1],	//chout
		ctx->float_args[0],	//dac_end
		ctx->float_args[1], //dac_start
		ctx->float_args[2],	//dac_step
		0.0,				//lock_point
		0.0					//derived_slope
	};

	DEBUG_INFO("Start lock_in...\n");

	int return_code = lock_in(&lock_ctx);
	ctx->output.output_items[0].data.i = return_code;
	ctx->output.output_items[0].tag = INT_TAG;
	strcpy(ctx->output.output_items[0].name, RETURN_STATUS_FLAG);

	ctx->output.output_items[1].data.f = lock_ctx.lock_point;
	ctx->output.output_items[1].tag = FLOAT_TAG;
	strcpy(ctx->output.output_items[1].name, LOCK_POINT_FLAG);

	ctx->output.output_items[2].data.f = lock_ctx.derived_slope;
	ctx->output.output_items[2].tag = FLOAT_TAG;
	strcpy(ctx->output.output_items[2].name, DERIVED_SLOPE_FLAG);

	ctx->output.num_outputs = 3;
	ctx->sweep_count = lock_ctx.num_readings;
	return return_code;
}






