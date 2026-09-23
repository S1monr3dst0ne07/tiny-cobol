
#define ATOM_FIELD_LEVEL 77

#include "common.h"
#include "lex.h"
#include "ast.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


char* parse_picture(lex_t* stream)
{
    char* content = lex_pop(stream);

    static char buffer[4096];
    char* iter = buffer;

    bool bracket = false;
    int count = 0;

    for (size_t i = 0; content[i]; i++)
    {
        char c = content[i];
        /**/ if (c == '(') bracket = true;
        else if (c == ')')
        {
            char inner = *(--iter);
            for (int i = 0; i < count; i++)
                *iter++ = inner;
            bracket = false;
        }

        else if (bracket) count = (count * 10) + (c - '0');
        else *iter++ = c;
    }

    *iter = '\0';
    return strdup(buffer);
}


ast_field_t* parse_field(lex_t* stream)
{
    ast_field_t* node = malloc(sizeof(ast_field_t));
    node->level = atoi  (lex_pop(stream));
    node->name  = strdup(lex_pop(stream));

    while (lex_peek(stream)[0] != '.')
    {
        char* token = lex_pop(stream);
        /**/ if (!strcmp(token, "occurs")) {
            node->occurs = atoi(lex_pop(stream));
            lex_expect(stream, "times");
        } 
        else if (!strcmp(token, "pic"))
            node->picture = parse_picture(stream);
        else if (!strcmp(token, "value"))
            node->value   = strdup(lex_pop(stream));
    }
    lex_expect(stream, ".");

    uint8_t lvl = atoi(lex_peek(stream));

    //if (node->level != ATOM_FIELD_LEVEL)
    node->child = (lvl >  node->level) ? parse_field(stream) : NULL;
    node->next  = (lvl == node->level) ? parse_field(stream) : NULL;

    return node;
}

void parse_ws(lex_t* stream, ast_prog_t* node)
{
    lex_expect(stream, "working-storage");
    lex_expect(stream, "section");
    lex_expect(stream, ".");
    node->data = parse_field(stream);
}





ast_expr_t* parse_expr(lex_t* stream)
{
    lex_pop(stream);
}

ast_stmt_t* parse_perform(lex_t* stream)
{
    ast_stmt_t* node = malloc(sizeof(ast_stmt_t));
    lex_expect(stream, "perform");
    char* name = strdup(lex_pop(stream));

    char* word = lex_peek(stream);
    if (!strcmp(word, "exactly")) {
        lex_expect(stream, "exactly");
        node->kind = AST_STMT_KIND_PERFORM_TIMES;
        node->content.perform_times.proc_name = name;
        node->content.perform_times.times     = parse_expr(stream);
        lex_expect(stream, "times");
    }
    lex_expect(stream, ".");

    return node;
}

ast_stmt_t* parse_display(lex_t* stream)
{
    ast_stmt_t* node = malloc(sizeof(ast_stmt_t));
    node->kind = AST_STMT_KIND_DISPLAY;
    lex_expect(stream, "display");
    node->content.display.target = parse_expr(stream);
    lex_expect(stream, ".");
    return node;
}

ast_stmt_t* parse_op(lex_t* stream)
{
    ast_stmt_t* node = malloc(sizeof(ast_stmt_t));
    char* verb = lex_pop(stream);

    /**/ if (!strcmp(verb, "add"))  node->kind = AST_STMT_KIND_ADD;
    else if (!strcmp(verb, "move")) node->kind = AST_STMT_KIND_MOVE;

    node->content.op.left  = parse_expr(stream);
    lex_pop(stream);
    node->content.op.right = parse_expr(stream);

    node->content.op.target = NULL;
    if (!strcmp(lex_peek(stream), "giving"))
    {
        lex_pop(stream);
        node->content.op.target = parse_expr(stream);
    }
    lex_expect(stream, ".");

    return node;
}

ast_stmt_t* parse_stmts(lex_t* stream)
{
    ast_stmt_t hook;
    ast_stmt_t* iter = &hook;

    while (true)
    {
        ast_stmt_t* new;
        char* token = lex_peek(stream);
        /**/ if (!strcmp(token, "perform")) new = parse_perform(stream);
        else if (!strcmp(token, "display")) new = parse_display(stream);
        else if (!strcmp(token, "add")    ) new = parse_op(stream);
        else if (!strcmp(token, "move")   ) new = parse_op(stream);
        else break;

        new->next  = NULL;
        iter->next = new;
        iter = new;
    }

    return hook.next;
}

ast_proc_t* parse_proc(lex_t* stream)
{
    ast_proc_t* node = malloc(sizeof(ast_proc_t));
    node->name = strdup(lex_pop(stream));
    lex_expect(stream, ".");
    node->stmt = parse_stmts(stream);
    node->next = lex_has(stream) ? parse_proc(stream) : NULL;

    return node;
}


void parse_data_div(lex_t* stream, ast_prog_t* node)
{
    lex_expect(stream, "data");
    lex_expect(stream, "division");
    lex_expect(stream, ".");

    char* token = lex_peek(stream);
    if (!strcmp(token, "working-storage")) parse_ws(stream, node);
}

void parse_proc_div(lex_t* stream, ast_prog_t* node)
{
    lex_expect(stream, "procedure");
    lex_expect(stream, "division");
    lex_expect(stream, ".");
    node->proc = parse_proc(stream);
}

ast_prog_t* parse_prog(lex_t* stream)
{
    ast_prog_t* node = malloc(sizeof(ast_prog_t));
    parse_data_div(stream, node);
    parse_proc_div(stream, node);

    return node;
}
