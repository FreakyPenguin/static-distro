#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "version.h"

enum mode {
    MODE_CMP,
    MODE_MIN,
    MODE_MAX,
};

static int parse_params(int argc, char *argv[]);

static int mode = MODE_CMP;

int main(int argc, char *argv[])
{
    enum vercmp_result res;
    int i, j;

    if ((i = parse_params(argc, argv)) == -1) {
        fprintf(stderr, "Usage: pkgvercmp [-m|-M] [VERSIONS...]\n");
        fprintf(stderr, "  Modes:\n");
        fprintf(stderr, "    Minimum -m: Multiple version numbers can be \n"
                        "      specified, the smallest one will be printed.\n");
        fprintf(stderr, "    Maximum -M: Multiple version numbers can be \n"
                        "      specified, the largest one will be printed.\n");
        fprintf(stderr, "    Comparision: If neither -m nor -M was specified \n"
                        "      two version numbers are expected and will be \n"
                        "      compared\n");
        return EXIT_FAILURE;
    }

    for (j = i; j < argc; j++) {
        if (version_validate(argv[j]) != 0) {
            fprintf(stderr, "Unsupported version number format: '%s'\n", argv[j]);
            return EXIT_FAILURE;
        }
    }

    if (mode == MODE_CMP) {
        res = version_cmp(argv[i], argv[i + 1]);
        switch (res) {
            case VER_LT: puts("less than"); break;
            case VER_EQ: puts("equal"); break;
            case VER_GT: puts("greater than"); break;
            case VER_FAILED:
                fprintf(stderr, "Unexpected: version_cmp failed\n");
                abort();
        }
    } else {
        for (j = i + 1; j < argc; j++) {
            res = version_cmp(argv[j], argv[i]);
            if ((res == VER_LT && mode == MODE_MIN) ||
                (res == VER_GT && mode == MODE_MAX))
            {
                i = j;
            } else if (res == VER_FAILED) {
                fprintf(stderr, "Unexpected: version_cmp failed: '%s' vs "
                        "'%s'\n", argv[j], argv[i]);
                abort();
            }
        }
        puts(argv[i]);
    }

    return EXIT_SUCCESS;
}

static int parse_params(int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "mM")) != -1) {
        switch (c) {
            case 'm':
                mode = MODE_MIN;
                break;
            case 'M':
                mode = MODE_MAX;
                break;
            case '?':
                fprintf(stderr, "Unknown option: %c\n", optopt);
                return -1;

            default:
                fprintf(stderr, "Unexpected option: %c\n", c);
                abort();
        }
    }

    /* if comparing, then we need exactly two version numbers */
    if (mode == MODE_CMP && optind + 2 != argc) {
        return -1;
    } else if (optind == argc) {
        /* we always need at least one number */
        return -1;
    }

    return optind;
}
