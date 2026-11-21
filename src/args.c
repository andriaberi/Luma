#include "args.h"
#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(FILE *out) {
    fprintf(out, "usage: luma <file.luma> [-o out | -d tokens|ast|asm]\n");
}

_Noreturn static void bad_args(const char *msg, const char *arg) {
    fprintf(stderr, "luma: %s '%s'\n", msg, arg);
    usage(stderr);
    exit(1);
}

Options parse_args(int argc, char **argv) {
    Options opt = { 0 };

    if (argc < 2) {
        usage(stderr);
        exit(1);
    }

    if (strcmp(argv[1], "-h") == 0) {
        usage(stdout);
        exit(0);
    }

    if (strcmp(argv[1], "-v") == 0) {
        printf("luma %s\n", LUMA_VERSION);
        exit(0);
    }

    if (argv[1][0] == '-') bad_args("unknown option", argv[1]);

    opt.path = argv[1];

    // The only forms are "file" and "file -o out | -d mode".
    if (argc == 2) return opt;
    if (argc == 3) bad_args("missing value after", argv[2]);
    if (argc > 4) bad_args("unexpected argument", argv[4]);

    const char *flag = argv[2], *value = argv[3];

    if (strcmp(flag, "-o") == 0) {
        opt.out = value;
    } else if (strcmp(flag, "-d") == 0) {
        if (strcmp(value, "tokens") == 0) {
            opt.dump = DUMP_TOKENS;
        } else if (strcmp(value, "ast") == 0) {
            opt.dump = DUMP_AST;
        } else if (strcmp(value, "asm") == 0) {
            opt.dump = DUMP_ASM;
        } else {
            bad_args("unknown dump value", value);
        }
    } else {
        bad_args("unknown option", flag);
    }

    return opt;
}
