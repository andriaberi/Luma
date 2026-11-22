#ifndef LUMA_TOKEN_H
#define LUMA_TOKEN_H

typedef enum {
    TOK_EOF,
    TOK_UNKNOWN,

    // Tokens with text; main.c prints the lexeme for TOK_IDENT..TOK_STRING
    TOK_IDENT,
    TOK_INTEGER,
    TOK_CHARACTER,
    TOK_STRING,

    // Keywords
    TOK_INT, TOK_CHAR, TOK_BOOL,
    TOK_CONST, TOK_STRUCT, TOK_ENUM,
    TOK_FN, TOK_RETURN,
    TOK_WHEN, TOK_LOOP,
    TOK_BREAK, TOK_CONTINUE,
    TOK_IMPORT, TOK_EXPORT,
    TOK_NEW, TOK_DELETE,
    TOK_IN, TOK_OUT, TOK_ERR,
    TOK_TRUE, TOK_FALSE, TOK_NULL,
    TOK_UNDERSCORE,     // _

    // Delimiters
    TOK_LPAREN,         // (
    TOK_RPAREN,         // )
    TOK_LBRACKET,       // [
    TOK_RBRACKET,       // ]
    TOK_LBRACE,         // {
    TOK_RBRACE,         // }
    TOK_COMMA,          // ,
    TOK_SEMICOLON,      // ;
    TOK_COLON,          // :
    TOK_DOT,            // .
    TOK_DOTDOT,         // ..
    TOK_FAT_ARROW,      // =>
    TOK_ARROW,          // ->

    // Operators
    TOK_AT,             // @
    TOK_AMP,            // &
    TOK_BANG,           // !
    TOK_TILDE,          // ~
    TOK_PLUS,           // +
    TOK_MINUS,          // -
    TOK_STAR,           // *
    TOK_SLASH,          // /
    TOK_PERCENT,        // %
    TOK_SHL,            // <<
    TOK_SHR,            // >>
    TOK_PIPE,           // |
    TOK_CARET,          // ^
    TOK_LT,             // <
    TOK_LE,             // <=
    TOK_GT,             // >
    TOK_GE,             // >=
    TOK_EQ,             // ==
    TOK_NE,             // !=
    TOK_AND_AND,        // &&
    TOK_PIPE_PIPE,      // ||

    // Assignment statements
    TOK_ASSIGN,         // =
    TOK_PLUS_ASSIGN,    // +=
    TOK_MINUS_ASSIGN,   // -=
    TOK_STAR_ASSIGN,    // *=
    TOK_SLASH_ASSIGN,   // /=
    TOK_PERCENT_ASSIGN, // %=
    TOK_AMP_ASSIGN,     // &=
    TOK_PIPE_ASSIGN,    // |=
    TOK_CARET_ASSIGN,   // ^=
    TOK_SHL_ASSIGN,     // <<=
    TOK_SHR_ASSIGN,     // >>=
    TOK_INC,            // ++
    TOK_DEC             // --
} TokenType;

typedef struct {
    TokenType kind;
    const char *start;  // into the source text, not '\0'-terminated
    int len;
    int line, col;      // 1-based; col counts bytes
} Token;

const char *token_name(TokenType type);

#endif
