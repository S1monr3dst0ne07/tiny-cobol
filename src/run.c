
#include "run.h"
#include "ast.h"

#include <string.h>
#include <stdio.h>

typedef struct
    // view into virtual file memory.
    // may also point into ast literals.
{
    char*  ptr;
    size_t len;
} mem_view_t;



mem_view_t eval_expr(ast_expr_t* expr)
{
    if (expr->ref) return (mem_view_t) {
        .ptr = expr->ref->base,
        .len = expr->ref->outer_size,
    };

    // for literals point into ast buffer.
    return (mem_view_t) {
        .ptr = expr->content,
        // TODO: fix this, this is slow!
        .len = strlen(expr->content),
    };
}

void run_display(struct ast_display_s* node)
{
    mem_view_t res = eval_expr(node->target);
    printf("%.*s\n", res.len, res.ptr);
}

void run_stmt(ast_stmt_t* node)
{
    switch (node->kind)
    {
        case AST_STMT_KIND_DISPLAY:
            run_display(&node->content.display);
            break;
        case AST_STMT_KIND_PERFORM_TIMES:
            //run_perform_times(&node->content.perform_times);
            break;
        case AST_STMT_KIND_MOVE:
        case AST_STMT_KIND_ADD:
        case AST_STMT_KIND_SUB:
        case AST_STMT_KIND_MUL:
        case AST_STMT_KIND_DIV:
            //run_op(&node->content.op);
            break;
    }
}

void run_proc(ast_proc_t* node)
{
    ast_stmt_t* iter = node->stmt;
    while (iter)
    {
        run_stmt(iter);
        iter = iter->next;
    }
}

void run_prog(ast_prog_t* root)
{
    ast_proc_t* first = root->proc;
    run_proc(first);
}





