#include <stdint.h>
#include <stdbool.h>



typedef enum
{
    LOAD_VALUE_OK,
    LOAD_VALUE_NO_KEY,
    LOAD_VALUE_NO_VAL,
    LOAD_VALUE_MALFORMED_VAL,
    LOAD_VALUE_EMPTY_VAL,
    LOAD_VALUE_OVERSIZED,
}   load_value_e;

typedef enum
{
    LOAD_CTX_OK,
    LOAD_CTX_LOAD_CMD_FAIL,
    LOAD_CTX_LOAD_ARGS_FAIL,
}   load_ctx_e;



typedef enum
{
    COMMAND_SIZE = 32,
    FLOAT_ARGS = 10,
    INT_ARGS = 10,
    UINT_ARGS = 10
}   cmd_dims_e;


typedef stuct
{
    char command_str[COMMAND_SIZE];
    float float_args[FLOAT_ARGS];
    int32_t int_args[INT_ARGS];
    uint32_t uint_args[UINT_ARGS];

    uint8_t num_floats;
    uint8_t num_ints;
    uint8_t num_uints;

    load_value_e float_status;
    load_value_e int_status;
    load_value_e uint_status;

    bool success;
}   cmd_ctx_t;



int load_context(const char* json, cmd_ctx_t* ctx) ;