
#include "common.h"
#include "lex.h"
#include <stdarg.h>

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
        .info = (lex_info_t) {
            .path = path,
            .line_no = 1,
        },
        .src = buffer,
        .src_idx = 0,
        .src_len = len,
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

        if (CHAR == '\n') stream->info.line_no++;

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

void lex_error(lex_info_t info, const char* pattern, ...)
{
    va_list args;
    va_start(args, pattern);

    char buffer[4096];
    vsprintf(buffer, pattern, args);

    fprintf(
        stderr, 
        "Error on line %d in file %s: %s", 
        info.line_no, 
        info.path, 
        buffer
    );
    exit(1);
}

void lex_expect(lex_t* stream, const char* word)
{
    char* token = lex_pop(stream);

    if (strcmp(word, token))
        lex_error(
            stream->info, 
            "Expected `%s` but got `%s`\n",
            word, 
            token
        );
}


lex_info_t lex_copy_info(lex_t* stream)
{
    return (lex_info_t) {
        .path    = strdup(stream->info.path),
        .line_no =        stream->info.line_no
    };
}

