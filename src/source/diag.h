#ifndef LUMA_DIAG_H
#define LUMA_DIAG_H

#include "source/file.h"

// Prints "path:line:col: error: <message>", the source line and a ^ under col, then exits 1.
// line and col are 1-based; col counts bytes, the ^ is placed by characters.
_Noreturn void error_at(const Source *src, int line, int col, const char *fmt, ...)
    __attribute__((format(printf, 4, 5)));

#endif
