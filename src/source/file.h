#ifndef LUMA_FILE_H
#define LUMA_FILE_H

#include <stddef.h>

typedef struct {
    const char *path;
    const char *text;
    size_t len;
} Source;

// Reads the whole file; prints an error and exits 1 if it cannot be read.
Source source_read(const char *path);

// Start of line `line` (1-based) in src->text; *len gets its length without the line ending.
const char *source_line(const Source *src, int line, size_t *len);

#endif
