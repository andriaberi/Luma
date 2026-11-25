#include <stdio.h>

#include "args.h"
#include "lexer/dump.h"
#include "source/file.h"

int main(int argc, char **argv) {
    Options opt = parse_args(argc, argv);
    Source src = source_read(opt.path);

    if (opt.dump == DUMP_TOKENS) {
        dump_tokens(&src);
        return 0;
    }

    // The stages behind these don't exist yet.
    if (opt.dump == DUMP_AST || opt.dump == DUMP_ASM) {
        fprintf(stderr, "luma: -d %s is not implemented yet\n", opt.dump == DUMP_AST ? "ast" : "asm");
        return 1;
    }

    fprintf(stderr, "luma: compiler not implemented yet\n");
    return 1;
}
