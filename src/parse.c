
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


void parse_data_div(lex_t* stream, ast_prog_t* node)
{
    lex_expect(stream, "data");
    lex_expect(stream, "division");
    lex_expect(stream, ".");

    char* token = lex_peek(stream);
    if (!strcmp(token, "working-storage")) parse_ws(stream, node);
}

ast_prog_t* parse_prog(lex_t* stream)
{
    ast_prog_t* node = malloc(sizeof(ast_prog_t));
    parse_data_div(stream, node);

    return node;
}
