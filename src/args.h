#ifndef LUMA_ARGS_H
#define LUMA_ARGS_H

typedef enum {
    DUMP_NONE,
    DUMP_TOKENS,
    DUMP_AST,
    DUMP_ASM
} DumpMode;

typedef struct {
    const char *path;
    const char *out;
    DumpMode dump;
} Options;

// Handles -h, -v and bad arguments itself: prints, then exits.
Options parse_args(int argc, char **argv);

#endif
