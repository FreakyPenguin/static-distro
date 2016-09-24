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

    res = version_cmp(argv[1], argv[2]);
    switch (res) {
        case VER_LT: puts("less than"); break;
        case VER_EQ: puts("equal"); break;
        case VER_GT: puts("greater than"); break;
        case VER_FAILED:
            fprintf(stderr, "Unsupported version number format\n");
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
