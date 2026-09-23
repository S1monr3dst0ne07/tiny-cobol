
#include <stdio.h>
#include "lex.h"
#include "ast.h"
#include "parse.h"


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "No file path provided.\n");
        fprintf(stderr, "Usage: ./inter <path>\n");
        return 1;
    }

    char* path = argv[1];
    lex_t lex = lex_make(path);
    ast_prog_t* root = parse_prog(&lex);

    return 0;
}
