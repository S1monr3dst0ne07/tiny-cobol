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

    void* base;

    // outer_size = inner_size * occur
    uint64_t inner_size;
    uint64_t outer_size;
} ast_field_t;



typedef struct 
{
    char* content;
    ast_field_t* ref; // NULL for literal
}ast_expr_t;

typedef enum
{
    AST_STMT_KIND_PERFORM_TIMES,
    AST_STMT_KIND_DISPLAY,
    AST_STMT_KIND_MOVE, // target = left
    AST_STMT_KIND_ADD,  // target = left + right
    AST_STMT_KIND_SUB,  // target = left - right
    AST_STMT_KIND_MUL,  // target = left * right
    AST_STMT_KIND_DIV,  // target = left / right
} ast_stmt_kind_t;

typedef struct ast_stmt_s
{
    ast_stmt_kind_t kind;
    struct ast_stmt_s* next;
    union
    {
        struct ast_op_s
        {
            ast_expr_t* left;
            ast_expr_t* right;
            ast_expr_t* target;
        } op;
        struct ast_perform_times_s
        {
            char* proc_name;
            ast_expr_t* times;
        } perform_times;
        struct ast_display_s
        {
            ast_expr_t* target;
        } display;
    } content;
} ast_stmt_t;

typedef struct ast_proc_s
{
    char* name;
    ast_stmt_t* stmt;
    struct ast_proc_s* next;
} ast_proc_t;


typedef struct
{
    ast_field_t* data;
    ast_proc_t*  proc;
    
    // virtual file allocated by `pre_prog`.
    // all COBOL data is stored in a big big nd-table.
    char* mem;
} ast_prog_t;


#endif
