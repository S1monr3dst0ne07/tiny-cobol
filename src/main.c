
#include <stdio.h>
#include "lex.h"


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
    lex_t* stream = &lex;

    while (lex_has(stream))
    {
        printf("%s\n", lex_pop(stream));
    }


    return 0;
}
