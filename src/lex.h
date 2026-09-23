#ifndef H_LEX 
#define H_LEX

#include "common.h"

typedef enum
{
    LEX_STATE_INVALID,
    LEX_STATE_WORD,
    LEX_STATE_DOT,
    LEX_STATE_QUOTE,
    LEX_STATE_FORMAT,
    LEX_STATE_SYMBOL,
    LEX_STATE_TERMINATOR,
} lex_state_t;

typedef struct
{
    char* path;
    char* src;
    size_t src_idx;
    size_t src_len;
    size_t line_no;
} lex_t;


lex_t lex_make(char* path);
char* lex_peek  (lex_t* stream);
char* lex_pop   (lex_t* stream);
bool  lex_has   (lex_t* stream);
void  lex_expect(lex_t* stream, const char* word);

#endif
