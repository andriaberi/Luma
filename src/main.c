#include <stdio.h>
#include <string.h>

#define LUMA_VERSION "0.1.0"

static void print_usage(FILE *out) {
    fprintf(out, "usage: luma <file.luma> [-o out | -d tokens|ast|asm]\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(stderr);
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0) {
        print_usage(stdout);
        return 0;
    }

    if (strcmp(argv[1], "-v") == 0) {
        printf("luma %s\n", LUMA_VERSION);
        return 0;
    }

    fprintf(stderr, "luma: compiler not implemented yet\n");
    return 1;
}
