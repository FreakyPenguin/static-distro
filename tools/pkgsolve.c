#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "control.h"
#include "solve.h"

/* Path to packages dir */
static const char *packagedir = "/packages";
/* Separator for package name and version */
static const char *nameversep = ",";

static int pkgdir_fd;

static int parse_params(int argc, char *argv[]);
static int get_pkt(struct solve_problem *p, const char *name, void *opaque);


int main(int argc, char *argv[])
{
    int num;
    unsigned i;
    struct solve_problem *p;
    struct solve_params params = {
            .get_pkt = get_pkt,
            .opaque = NULL,
        };
    const char *pkg_name, *pkg_version;
    int ret = EXIT_SUCCESS;

    if ((num = parse_params(argc, argv)) < 0) {
        fprintf(stderr, "Usage: pkgsolve [OPTIONS] CONSTRAINTS...\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "    -d DIR: Package directory (default: /packages)\n");
        fprintf(stderr, "    -s SEP: Separator between package name and version"
                " in output (default: ,)\n");
        return EXIT_FAILURE;
    }

    if ((pkgdir_fd = open(packagedir, O_RDONLY | O_DIRECTORY)) == -1) {
        perror("opening package dir failed");
        return EXIT_FAILURE;
    }

    if (solve_init(&params, &p) != 0) {
        fprintf(stderr, "Initializing solver failed\n");
        close(pkgdir_fd);
        return EXIT_FAILURE;
    }

    /* add constraints */
    for (; num < argc; num++) {
        if (solve_constraint_add(p, argv[num]) != 0) {
            fprintf(stderr, "Error adding constraint '%s'\n", argv[num]);
            ret = EXIT_FAILURE;
            goto out;
        }
    }

    if (solve_run(p) != 0) {
        ret = EXIT_FAILURE;
        goto out;
    }

    for (i = 0; solve_result_package(p, i, &pkg_name, &pkg_version) == 0; i++) {
        printf("%s%s%s\n", pkg_name, nameversep, pkg_version);
    }

out:
    solve_destroy(p);
    close(pkgdir_fd);
    return ret;
}

static int parse_params(int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "d:s:")) != -1) {
        switch (c) {
            case 'd':
                packagedir = optarg;
                break;
            case 's':
                nameversep = optarg;
                break;
            case '?':
                fprintf(stderr, "Unknown option: %c\n", optopt);
                return -1;
                break;

            default:
                fprintf(stderr, "Unexpected option: %c\n", c);
                abort();
        }
    }

    return optind;
}

static int get_pkt(struct solve_problem *p, const char *name, void *opaque)
{
    int dfd, vfd, cfd, r, ret = 0;
    DIR *pkg_d;
    struct dirent *de;
    unsigned num_deps = 0;
    const char **deps = NULL;
    struct control *c;
    struct control_dependency *cd;

    (void) opaque;

    if ((dfd = openat(pkgdir_fd, name, O_RDONLY | O_DIRECTORY)) == -1) {
        fprintf(stderr, "Opening package dir for '%s' failed\n", name);
        return -1;
    }

    if ((pkg_d = fdopendir(dfd)) == NULL) {
        close(dfd);
        perror("fdopendir failed");
        fprintf(stderr, "fdopendir for package dir failed: '%s'\n", name);
        return -1;
    }

    while ((de = readdir(pkg_d)) != NULL) {
        if (de->d_name[0] == '.') {
            continue;
        }

        /* open version directory */
        if ((vfd = openat(dfd, de->d_name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW))
                == -1)
        {
            fprintf(stderr, "get_pkt: openat failed (%s,%s), skipping "
                    "version\n", name, de->d_name);
            continue;
        }

        /* open control file */
        if ((cfd = openat(vfd, "control", O_RDONLY)) == -1) {
            fprintf(stderr, "get_pkt: openat control failed (%s,%s), skipping "
                    "version\n", name, de->d_name);
            close(vfd);
            continue;
        }
        close(vfd);

        /* parse control file */
        r = control_parsefd(cfd, &c);
        close(cfd);
        if (r != 0) {
            fprintf(stderr, "get_pkt: parsing control failed (%s,%s), skipping "
                    "version\n", name, de->d_name);
            continue;
        }

        /* generate dependency array */
        num_deps = 0;
        for (cd = c->run_depend; cd != NULL; cd = cd->next) {
            num_deps++;
        }

        if (num_deps != 0 && (deps = malloc(sizeof(*deps) * num_deps))
                == NULL)
        {
            fprintf(stderr, "get_pkt: allocating deps array failed\n");
            control_destroy(c);
            ret = -1;
            goto out;
        }

        num_deps = 0;
        for (cd = c->run_depend; cd != NULL; cd = cd->next) {
            deps[num_deps++] = cd->package;
        }

        /* add version */
        if (solve_package_version_add(p, name, de->d_name, num_deps, deps)
                != 0)
        {
            fprintf(stderr, "get_pkt: solve_package_version_add failed (%s,%s)"
                    "\n", name, de->d_name);
            ret = -1;
            control_destroy(c);
            free(deps);
            goto out;
        }
        control_destroy(c);
        free(deps);
    }

out:
    closedir(pkg_d);
    close(dfd);
    return ret;
}
