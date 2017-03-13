#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "control.h"

enum version_mode {
    VERSION_KEEP,
    VERSION_OVERRIDE,
    VERSION_APPEND,
};

static int parse_params(int argc, char *argv[]);
static int unpack(struct source_control *c);
static int build(struct source_control *c);
static int gencontrol(struct source_control *c);
static int copy_file(int srcd_fd, const char *src, int dstd_fd,
        const char *dst);
static int run_exec(char *script[]);

static int do_build = 1;
static int do_unpack = 1;
static int do_gencontrol = 1;
static char *work_dir = NULL;
static char *dist_dir = NULL;
static char *out_dir = NULL;
static char *ver_param = NULL;
static char *version;
static char *control_path = NULL;
static enum version_mode ver_mode = VERSION_KEEP;

int main(int argc, char *argv[])
{
    int fd, ret;
    struct source_control *c;
    char *pkgdir;

    if (parse_params(argc, argv) != 0) {
        fprintf(stderr, "Usage: pkgbuild OPTIONS SOURCE-CONTROL\n");
        fprintf(stderr, "  Options:\n");
        fprintf(stderr, "    -U       No unpack\n");
        fprintf(stderr, "    -B       No build\n");
        fprintf(stderr, "    -C       Do not generate control files\n");
        fprintf(stderr, "    -o ODIR  Output directory for binary package\n");
        fprintf(stderr, "    -w WDIR  Work directory\n");
        fprintf(stderr, "    -d DDIR  Directory for distfiles\n");
        fprintf(stderr, "    -v VER   Set binary package version\n");
        fprintf(stderr, "    -V AVER  Append AVER to binary package version\n");
        return EXIT_FAILURE;
    }

    /* parse source control file */
    if ((fd = open(control_path, O_RDONLY)) == -1) {
        perror("Opening control file failed");
        return EXIT_FAILURE;
    }
    ret = source_control_parsefd(fd, &c);
    close(fd);
    if (ret != 0) {
        fprintf(stderr, "Parsing source control file failed\n");
        return EXIT_FAILURE;
    }

    /* figure out version */
    version = c->version;
    if (ver_mode == VERSION_OVERRIDE) {
        version = ver_param;
    } else if (ver_mode == VERSION_APPEND) {
        if (asprintf(&version, "%s%s", c->version, ver_param) == -1) {
          perror("asprintf version failed");
          return EXIT_FAILURE;
        }
    }

    /* setup environment */
    if (asprintf(&pkgdir, "/packages/%s/%s", c->source, version) == -1) {
        perror("asprintf pkgdir failed");
        return EXIT_FAILURE;
    }
    if (setenv("PKG_NAME", c->source, 1) != 0 ||
            setenv("PKG_SRCVERSION", c->version, 1) != 0 ||
            setenv("PKG_VERSION", version, 1) != 0 ||
            setenv("PKG_DIR", pkgdir, 1) != 0)
    {
        perror("setenv failed");
        return EXIT_FAILURE;
    }
    if (chdir(work_dir) != 0) {
        perror("chdir failed");
        return EXIT_FAILURE;
    }

    if (do_unpack && unpack(c) != 0)
        return EXIT_FAILURE;

    if (do_build && build(c) != 0)
        return EXIT_FAILURE;

    if (do_gencontrol && gencontrol(c) != 0)
        return EXIT_FAILURE;

    source_control_destroy(c);
    return EXIT_SUCCESS;
}

static int parse_params(int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "BUCo:w:d:v:V:")) != -1) {
        switch (c) {
            case 'U':
                do_unpack = 0;
                break;
            case 'B':
                do_build = 0;
                break;
            case 'C':
                do_gencontrol = 0;
                break;
            case 'o':
                out_dir = optarg;
                break;
            case 'w':
                if ((work_dir = realpath(optarg, NULL)) == NULL) {
                    fprintf(stderr, "realpath(%s) failed: %s\n", optarg,
                            strerror(errno));
                    return -1;
                }
                break;
            case 'd':
                if ((dist_dir = realpath(optarg, NULL)) == NULL) {
                    fprintf(stderr, "realpath(%s) failed: %s\n", optarg,
                            strerror(errno));
                    return -1;
                }
                break;
            case 'v':
                ver_mode = VERSION_OVERRIDE;
                ver_param = optarg;
                break;
            case 'V':
                ver_mode = VERSION_APPEND;
                ver_param = optarg;
                break;
            case '?':
                fprintf(stderr, "Unknown option: %c\n", optopt);
                return -1;

            default:
                fprintf(stderr, "Unexpected option: %c\n", c);
                abort();
        }
    }

    if (work_dir == NULL) {
        fprintf(stderr, "A working directory must be set\n");
        return -1;
    }

    if (optind + 1 != argc)
        return -1;

    if ((control_path = realpath(argv[optind], NULL)) == NULL) {
        fprintf(stderr, "realpath(%s) failed: %s\n", argv[optind],
                strerror(errno));
        return -1;
    }
    return 0;
}

static int unpack(struct source_control *c)
{
    struct control_source *cs;
    int distd_fd, workd_fd;
    int ret = 0;
    char *unpack_argv[] = { "./unpack.sh", NULL };

    if (dist_dir == NULL) {
        fprintf(stderr, "unpack: dist dir must be set\n");
        return -1;
    }

    if ((distd_fd = open(dist_dir, O_RDONLY | O_DIRECTORY)) == -1) {
        perror("unpack: open distdir failed");
        return -1;
    }
    if ((workd_fd = open(work_dir, O_RDONLY | O_DIRECTORY)) == -1) {
        perror("unpack: open workdir failed");
        ret = -1;
        goto error_close_dist;
    }

    /* copying distfiles */
    for (cs = c->sources; cs != NULL; cs = cs->next) {


        if (copy_file(distd_fd, cs->source, workd_fd, cs->source) != 0) {
            fprintf(stderr, "unpack: copying source %s failed\n", cs->source);
            ret = -1;
            goto error_close_work;
        }
    }

    /* run unpack script */
    if (run_exec(unpack_argv) != 0) {
        fprintf(stderr, "unpack: running unpack script failed\n");
        goto error_close_work;
    }

error_close_work:
    close(workd_fd);
error_close_dist:
    close(distd_fd);
    return ret;
}

static int build(struct source_control *c)
{
    int ret;
    char *outdir;
    char *build_argv[] = { "./build.sh", NULL };

    (void) c;

    if (out_dir == NULL) {
        fprintf(stderr, "build: out dir must be set\n");
        return -1;
    }

    if (asprintf(&outdir, "%s/%s", work_dir, out_dir) == -1) {
        perror("build: asprintf failed");
        return -1;
    }
    ret = setenv("PKG_INSTDIR", outdir, 1);
    free(outdir);
    if (ret != 0) {
        perror("build: setting PKG_INSTDIR failed");
        return -1;
    }

    /* run build script */
    if (run_exec(build_argv) != 0) {
        fprintf(stderr, "build: running build script failed\n");
        return -1;
    }

    return 0;
}

static int gencontrol(struct source_control *c)
{
    struct source_control_bin *sb;
    char *out_control;
    char *argv[] = { "gencontrol", "-o", NULL, control_path, NULL, NULL, };

    if (out_dir == NULL) {
        fprintf(stderr, "gencontrol: out dir must be set\n");
        return -1;
    }

    for (sb = c->bins; sb != NULL; sb = sb->next) {
        if (asprintf(&out_control, "%s/%s/packages/%s/%s/control", work_dir,
                    out_dir, sb->package, version) == -1)
        {
            perror("gencontrol: asprintf failed");
            return -1;
        }

        argv[4] = sb->package;
        argv[2] = out_control;

        if (run_exec(argv) != 0) {
            fprintf(stderr, "gencontrol: running for %s failed\n", sb->package);
            free(out_control);
            return -1;
        }

        free(out_control);
    }

    return 0;
}

static int copy_file(int srcd_fd, const char *src, int dstd_fd,
        const char *dst)
{
    if (linkat(srcd_fd, src, dstd_fd, dst, AT_SYMLINK_FOLLOW) != 0) {
        perror("copy_file failed");
        /* TODO: offer copy as fallback? */
        return -1;
    }

    /* run build script */

    return 0;
}

static int run_exec(char *argv[])
{
    pid_t pid, p2;
    int st;

    pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("run_exec: execvp failed");
        exit(-1);
    } else if (pid > 0) {
        do {
            p2 = waitpid(pid, &st, 0);
            if (p2 == -1) {
                perror("run_exec: waitpid failed");
                return -1;
            } else if (p2 != pid) {
                fprintf(stderr, "run_exec: waitpid unexpected pid: %d "
                        "expected %d\n", p2, pid);
                return -1;
            }
        } while (p2 != pid);

        if (!WIFEXITED(st) || WEXITSTATUS(st) != EXIT_SUCCESS) {
            return -1;
        }
    } else {
        perror("run_exec: fork failed");
        return -1;
    }

    return 0;
}
