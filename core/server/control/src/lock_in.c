#include "lock_in.h"



void lock_in(vLockFsm_t* vFsm)
{
    switch(vFsm->state)
    {
        case LOCKIN_INIT:
            get_voltage(vFsm);
            vFsm->best_voltage = vFsm->current_voltage;
            vFsm->current_code = DAC_INIT;
            vFsm->best_code = DAC_INIT;
            vFsm->state = LOCKIN_INC;
            //fallthrough


        case LOCKIN_INC:
            if(vFsm->current_code < DAC_HIGH)
            {
                write_code(vFsm);
                get_voltage(vFsm);

                if(vFsm->current_voltage < vFsm->best_voltage)
                {
                    vFsm->best_voltage = vFsm->current_voltage;
                    vFsm->best_code = vFsm->current_code;
                }

                vFsm->current_code += DAC_STEP;
                break;
            }

            else
            {
                vFsm->current_code = DAC_INIT - DAC_STEP;
                vFsm->state = LOCKIN_DEC;
            }
            //fallthrough
        

        case LOCKIN_DEC:
            if(vFsm->current_code > DAC_LOW)
            {
                write_code(vFsm);
                get_voltage(vFsm);

                if(vFsm->current_voltage < vFsm->best_voltage)
                {
                    vFsm->best_voltage = vFsm->current_voltage;
                    vFsm->best_code = vFsm->current_code;
                }

                vFsm->current_code -= DAC_STEP;
                break;
            }

            else
            {
                vFsm->current_code = DAC_INIT;
                vFsm->state = LOCKIN_TERMINATE;
            }
            //fallthrough
        

        case LOCKIN_TERMINATE:
            vFsm->current_code = vFsm->best_code;
            write_code(vFsm);
            vFsm->state = LOCKIN_INIT;
            break;


        default:
            break;
    }

    return;
}



//TODO: Implement these
static inline void get_voltage(vLockFsm_t* vFsm)
{
    return;
}

static inline void write_code(vLockFsm_t* vFsm)
{
    return;
}