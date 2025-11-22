#include <stdint.h>

enum
{
    DAC_INIT = 0,
    DAC_HIGH = 100, //DUMMY VALUES! 
    DAC_LOW = -100,
    DAC_STEP = 1,
} gLockDefaults;


typedef enum 
{
    LOCKIN_INIT,
    LOCKIN_INC, 
    LOCKIN_DEC,
    LOCKIN_TERMINATE,
} gLockState_e;

typedef struct
{
    gLockState_e state; //uint32_t
    int32_t current_code;
    int32_t best_code;
    uint32_t current_voltage; 
    uint32_t best_voltage;
} vLockFsm_t;


void lock_in(vLockFsm_t* vFsm);
static inline void get_voltage(vLockFsm_t* vFsm);
static inline void write_code(vLockFsm_t* vFsm);