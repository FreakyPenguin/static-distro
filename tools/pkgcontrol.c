#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "control.h"

static int parse_params(int argc, char *argv[]);

enum field {
    FIELD_UNSPECIFIED,
    FIELD_NAME,
    FIELD_VERSION,
    FIELD_SOURCE,
    FIELD_DEP_RUN,
    FIELD_BUILT_FROM,
};

static enum field field = FIELD_UNSPECIFIED;

int main(int argc, char *argv[])
{
    int num;
    struct control *c;
    struct control_dependency *cd;
    struct control_built_from *cbf;

    if ((num = parse_params(argc, argv)) < 0) {
        fprintf(stderr, "Usage: pkgcontrol [COMMAND] FILES...\n");
        fprintf(stderr, "Commands:\n");
        fprintf(stderr, "    -n: Dump package name\n");
        fprintf(stderr, "    -v: Dump package version\n");
        fprintf(stderr, "    -s: Dump package source\n");
        fprintf(stderr, "    -r: Dump package runtime dependencies\n");
        fprintf(stderr, "    -b: Dump package built froms\n");
        return EXIT_FAILURE;
    }

    for (; num < argc; num++) {
        if (control_parse(argv[num], &c) != 0) {
            fprintf(stderr, "Error parsing '%s'\n", argv[num]);
            return EXIT_FAILURE;
        }

        switch (field) {
            case FIELD_NAME:
                puts(c->package);
                break;
            case FIELD_VERSION:
                puts(c->version);
                break;
            case FIELD_SOURCE:
                printf("%s,%s\n", c->src_package, c->src_version);
                break;
            case FIELD_DEP_RUN:
                for (cd = c->run_depend; cd != NULL; cd = cd->next) {
                    puts(cd->package);
                }
                break;
            case FIELD_BUILT_FROM:
                for (cbf = c->built_froms; cbf != NULL; cbf = cbf->next) {
                    printf("%s,%s\n", cbf->package, cbf->version);
                }
                break;

            default:
                break;
        }
        control_destroy(c);
    }
    return 0;
}

static int parse_params(int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "nvsrb")) != -1) {
        switch (c) {
            case 'n':
                field = FIELD_NAME;
                break;
            case 'v':
                field = FIELD_VERSION;
                break;
            case 's':
                field = FIELD_SOURCE;
                break;
            case 'r':
                field = FIELD_DEP_RUN;
                break;
            case 'b':
                field = FIELD_BUILT_FROM;
                break;
            case '?':
                fprintf(stderr, "Unknown option: %c\n", optopt);
                return -1;
                break;

            default:
                abort();
        }
    }

    if (field == FIELD_UNSPECIFIED) {
        fprintf(stderr, "No field specified\n");
        return -1;
    }

    return optind;
}
