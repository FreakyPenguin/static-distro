#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "control.h"

static int parse_params(int argc, char *argv[]);

enum version_mode {
    VERSION_KEEP,
    VERSION_OVERRIDE,
    VERSION_APPEND,
};

static char *file_name = NULL;
static char *package_name = NULL;
static enum version_mode ver_mode = VERSION_KEEP;
static char *ver_param = NULL;
static struct control_built_from *bf_extra = NULL;
static FILE *outf = NULL;

int main(int argc, char *argv[])
{
    int ret, fd;
    struct source_control *c;
    struct source_control_bin *sb;
    struct control_dependency *cd;
    struct control_built_from *bf;

    outf = stdout;
    if (parse_params(argc, argv) != 0) {
        fprintf(stderr, "Usage: gencontrol [OPTIONS] CONTROL-FILE "
            "PACKAGE-NAME\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "    -v V: Override version number with V\n");
        fprintf(stderr, "    -V V: Append V to version number\n");
        fprintf(stderr, "    -o F: Write control file to F\n");
        fprintf(stderr, "    -b P,V: Add extra packet P version V to built "
            "from list\n");
        return EXIT_FAILURE;
    }

    /* parse source control file */
    if ((fd = open(file_name, O_RDONLY)) == -1) {
        perror("Opening control file failed");
        return EXIT_FAILURE;
    }
    ret = source_control_parsefd(fd, &c);
    close(fd);
    if (ret != 0) {
        fprintf(stderr, "Parsing source control file failed\n");
        return EXIT_FAILURE;
    }

    /* look for package definition */
    for (sb = c->bins; sb != NULL && strcmp(sb->package, package_name) != 0;
        sb = sb->next);
    if (sb == NULL) {
        fprintf(stderr, "Package '%s' not found\n", package_name);
        return EXIT_FAILURE;
    }

    fprintf(outf, "Package: %s\n", sb->package);

    switch (ver_mode) {
        case VERSION_KEEP:
            fprintf(outf, "Version: %s\n", c->version);
            break;
        case VERSION_OVERRIDE:
            fprintf(outf, "Version: %s\n", ver_param);
            break;
        case VERSION_APPEND:
            fprintf(outf, "Version: %s%s\n", c->version, ver_param);
            break;
        default:
            abort();
    }

    fprintf(outf, "Source: %s %s\n", c->source, c->version);

    fprintf(outf, "Depends-Run:\n");
    for (cd = sb->run_depend; cd != NULL; cd = cd->next)
        fprintf(outf, "  %s\n", cd->package);

    if (bf_extra != NULL) {
        fprintf(outf, "Built-From:\n");
        for (bf = bf_extra; bf != NULL; bf = bf->next)
          fprintf(outf, "  %s %s\n", bf->package, bf->version);
    }

    source_control_destroy(c);

    if (outf != stdout)
      fclose(outf);

    while ((bf = bf_extra) != NULL) {
        bf_extra = bf->next;
        free(bf);
    }

    return EXIT_SUCCESS;
}

static int parse_params(int argc, char *argv[])
{
    int c;
    char *comma;
    struct control_built_from *bf, *bf_first = NULL, *bf_last = NULL;

    while ((c = getopt(argc, argv, "v:V:o:b:")) != -1) {
        switch (c) {
            case 'v':
                ver_mode = VERSION_OVERRIDE;
                ver_param = optarg;
                break;
            case 'V':
                ver_mode = VERSION_APPEND;
                ver_param = optarg;
                break;
            case 'o':
                if ((outf = fopen(optarg, "w")) == NULL) {
                    fprintf(stderr, "Opening output file failed: %s\n", optarg);
                    goto out_error;
                }
                break;
            case 'b':
                if ((comma = strchr(optarg, ',')) == NULL) {
                    fprintf(stderr, "Option -b expects packet,version format, "
                        " no comma found (%s)\n", optarg);
                    goto out_error;
                }
                *comma = 0;

                if ((bf = malloc(sizeof(*bf))) == NULL) {
                    perror("parse_params: malloc built from failed");
                    goto out_error;
                }
                bf->package = optarg;
                bf->version = comma + 1;

                bf->next = NULL;
                if (bf_last != NULL) {
                    bf_last->next = bf;
                } else {
                    bf_first = bf;
                }
                bf_last = bf;
                break;

            case '?':
                fprintf(stderr, "Unknown option: %c\n", optopt);
                goto out_error;
                break;

            default:
                abort();
        }
    }

    if (optind + 2 != argc)
        goto out_error;

    file_name = argv[optind];
    package_name = argv[optind + 1];

    bf_extra = bf_first;

    return 0;

out_error:
    for (; (bf = bf_first) != NULL ; ) {
        bf_first = bf->next;
        free(bf);
    }
    return -1;
}
