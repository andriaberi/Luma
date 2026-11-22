#ifndef LUMA_LEXER_H
#define LUMA_LEXER_H

#include "lexer/token.h"
#include "source/file.h"

typedef struct {
    const Source *src;
    const char *p;           // next character to read
    const char *line_start;  // col = p - line_start + 1
    int line;
} Lexer;

Lexer lexer_init(const Source *src);

// Next token, or TOK_EOF at the end (and on every call after that).
// On a lexical error, reports it with error_at() and exits 1.
Token lexer_next(Lexer *lex);

#endif
