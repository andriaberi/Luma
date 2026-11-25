#include "lexer/dump.h"

#include <stdio.h>
#include <string.h>

#include "lexer/lexer.h"
#include "lexer/token.h"

static int digits(int n) {
    int d = 1;
    for (; n >= 10; n /= 10) d++;
    return d;
}

void dump_tokens(const Source *src) {
    // Size each column to this file: line and col by digit count, kind by the longest name used.
    int lines = 1, col = 1, max_col = 1;
    for (size_t i = 0; i < src->len; i++) {
        if (src->text[i] == '\n') {
            lines++;
            col = 1;
        } else if (++col > max_col) {
            max_col = col;
        }
    }

    int kind_w = 0;
    Lexer lex = lexer_init(src);
    Token tok;
    do {
        tok = lexer_next(&lex);
        int n = (int) strlen(token_name(tok.kind));
        if (n > kind_w) kind_w = n;
    } while (tok.kind != TOK_EOF);

    int line_w = digits(lines), col_w = digits(max_col);

    lex = lexer_init(src);
    do {
        tok = lexer_next(&lex);
        printf("%*d:%-*d ", line_w, tok.line, col_w, tok.col);
        if (tok.len == 0) {
            printf("%s\n", token_name(tok.kind));  // EOF has no lexeme, so skip the padding
        } else {
            printf("%-*s %.*s\n", kind_w, token_name(tok.kind), tok.len, tok.start);
        }
    } while (tok.kind != TOK_EOF);
}
