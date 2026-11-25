#include "lexer/lexer.h"

#include <stdbool.h>
#include <string.h>

#include "source/diag.h"

static bool is_digit(char c) { return c >= '0' && c <= '9'; }
static bool is_hex_digit(char c) { return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
static bool is_ident_start(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
static bool is_ident_char(char c) { return is_ident_start(c) || is_digit(c); }

static bool at_end(const Lexer *lex) {
    return lex->p >= lex->src->text + lex->src->len;
}

// At the end this is the '\0' that source_read() appends, so character-class loops stop there.
static char peek(const Lexer *lex) {
    return *lex->p;
}

static char peek_next(const Lexer *lex) {
    return at_end(lex) ? '\0' : lex->p[1];
}

static char advance(Lexer *lex) {
    char c = *lex->p++;
    if (c == '\n') {
        lex->line++;
        lex->line_start = lex->p;
    }
    return c;
}

static bool match(Lexer *lex, char c) {
    if (at_end(lex) || peek(lex) != c) return false;
    advance(lex);
    return true;
}

static int current_col(const Lexer *lex) {
    return (int) (lex->p - lex->line_start) + 1;
}

static Token make_token(const Lexer *lex, TokenType kind, const char *start, int line, int col) {
    return (Token) { .kind = kind, .start = start, .len = (int) (lex->p - start), .line = line, .col = col };
}

static TokenType ident_kind(const char *s, int len) {
    static const struct { const char *word; TokenType kind; } keywords[] = {
        { "int", TOK_INT },         { "char", TOK_CHAR },         { "bool", TOK_BOOL },
        { "const", TOK_CONST },     { "struct", TOK_STRUCT },     { "enum", TOK_ENUM },
        { "fn", TOK_FN },           { "return", TOK_RETURN },
        { "when", TOK_WHEN },       { "loop", TOK_LOOP },
        { "break", TOK_BREAK },     { "continue", TOK_CONTINUE },
        { "import", TOK_IMPORT },   { "export", TOK_EXPORT },
        { "new", TOK_NEW },         { "delete", TOK_DELETE },
        { "in", TOK_IN },           { "out", TOK_OUT },           { "err", TOK_ERR },
        { "true", TOK_TRUE },       { "false", TOK_FALSE },       { "null", TOK_NULL },
        { "_", TOK_UNDERSCORE },
    };
    for (size_t i = 0; i < sizeof keywords / sizeof keywords[0]; i++) {
        if (strlen(keywords[i].word) == (size_t) len && memcmp(keywords[i].word, s, (size_t) len) == 0) {
            return keywords[i].kind;
        }
    }
    return TOK_IDENT;
}

static bool is_reserved(const char *s, int len) {
    return (len == 6 && memcmp(s, "extern", 6) == 0) || (len == 5 && memcmp(s, "defer", 5) == 0);
}

static void skip_whitespace(Lexer *lex) {
    while (!at_end(lex)) {
        char c = peek(lex);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance(lex);
        } else if (c == '/' && peek_next(lex) == '/') {
            while (!at_end(lex) && peek(lex) != '\n') advance(lex);
        } else if (c == '/' && peek_next(lex) == '*') {
            int line = lex->line, col = current_col(lex);
            advance(lex);
            advance(lex);
            // Block comments don't nest: the first "*/" ends it.
            while (!(peek(lex) == '*' && peek_next(lex) == '/')) {
                if (at_end(lex)) error_at(lex->src, line, col, "unterminated comment");
                advance(lex);
            }
            advance(lex);
            advance(lex);
        } else {
            return;
        }
    }
}

static void escape(Lexer *lex) {
    int line = lex->line, col = current_col(lex);
    advance(lex);
    char c = peek(lex);
    if (c == 'x') {
        // p[2] is safe: p[1] was a hex digit, so at worst p[2] is the trailing '\0'.
        if (!is_hex_digit(peek_next(lex)) || !is_hex_digit(lex->p[2])) {
            error_at(lex->src, line, col, "'\\x' needs two hex digits");
        }
        advance(lex);
        advance(lex);
        advance(lex);
    } else if (c != '\0' && strchr("ntr0\\'\"$", c)) {  // strchr finds '\0' in any string
        advance(lex);
    } else {
        error_at(lex->src, line, col, "invalid escape sequence");
    }
}

static void char_literal(Lexer *lex) {
    int line = lex->line, col = current_col(lex);
    advance(lex);
    char c = peek(lex);
    if (c == '\\') {
        escape(lex);
    } else if (c == '\'') {
        error_at(lex->src, line, col, "empty character literal");
    } else if (c >= 32 && c <= 126) {
        advance(lex);
    } else if (!at_end(lex) && c != '\n') {  // end of input or line falls through to "unterminated"
        error_at(lex->src, lex->line, current_col(lex), "invalid character in character literal");
    }
    if (!match(lex, '\'')) error_at(lex->src, line, col, "unterminated character literal");
}

// The expression is left for the parser to lex again.
static void interpolation(Lexer *lex) {
    int line = lex->line, col = current_col(lex);
    advance(lex);
    advance(lex);
    while (peek(lex) != '}') {
        // Strings aren't allowed inside, so a '"' means the '}' is missing.
        if (at_end(lex) || peek(lex) == '\n' || peek(lex) == '"') {
            error_at(lex->src, line, col, "unterminated interpolation");
        }
        if (peek(lex) == '\'') {
            char_literal(lex);  // may hold a '}'
        } else {
            advance(lex);
        }
    }
    advance(lex);
}

static void string_literal(Lexer *lex) {
    int line = lex->line, col = current_col(lex);
    advance(lex);
    while (peek(lex) != '"') {
        if (at_end(lex) || peek(lex) == '\n') error_at(lex->src, line, col, "unterminated string literal");
        if (peek(lex) == '\\') {
            escape(lex);
        } else if (peek(lex) == '$' && peek_next(lex) == '{') {
            interpolation(lex);
        } else {
            advance(lex);
        }
    }
    advance(lex);
}

static void number(Lexer *lex, int line, int col) {
    if (peek(lex) == '0' && peek_next(lex) == 'x') {
        advance(lex);
        advance(lex);
        const char *digits = lex->p;
        while (is_hex_digit(peek(lex))) advance(lex);
        if (lex->p == digits) error_at(lex->src, line, col, "expected hex digits after '0x'");
        // Leading zeros don't count toward the 8-digit (32-bit) limit.
        while (*digits == '0' && digits < lex->p - 1) digits++;
        if (lex->p - digits > 8) error_at(lex->src, line, col, "integer literal is too large");
    } else {
        // 2147483648 passes: it is valid after a unary '-', which only the parser can see.
        long long value = 0;
        while (is_digit(peek(lex))) {
            value = value * 10 + (advance(lex) - '0');
            if (value > 2147483648LL) error_at(lex->src, line, col, "integer literal is too large");
        }
    }
    if (is_ident_char(peek(lex))) {
        error_at(lex->src, lex->line, current_col(lex), "invalid character '%c' in number", peek(lex));
    }
}

Lexer lexer_init(const Source *src) {
    return (Lexer) { .src = src, .p = src->text, .line_start = src->text, .line = 1 };
}

Token lexer_next(Lexer *lex) {
    skip_whitespace(lex);

    const char *start = lex->p;
    int line = lex->line, col = current_col(lex);

    if (at_end(lex)) return make_token(lex, TOK_EOF, start, line, col);

    char c = peek(lex);

    if (is_digit(c)) {
        number(lex, line, col);
        return make_token(lex, TOK_INTEGER, start, line, col);
    }
    if (is_ident_start(c)) {
        while (is_ident_char(peek(lex))) advance(lex);
        int len = (int) (lex->p - start);
        if (is_reserved(start, len)) error_at(lex->src, line, col, "'%.*s' is reserved", len, start);
        return make_token(lex, ident_kind(start, len), start, line, col);
    }
    if (c == '\'') {
        char_literal(lex);
        return make_token(lex, TOK_CHARACTER, start, line, col);
    }
    if (c == '"') {
        string_literal(lex);
        return make_token(lex, TOK_STRING, start, line, col);
    }

    advance(lex);
    TokenType kind;
    switch (c) {
        case '(': kind = TOK_LPAREN; break;
        case ')': kind = TOK_RPAREN; break;
        case '[': kind = TOK_LBRACKET; break;
        case ']': kind = TOK_RBRACKET; break;
        case '{': kind = TOK_LBRACE; break;
        case '}': kind = TOK_RBRACE; break;
        case ',': kind = TOK_COMMA; break;
        case ';': kind = TOK_SEMICOLON; break;
        case ':': kind = TOK_COLON; break;
        case '@': kind = TOK_AT; break;
        case '~': kind = TOK_TILDE; break;
        case '.': kind = match(lex, '.') ? TOK_DOTDOT : TOK_DOT; break;
        case '=': kind = match(lex, '>') ? TOK_FAT_ARROW : match(lex, '=') ? TOK_EQ : TOK_ASSIGN; break;
        case '!': kind = match(lex, '=') ? TOK_NE : TOK_BANG; break;
        case '+': kind = match(lex, '+') ? TOK_INC : match(lex, '=') ? TOK_PLUS_ASSIGN : TOK_PLUS; break;
        case '-':
            kind = match(lex, '>') ? TOK_ARROW : match(lex, '-') ? TOK_DEC : match(lex, '=') ? TOK_MINUS_ASSIGN : TOK_MINUS;
            break;
        case '*': kind = match(lex, '=') ? TOK_STAR_ASSIGN : TOK_STAR; break;
        case '/': kind = match(lex, '=') ? TOK_SLASH_ASSIGN : TOK_SLASH; break;
        case '%': kind = match(lex, '=') ? TOK_PERCENT_ASSIGN : TOK_PERCENT; break;
        case '^': kind = match(lex, '=') ? TOK_CARET_ASSIGN : TOK_CARET; break;
        case '&': kind = match(lex, '&') ? TOK_AND_AND : match(lex, '=') ? TOK_AMP_ASSIGN : TOK_AMP; break;
        case '|': kind = match(lex, '|') ? TOK_PIPE_PIPE : match(lex, '=') ? TOK_PIPE_ASSIGN : TOK_PIPE; break;
        case '<':
            kind = match(lex, '<') ? (match(lex, '=') ? TOK_SHL_ASSIGN : TOK_SHL) : match(lex, '=') ? TOK_LE : TOK_LT;
            break;
        case '>':
            kind = match(lex, '>') ? (match(lex, '=') ? TOK_SHR_ASSIGN : TOK_SHR) : match(lex, '=') ? TOK_GE : TOK_GT;
            break;
        default:
            if (c >= 32 && c <= 126) error_at(lex->src, line, col, "unexpected character '%c'", c);
            error_at(lex->src, line, col, "unexpected byte 0x%02X", (unsigned char) c);
    }
    return make_token(lex, kind, start, line, col);
}
