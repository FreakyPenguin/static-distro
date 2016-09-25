#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "repo.h"
#include "solve.h"

struct package {
    char *constraint;
    struct package *next;
};

/* Path to packages dir */
static char *packagedir = "/packages";
/* List of packages to be added to environment */
static struct package *packages = NULL;

static int parse_opts(int *argc, char *argv[]);
static int get_pkg(struct solve_problem *p, const char *name, void *opaque);
static int add_constraints(struct solve_problem *p);
static int build_args(struct solve_problem *sp, int argc, char **argv,
        char ***new_argv);

int main(int argc, char *argv[])
{
    struct solve_problem *sp;
    struct solve_params solve_params;
    struct repo *repo;
    int ret;
    char **new_argv;

    if (parse_opts(&argc, argv) != 0) {
        fprintf(stderr, "Usage: withpkgs [OPTIONS] CMD...\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "    -d DIR: Host package directory (default: /packages)"
                "\n");
        fprintf(stderr, "    -p PKT: Add package constraint\n\n");
        fprintf(stderr, "    Other options are passed through to withenv\n");
        return EXIT_FAILURE;
    }

    if ((repo = repo_init(packagedir)) == NULL) {
        fprintf(stderr, "Initializing repo (%s) failed\n", packagedir);
        return EXIT_FAILURE;
    }

    solve_params.opaque = repo;
    solve_params.get_pkt = get_pkg;
    if (solve_init(&solve_params, &sp) != 0) {
        fprintf(stderr, "Initializing solver failed\n");
        repo_destroy(repo);
        return EXIT_FAILURE;
    }

    if (add_constraints(sp) != 0) {
        solve_destroy(sp);
        repo_destroy(repo);
        return EXIT_FAILURE;
    }

    if (solve_run(sp) != 0) {
        fprintf(stderr, "Solving dependencies failed\n");
        solve_destroy(sp);
        repo_destroy(repo);
        return EXIT_FAILURE;
    }

    ret = build_args(sp, argc, argv, &new_argv);
    repo_destroy(repo);
    if (ret != 0) {
        solve_destroy(sp);
        return EXIT_FAILURE;
    }

    execvp(new_argv[0], new_argv);
    perror("execvp failed");
    solve_destroy(sp);

    return EXIT_FAILURE;
}

/* Parse command line options into global variables */
static int parse_opts(int *argc, char *argv[])
{
    int i, j;
    struct package *p;

    for (i = 1, j = 1; i < *argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            if (*argc <= i + 1) {
                return -1;
            }

            packagedir = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-p") == 0) {
            if (*argc <= i + 1) {
                return -1;
            }

            if ((p = malloc(sizeof(*p))) == NULL) {
                fprintf(stderr, "malloc package struct failed\n");
                return -1;
            }
            p->constraint = argv[i + 1];
            p->next = packages;
            packages = p;

            i++;
        } else {
            argv[j++] = argv[i];
        }
    }

    *argc = j;
    return 0;
}

static int get_pkg(struct solve_problem *p, const char *name, void *opaque)
{
    struct repo *repo = opaque;
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

static int add_constraints(struct solve_problem *p)
{
    struct package *pkg;

    for (pkg = packages; pkg != NULL; pkg = pkg->next) {
        if (solve_constraint_add(p, pkg->constraint) != 0) {
            fprintf(stderr, "solve_constraint_add failed\n");
            return -1;
        }
    }

    return 0;
}

static int build_args(struct solve_problem *sp, int argc, char **argv,
        char ***new_argv)
{
    unsigned i, n, nargc;
    const char *name, *version;
    char **nargv;

    /* count result packages */
    for (i = 0; solve_result_package(sp, i, &name, &version) == 0; i++);

    /* argc + 2 for -d packagedir + 2 for each -p package,version */
    nargc = argc + 2 + 2 * i;

    /* argc + 1 for NULL at the end */
    if ((nargv = calloc(nargc + 1, sizeof(*nargv))) == NULL) {
        fprintf(stderr, "build_args: calloc failed\n");
        return -1;
    }
    n = 0;
    nargv[n++] = "withenv";

    /* set working directory */
    nargv[n++] = "-d";
    nargv[n++] = packagedir;

    /* add args for result packages */
    for (i = 0; solve_result_package(sp, i, &name, &version) == 0; i++) {
        nargv[n++] = "-p";
        if (asprintf(&nargv[n++], "%s,%s", name, version) == -1) {
            fprintf(stderr, "build_args: asprintf failed\n");
            return -1;
        }
    }

    /* copy over old argv */
    for (i = 1; i < (unsigned) argc; i++) {
        nargv[n++] = argv[i];
    }

    *new_argv = nargv;

    return 0;
}
