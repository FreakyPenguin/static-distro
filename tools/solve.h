#ifndef SOLVE_H_
#define SOLVE_H_

struct solve_problem;

struct solve_params {
    int (*get_pkt)(struct solve_problem *p, const char *name, void *opaque);
    void *opaque;
};

struct solve_packet {
    char *name;
    char *version;
};

int solve_init(const struct solve_params *params, struct solve_problem **p);
void solve_destroy(struct solve_problem *p);
int solve_constraint_add(struct solve_problem *p, const char *constraint);
int solve_package_version_add(struct solve_problem *p, const char *name,
        const char *version, unsigned n_deps, const char **deps);
int solve_run(struct solve_problem *p);
int solve_result_package(struct solve_problem *p, unsigned n,
        const char **pkg_name, const char **pkg_version);

#endif /* ndef SOLVE_H_ */
