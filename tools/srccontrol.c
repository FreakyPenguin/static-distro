#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "control.h"

static int parse_params(int argc, char *argv[]);

enum field {
    FIELD_UNSPECIFIED,
    FIELD_NAME,
    FIELD_VERSION,
    FIELD_DEP_BUILD,
    FIELD_DEP_UNPACK,
    FIELD_SRCS,
    FIELD_PKGS,
};

static enum field field = FIELD_UNSPECIFIED;

int main(int argc, char *argv[])
{
    int num, ret, fd;
    struct source_control *c;
    struct control_dependency *cd;
    struct control_source *cs;
    struct source_control_bin *sb;

    if ((num = parse_params(argc, argv)) < 0) {
        fprintf(stderr, "Usage: srccontrol [COMMAND] FILES...\n");
        fprintf(stderr, "Commands:\n");
        fprintf(stderr, "    -n: Dump source name\n");
        fprintf(stderr, "    -v: Dump source version\n");
        fprintf(stderr, "    -b: Dump build dependencies\n");
        fprintf(stderr, "    -u: Dump unpack dependencies\n");
        fprintf(stderr, "    -s: Dump package sources\n");
        fprintf(stderr, "    -p: Dump built binary packages\n");
        return EXIT_FAILURE;
    }

    for (; num < argc; num++) {
        if ((fd = open(argv[num], O_RDONLY)) == -1) {
            perror("Opening source control file failed");
            fprintf(stderr, "Opening %s failed\n", argv[num]);
        }
        ret = source_control_parsefd(fd, &c);
        close(fd);
        if (ret != 0) {
            fprintf(stderr, "Error parsing '%s'\n", argv[num]);
            return EXIT_FAILURE;
        }

        switch (field) {
            case FIELD_NAME:
                puts(c->source);
                break;
            case FIELD_VERSION:
                puts(c->version);
                break;
            case FIELD_DEP_BUILD:
                for (cd = c->build_depend; cd != NULL; cd = cd->next) {
                    puts(cd->package);
                }
                break;
            case FIELD_DEP_UNPACK:
                for (cd = c->unpack_depend; cd != NULL; cd = cd->next) {
                    puts(cd->package);
                }
                break;
            case FIELD_SRCS:
                for (cs = c->sources; cs != NULL; cs = cs->next) {
                    puts(cs->source);
                }
                break;
            case FIELD_PKGS:
                for (sb = c->bins; sb != NULL; sb = sb->next) {
                    puts(sb->package);
                }
                break;

            default:
                break;
        }
        source_control_destroy(c);
    }
    return 0;
}

static int parse_params(int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "nvbusp")) != -1) {
        switch (c) {
            case 'n':
                field = FIELD_NAME;
                break;
            case 'v':
                field = FIELD_VERSION;
                break;
            case 'b':
                field = FIELD_DEP_BUILD;
                break;
            case 'u':
                field = FIELD_DEP_UNPACK;
                break;
            case 's':
                field = FIELD_SRCS;
                break;
            case 'p':
                field = FIELD_PKGS;
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
