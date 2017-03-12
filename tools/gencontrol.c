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
static FILE *outf = NULL;

int main(int argc, char *argv[])
{
    int ret, fd;
    struct source_control *c;
    struct source_control_bin *sb;
    struct control_dependency *cd;

    outf = stdout;
    if (parse_params(argc, argv) != 0) {
        fprintf(stderr, "Usage: gencontrol [OPTIONS] CONTROL-FILE "
            "PACKAGE-NAME\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "    -v V: Override version number with V\n");
        fprintf(stderr, "    -V V: Append V to version number\n");
        fprintf(stderr, "    -o F: Write control file to F\n");
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

    fprintf(outf, "Depends-Run:\n");
    for (cd = sb->run_depend; cd != NULL; cd = cd->next)
        fprintf(outf, "  %s\n", cd->package);

    source_control_destroy(c);

    if (outf != stdout)
      fclose(outf);

    return EXIT_SUCCESS;
}

static int parse_params(int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "v:V:o:")) != -1) {
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
                    return -1;
                }
                break;
            case '?':
                fprintf(stderr, "Unknown option: %c\n", optopt);
                return -1;
                break;

            default:
                abort();
        }
    }

    if (optind + 2 != argc)
        return -1;

    file_name = argv[optind];
    package_name = argv[optind + 1];

    return 0;
}
