#include "solve.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct solve_problem {
    struct solve_params params;
    struct package *packages_first;
    struct package *packages_last;
    int solved;
};

struct package {
    char *name;
    struct version *available;
    struct version *chosen;
    struct package *next;
    int scanned;
};

struct version {
    char *version;
    struct dependency *dependencies;
    struct version *next;
};

struct dependency {
    char *constraint;
    struct dependency *next;
};

static inline struct package *pkg_lookup(struct solve_problem *p,
        const char *name);
static inline struct package *pkg_add(struct solve_problem *p,
        const char *name);

int solve_init(const struct solve_params *params, struct solve_problem **pp)
{
    struct solve_problem *p;
    if ((p = malloc(sizeof(*p))) == NULL) {
        perror("solve_init: malloc failed");
        return -1;
    }
    p->params = *params;
    p->packages_first = NULL;
    p->packages_last = NULL;
    p->solved = 0;
    *pp = p;
    return 0;
}

void solve_destroy(struct solve_problem *p)
{
    struct package *pkg, *pkg_next;
    struct version *ver, *ver_next;
    struct dependency *dep, *dep_next;

    for (pkg = p->packages_first; pkg != NULL; pkg = pkg_next) {
        for (ver = pkg->available; ver != NULL; ver = ver_next) {
            for (dep = ver->dependencies; dep != NULL; dep = dep_next) {
                dep_next = dep->next;
                free(dep->constraint);
                free(dep);
            }
            ver_next = ver->next;
            free(ver->version);
            free(ver);
        }
        pkg_next = pkg->next;
        free(pkg->name);
        free(pkg);
    }
    free(p);
}

int solve_constraint_add(struct solve_problem *p, const char *constraint)
{
    struct package *pkg;

    if ((pkg = pkg_add(p, constraint)) == NULL) {
        return -1;
    }

    return 0;
}

int solve_package_version_add(struct solve_problem *p, const char *name,
        const char *version, unsigned n_deps, const char **deps)
{
    struct package *pkg;
    struct version *ver;
    struct dependency *dep, *dep_next;
    unsigned i;

    if ((pkg = pkg_add(p, name)) == NULL) {
        return -1;
    }

    /* make sure version does not exist yet */
    for (ver = pkg->available; ver != NULL; ver = ver->next) {
        if (strcmp(ver->version, version) == 0) {
            return 0;
        }
    }

    /* allocate version struct and copy string */
    if ((ver = malloc(sizeof(*ver))) == NULL) {
        goto out_error;
    }
    if ((ver->version = strdup(version)) == NULL) {
        goto out_verstr;
    }

    /* allocate dependencies */
    ver->dependencies = NULL;
    for (i = 0; i < n_deps; i++) {
        if ((dep = malloc(sizeof(*dep))) == NULL) {
            goto out_deps;
        }
        if ((dep->constraint = strdup(deps[i])) == NULL) {
            free(dep);
            goto out_deps;
        }
        dep->next = ver->dependencies;
        ver->dependencies = dep;
    }

    /* add version to package */
    ver->next = pkg->available;
    pkg->available = ver;
    return 0;

out_deps:
    for (dep = ver->dependencies; dep != NULL; dep = dep_next) {
        free(dep->constraint);
        dep_next = dep->next;
        free(dep);
    }
    free(ver->version);
out_verstr:
    free(ver);
out_error:
    return -1;
}

int solve_run(struct solve_problem *p)
{
    struct package *pkg;
    struct dependency *dep;

    if (p->solved != 0) {
        fprintf(stderr, "solve_run: already ran on this problem\n");
        return -1;
    }

    for (pkg = p->packages_first; pkg != NULL; pkg = pkg->next) {
        if (p->params.get_pkt(p, pkg->name, p->params.opaque) != 0) {
            goto out_error;
        }

        if (pkg->available == NULL) {
            fprintf(stderr, "solve_run: no versions for package '%s'\n",
                    pkg->name);
            goto out_error;
        }

        /* for now just choose first available version */
        pkg->chosen = pkg->available;

        /* add dependency constraint */
        for (dep = pkg->chosen->dependencies; dep != NULL; dep = dep->next) {
            if (solve_constraint_add(p, dep->constraint) != 0) {
                fprintf(stderr, "solve_run: failed to add dependency "
                        "constraint '%s' for package '%s'\n", dep->constraint,
                        pkg->name);
                goto out_error;
            }
        }
    }

    p->solved = 1;
    return 0;

out_error:
    p->solved = -1;
    return -1;
}

int solve_result_package(struct solve_problem *p, unsigned n,
        const char **pkg_name, const char **pkg_version)
{
    struct package *pkg = p->packages_first;
    unsigned i = 0;

    while (pkg != NULL) {
        /* skip packages without chosen version */
        while (pkg != NULL && pkg->chosen == NULL) {
            pkg = pkg->next;
        }

        if (i == n) {
            break;
        }
        pkg = pkg->next;
        i++;
    }

    if (pkg == NULL) {
        return -1;
    }

    *pkg_name = pkg->name;
    *pkg_version = pkg->chosen->version;
    return 0;
}

static inline struct package *pkg_lookup(struct solve_problem *p,
        const char *name)
{
    struct package *pkg;
    for (pkg = p->packages_first; pkg != NULL; pkg = pkg->next) {
        if (strcmp(pkg->name, name) == 0) {
            return pkg;
        }
    }
    return NULL;
}

static inline struct package *pkg_add(struct solve_problem *p,
        const char *name)
{
    struct package *pkg;

    if ((pkg = pkg_lookup(p, name)) != NULL) {
        return pkg;
    }

    if ((pkg = malloc(sizeof(*pkg))) == NULL) {
        return NULL;
    }

    if ((pkg->name = strdup(name)) == NULL) {
        free(pkg);
        return NULL;
    }

    pkg->available = NULL;
    pkg->chosen = NULL;
    pkg->scanned = 0;
    pkg->next = NULL;
    if (p->packages_last == NULL) {
        p->packages_first = pkg;
    } else {
        p->packages_last->next = pkg;
    }
    p->packages_last = pkg;

    return pkg;
}
