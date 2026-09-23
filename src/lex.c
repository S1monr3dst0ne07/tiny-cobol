
#include "lex.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


lex_t lex_make(char* path)
{
    FILE* fd = fopen(path, "r");

    fseek(fd, 0, SEEK_END);
    size_t len = ftell(fd) + 1;
    fseek(fd, 0, SEEK_SET);

    char* buffer = malloc(len);
    fread(buffer, len, sizeof(char), fd);
    buffer[len-1] = '\0';

    fclose(fd);


    return (lex_t) {
        .path = path,
        .src = buffer,
        .src_idx = 0,
        .src_len = len,
        .line_no = 1,
    };
}


lex_state_t get(char c)
{
    if (isalnum(c)) return LEX_STATE_WORD;
    switch(c)
    {
        case '-' :
        case '(' :
        case ')' : return LEX_STATE_WORD;
        case '.' : return LEX_STATE_DOT;
        case '"' : return LEX_STATE_QUOTE;
        case ' ' : 
        case '\n': return LEX_STATE_FORMAT;
        case '\0': return LEX_STATE_TERMINATOR;
        default  : return LEX_STATE_SYMBOL;
    }
}

bool lex_has(lex_t* stream)
{
    return stream->src_idx < stream->src_len;
}

char* lex_pop(lex_t* stream)
{
    static char buffer[1 << 16];
    char* iter = buffer;
    lex_state_t state = LEX_STATE_INVALID;

    for (;lex_has(stream);)
    {
        char c = stream->src[stream->src_idx];
        lex_state_t kind = get(c);

        if (c == '\n') stream->line_no++;
        //if (state == LEX_STATE_QUOTE) stream->in_string ^= true;

        if (state != LEX_STATE_INVALID)
        if (state != kind)// && !stream->in_string)
        {
            if (state != LEX_STATE_FORMAT)
                goto done;
            iter = buffer;
        }

        //if (state != LEX_STATE_QUOTE)
        *iter++ = c;
        
        stream->src_idx++;
        state = kind;
    }

done:
    *iter = '\0';
    return buffer;
}

char* lex_peek(lex_t* stream)
{
    size_t src_idx = stream->src_idx;
    char* ptr = lex_pop(stream);
    stream->src_idx = src_idx;
    return ptr;
}

void lex_expect(lex_t* stream, const char* word)
{
    char* token = lex_pop(stream);

    if (strcmp(word, token))
    {
        fprintf(
            stderr, 
            "Error on line %d in file %s: Expected `%s` but got `%s`\n",
            stream->line_no, stream->path, word, token
        );
        exit(1);
    }
}


