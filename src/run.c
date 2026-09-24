
#include "common.h"
#include "run.h"
#include "ast.h"

void run_proc(ast_proc_t* node);

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

uint64_t load_expr(ast_expr_t* expr)
{
    mem_view_t string = eval_expr(expr);
    uint64_t value = 0;

    for (int i = 0; i < string.len; i++)
    if (isdigit(string.ptr[i]))
        value = (value * 10) + (string.ptr[i] - '0');

    return value;
}

void run_display(struct ast_display_s* node)
{
    FILE* stream = stdout;

    mem_view_t res = eval_expr(node->target);
    fwrite(res.ptr, res.len, sizeof(char), stream);
    fputs("\n", stream);
}

void run_perform_times(struct ast_perform_times_s* node)
{
    uint64_t times = load_expr(node->times);
    for (uint64_t i = 0; i < times; i++)
        run_proc(node->ref->ref);

}
void run_move(struct ast_op_s* node)
{
    mem_view_t   src = eval_expr(node->left);
    ast_field_t* dst = node->right->ref;
    if (dst == NULL) lex_error(
        node->right->info, 
        "Move into non-field expression\n"
    );

    memcpy(
        dst->base,
        src.ptr,
        src.len
    );
}

void run_op(ast_stmt_t* node)
{
    uint64_t left  = load_expr(node->content.op.left);
    uint64_t right = load_expr(node->content.op.right);
    uint64_t res = 0;

    switch(node->kind)
    {
        case AST_STMT_KIND_ADD: res = left + right; break;
        case AST_STMT_KIND_SUB: res = left - right; break;
        case AST_STMT_KIND_MUL: res = left * right; break;
        case AST_STMT_KIND_DIV: res = left / right; break;
    }

    ast_expr_t* target = node->content.op.target;
    ast_field_t* dst = target->ref;
    if (dst == NULL) lex_error(
        target->info, 
        "Operate into non-field expression\n"
    );

    // HYPER scuffed lol, should work tho
    sprintf(dst->base, "%.*d", dst->outer_size, res);
}

void run_stmt(ast_stmt_t* node)
{
    switch (node->kind)
    {
        case AST_STMT_KIND_DISPLAY:
            run_display(&node->content.display);
            break;
        case AST_STMT_KIND_PERFORM_TIMES:
            run_perform_times(&node->content.perform_times);
            break;
        case AST_STMT_KIND_MOVE:
            run_move(&node->content.op);
            break;

        case AST_STMT_KIND_ADD:
        case AST_STMT_KIND_SUB:
        case AST_STMT_KIND_MUL:
        case AST_STMT_KIND_DIV:
            run_op(node);
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





