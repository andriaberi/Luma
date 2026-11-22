#include "lexer/lexer.h"

#include "source/diag.h"

Lexer lexer_init(const Source *src) {
    // TODO
    (void) src;
    return (Lexer) { 0 };
}

Token lexer_next(Lexer *lex) {
    // TODO
    (void) lex;
    return (Token) { .kind = TOK_EOF };
}
