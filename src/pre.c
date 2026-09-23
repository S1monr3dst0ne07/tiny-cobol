
// precomputation pass.
// - compute data layout in virtual file.  
// - result expression references to fields.

#include "pre.h"
#include "common.h"
#include "ast.h"


uint64_t pre_field_sizes(ast_field_t* field)
    // precompute field content sizes.
    // returns total size of field collection.
{
    field->inner_size = 0;
    if (field->picture) field->inner_size += strlen(field->picture);
    if (field->child  ) field->inner_size += pre_field_sizes(field->child);

    field->outer_size = field->inner_size * field->occurs;

    return field->outer_size + 
        ((field->next) ? pre_field_sizes(field->next) : 0);
}

void pre_field_address(char* base, ast_field_t* field)
{
    field->base = base;
    if (field->child) pre_field_address(base,                     field->child);
    if (field->next ) pre_field_address(base + field->outer_size, field->next);
}

void pre_field_init(ast_field_t* field)
{
    if (field->value)
        memcpy(field->base, field->value, strlen(field->value));

    if (field->child) pre_field_init(field->child);
    if (field->next ) pre_field_init(field->next);
}


void pre_expr(ast_prog_t* root, ast_expr_t* node)
{
    if (node == NULL) return;

    ast_field_t* iter = root->data;
    for (; iter; iter = iter->next)
        if (!strcmp(iter->name, node->content))
            node->ref = iter;
}

void pre_proc_ref(ast_prog_t* root, ast_proc_ref_t* node)
{
    if (node == NULL) return;

    ast_proc_t* iter = root->proc;
    for (; iter; iter = iter->next)
        if (!strcmp(iter->name, node->name))
            node->ref = iter;
}

void pre_stmt(ast_prog_t* root, ast_stmt_t* node)
{
    switch (node->kind)
    {
        case AST_STMT_KIND_MOVE:
        case AST_STMT_KIND_ADD:
        case AST_STMT_KIND_SUB:
        case AST_STMT_KIND_MUL:
        case AST_STMT_KIND_DIV:
            pre_expr(root, node->content.op.left);
            pre_expr(root, node->content.op.right);
            pre_expr(root, node->content.op.target);
            break;

        case AST_STMT_KIND_PERFORM_TIMES:
            pre_proc_ref(root, node->content.perform_times.ref);
            pre_expr    (root, node->content.perform_times.times);
            break;

        case AST_STMT_KIND_DISPLAY:
            pre_expr(root, node->content.display.target);
            break;
    }

    if (node->next) pre_stmt(root, node->next);
}

void pre_proc(ast_prog_t* root, ast_proc_t* proc)
{
    pre_stmt(root, proc->stmt);
    if (proc->next) pre_proc(root, proc->next);
}

void pre_prog(ast_prog_t* root)
{
    if (root->data)
    {
        uint64_t size = pre_field_sizes(root->data);
        root->mem = malloc(size); // virtual file.
    
        pre_field_address(root->mem, root->data);
        pre_field_init(root->data); // initial by `value` parameter
    }

    pre_proc(root, root->proc);
}



