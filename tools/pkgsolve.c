#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "control.h"
#include "solve.h"
#include "repo.h"

/* Path to packages dir */
static const char *packagedir = "/packages";
/* Separator for package name and version */
static const char *nameversep = ",";

static struct repo *repo = NULL;

static int parse_params(int argc, char *argv[]);
static int get_pkg(struct solve_problem *p, const char *name, void *opaque);


int main(int argc, char *argv[])
{
    int num;
    unsigned i;
    struct solve_problem *p;
    struct solve_params params = {
            .get_pkt = get_pkg,
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

    if ((repo = repo_init(packagedir)) == NULL) {
        perror("opening repo failed");
        return EXIT_FAILURE;
    }

    if (solve_init(&params, &p) != 0) {
        fprintf(stderr, "Initializing solver failed\n");
        repo_destroy(repo);
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
    repo_destroy(repo);
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

static int get_pkg(struct solve_problem *p, const char *name, void *opaque)
{
    struct repo_package *pkg;
    struct repo_version *ver;
    int ret;
    unsigned num_deps = 0;
    const char **deps = NULL;
    struct control_dependency *cd;

    (void) opaque;

    if ((pkg = repo_package_get(repo, name)) == NULL) {
        fprintf(stderr, "get_pkg: repo_package_get(%s) failed\n", name);
        return -1;
    }

    for (ver = pkg->versions; pkg != NULL; pkg = pkg->next) {
        /* generate dependency array */
        num_deps = 0;
        for (cd = ver->control->run_depend; cd != NULL; cd = cd->next) {
            num_deps++;
        }

        if (num_deps != 0 && (deps = malloc(sizeof(*deps) * num_deps))
                == NULL)
        {
            fprintf(stderr, "get_pkg: allocating deps array failed\n");
            return -1;
        }

        num_deps = 0;
        for (cd = ver->control->run_depend; cd != NULL; cd = cd->next) {
            deps[num_deps++] = cd->package;
        }

        /* add version */
        ret = solve_package_version_add(p, name, ver->version, num_deps, deps);
        free(deps);
        if (ret != 0) {
            fprintf(stderr, "get_pkg: solve_package_version_add failed (%s,%s)"
                    "\n", name, ver->version);
            return -1;
        }
    }

    return 0;
}
