#include <stdio.h>
#include <stdlib.h>

#include "version.h"

int main(int argc, char *argv[])
{
    enum vercmp_result res;

    if (argc  != 3) {
        fprintf(stderr, "Usage: pkgvercmp VERSION1 VERSION2\n");
        return EXIT_FAILURE;
    }

    if (version_validate(argv[1]) != 0) {
        fprintf(stderr, "Unsupported version number format: '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }
    if (version_validate(argv[2]) != 0) {
        fprintf(stderr, "Unsupported version number format: '%s'\n", argv[2]);
        return EXIT_FAILURE;
    }

    res = version_cmp(argv[1], argv[2]);
    switch (res) {
        case VER_LT: puts("less than"); break;
        case VER_EQ: puts("equal"); break;
        case VER_GT: puts("greater than"); break;
        case VER_FAILED:
            fprintf(stderr, "Unexpected: version_cmp failed\n");
            abort();
    }

    return EXIT_SUCCESS;
}
