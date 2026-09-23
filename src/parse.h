#ifndef H_PARSE
#define H_PARSE

#include "ast.h"
#include "lex.h"

ast_prog_t* parse_prog(lex_t* stream);


#endif
