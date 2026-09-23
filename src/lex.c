
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
        case '\0':
        case ' ' : 
        case '\n': return LEX_STATE_FORMAT;
        default  : return LEX_STATE_SYMBOL;
    }
}

bool lex_has(lex_t* stream)
{
    return stream->src_idx < stream->src_len;
}

#define CHAR (stream->src[stream->src_idx])

void skip_format(lex_t* stream)
{
    while (get(CHAR) == LEX_STATE_FORMAT)
        stream->src_idx++;
}

char* lex_pop(lex_t* stream)
{
    static char buffer[1 << 16];
    char* iter = buffer;

    skip_format(stream);

    bool in_string = false;
    lex_state_t state = get(CHAR);
    for (;lex_has(stream);)
    {
        lex_state_t kind = get(CHAR);

        if (CHAR == '\n') stream->line_no++;

        if (state != kind) if (!in_string)
            goto done;

        if (kind == LEX_STATE_QUOTE) in_string ^= true;
        else

        // spooky action at a distance
        *iter++ = CHAR;
        
        stream->src_idx++;
        state = kind;
    }

done:
    skip_format(stream);

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


