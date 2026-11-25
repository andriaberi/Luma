#ifndef LUMA_LEXER_DUMP_H
#define LUMA_LEXER_DUMP_H

#include "source/file.h"

// -d tokens: prints every token of src to stdout as "line:col KIND lexeme", columns aligned.
void dump_tokens(const Source *src);

#endif
