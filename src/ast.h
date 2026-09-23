#ifndef H_AST
#define H_AST

#include "common.h"

typedef struct ast_field_s
{
    uint8_t level;
    char*   name;

    char* picture;
    char* value;

    uint64_t occurs;

    struct ast_field_s* child;
    struct ast_field_s* next;

    void* base_address;

    // outer_size = inner_size * occur
    uint64_t inner_size;
    uint64_t outer_size;
} ast_field_t;



typedef struct
{
    ast_field_t* data;
} ast_prog_t;



#endif
