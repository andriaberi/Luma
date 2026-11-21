#include "source/diag.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void error_at(const Source *src, int line, int col, const char *fmt, ...) {
    fflush(stdout);  // -d output printed so far comes before the error
    fprintf(stderr, "%s:%d:%d: error: ", src->path, line, col);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fputc('\n', stderr);

    size_t len;
    const char *text = source_line(src, line, &len);

    fprintf(stderr, "    %.*s\n    ", (int) len, text);
    // Copy tabs so the ^ lines up however the terminal draws them, and skip UTF-8
    // continuation bytes so each character before col takes one column.
    for (int i = 0; i < col - 1 && (size_t) i < len; i++) {
        unsigned char c = (unsigned char) text[i];
        if ((c & 0xC0) == 0x80) continue;
        fputc(c == '\t' ? '\t' : ' ', stderr);
    }

    fputs("^\n", stderr);

    exit(1);
}
