#ifndef H_RUN
#define H_RUN

#include "ast.h"

typedef struct
    // view into virtual file memory.
    // may also point into ast literals.
{
    bool is_special;
    char literal;

    char*  ptr;
    size_t len;
} mem_view_t;

mem_view_t eval_expr(ast_expr_t* expr);
void run_store(ast_field_t* target, mem_view_t value);
void run_prog(ast_prog_t* root);

#endif





