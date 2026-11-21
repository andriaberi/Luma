#include "source/file.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Noreturn static void cannot_read(const char *path) {
    fprintf(stderr, "luma: cannot read '%s': %s\n", path, strerror(errno));
    exit(1);
}

Source source_read(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) cannot_read(path);

    // Read in chunks until EOF: no size guess needed, and a directory fails here with EISDIR.
    size_t len = 0, cap = 0;
    char *text = NULL;
    do {
        if (len == cap) {
            cap = cap ? cap * 2 : 4096;
            text = realloc(text, cap + 1);  // +1 for the '\0'
            if (!text) {
                fprintf(stderr, "luma: out of memory reading '%s'\n", path);
                exit(1);
            }
        }
        len += fread(text + len, 1, cap - len, fp);
    } while (!feof(fp) && !ferror(fp));

    if (ferror(fp)) cannot_read(path);
    fclose(fp);
    text[len] = '\0';

    return (Source) { .path = path, .text = text, .len = len };
}

const char *source_line(const Source *src, int line, size_t *len) {
    const char *p = src->text, *end = src->text + src->len;
    for (int n = 1; n < line && p < end; p++) {
        if (*p == '\n') n++;
    }

    const char *e = p;
    while (e < end && *e != '\n') e++;
    if (e > p && e[-1] == '\r') e--;

    *len = (size_t) (e - p);
    return p;
}
