#ifndef REPO_H_
#define REPO_H_

#include "control.h"

struct repo;

struct repo_package {
    char *name;
    struct repo_version *versions;
    struct repo_package *next;
};

struct repo_version {
    char *version;
    struct control *control;
    struct repo_version *next;
};

struct repo *repo_init(const char *path);
struct repo_package *repo_package_get(struct repo *repo, const char *name);
void repo_destroy(struct repo *repo);

#endif /* ndef REPO_H_*/
