#pragma once

#include <stdint.h>
#include <stdbool.h>

#define NAME_SIZE 24
#define RETURN_STATUS_FLAG "return_status"

typedef enum
{
    FLOAT_TAG,
    INT_TAG,
    UINT_TAG,
}   output_tags_e;


typedef enum
{
    LOAD_CTX_OK,
    LOAD_CTX_NO_STRING,
    LOAD_CTX_LOAD_CMD_FAIL,
    LOAD_CTX_NO_KEY,
}   load_ctx_codes_e;


typedef enum
{
    DISPATCH_CMD_OK,
    DISPATCH_CMD_NO_CMD,
    DISPATCH_CMD_FLOAT_ARG_MISMATCH,
    DISPATCH_CMD_INT_ARG_MISMATCH,
    DISPATCH_CMD_UINT_ARG_MISMATCH
}   dispatch_cmd_codes_e;


typedef enum
{
    COMMAND_SIZE = 32,
    FLOAT_ARGS = 10,
    INT_ARGS = 10,
    UINT_ARGS = 10,
    OUTPUT_ITEMS
}   cmd_dims_e;


typedef struct{
    union
    {
        float f;
        int32_t i;
        uint32_t u;
    } data;
    uint8_t tag;
    char name[NAME_SIZE];
}   output_item_t;


typedef struct
{
    char name[COMMAND_SIZE];
    float float_args[FLOAT_ARGS];
    int32_t int_args[INT_ARGS];
    uint32_t uint_args[UINT_ARGS];

    uint8_t num_floats;
    uint8_t num_ints;
    uint8_t num_uints;

    load_ctx_codes_e float_status;
    load_ctx_codes_e int_status;
    load_ctx_codes_e uint_status;

    struct
    {
        output_item_t output_items[OUTPUT_ITEMS];
        uint16_t num_outputs;   
    } output;

}   cmd_ctx_t;



typedef int (*cmd_handler_t)(cmd_ctx_t* ctx);

typedef struct
{
    const char* name;
    cmd_handler_t func;
    uint8_t required_floats;
    uint8_t required_ints;
    uint8_t required_uints;
} cmd_entry_t;


int load_context(const char* json, cmd_ctx_t* ctx);
int dispatch_cmd(cmd_ctx_t* ctx);