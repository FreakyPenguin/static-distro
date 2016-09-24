#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "repo.h"

struct repo {
    int repofd;
    struct repo_package *packages;
};

static struct repo_package *package_lookup(struct repo *repo, const char *pkg);
static void package_destroy(struct repo_package *pkg);

struct repo *repo_init(const char *path)
{
    struct repo *r;

    if ((r = malloc(sizeof(*r))) == NULL) {
        return NULL;
    }
    r->packages = NULL;

    if ((r->repofd = open(path, O_RDONLY | O_DIRECTORY)) == -1) {
        perror("repo_init: openening repository directory failed");
        free(r);
        return NULL;
    }

    return r;
}

struct repo_package *repo_package_get(struct repo *repo, const char *name)
{
    struct repo_package *pkg;
    struct repo_version *ver;
    int dfd, vfd, cfd, r;
    DIR *pkg_d;
    struct dirent *de;
    struct control *c;


    if ((pkg = package_lookup(repo, name)) != NULL) {
        return pkg;
    }

    if ((pkg = malloc(sizeof(*pkg))) == NULL) {
        return NULL;
    }
    if ((pkg->name = strdup(name)) == NULL) {
        free(pkg);
        return NULL;
    }
    pkg->versions = NULL;


    if ((dfd = openat(repo->repofd, name, O_RDONLY | O_DIRECTORY)) == -1) {
        goto out_error;
    }

    if ((pkg_d = fdopendir(dfd)) == NULL) {
        close(dfd);
        perror("repo_package_get: fdopendir failed");
        goto out_error;
    }

    while ((de = readdir(pkg_d)) != NULL) {
        if (de->d_name[0] == '.') {
            continue;
        }

        /* open version directory */
        if ((vfd = openat(dfd, de->d_name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW))
                == -1)
        {
            fprintf(stderr, "repo_package_get: openat failed (%s,%s), skipping "
                    "version\n", name, de->d_name);
            continue;
        }

        /* open control file */
        cfd = openat(vfd, "control", O_RDONLY);
        close(vfd);
        if (cfd == -1) {
            fprintf(stderr, "repo_package_get: openat control failed (%s,%s), "
                    "skipping version\n", name, de->d_name);
            continue;
        }

        /* parse control file */
        r = control_parsefd(cfd, &c);
        close(cfd);
        if (r != 0) {
            fprintf(stderr, "repo_package_get: parsing control failed (%s,%s), "
                    "skipping version\n", name, de->d_name);
            continue;
        }

        /* add version */
        if ((ver = malloc(sizeof(*ver))) == NULL) {
            fprintf(stderr, "repo_package_get: malloc failed (%s,%s), "
                    "aborting\n", name, de->d_name);
            control_destroy(c);
            goto out_closedir;
        }

        if ((ver->version = strdup(de->d_name)) == NULL) {
            fprintf(stderr, "repo_package_get: strdup failed (%s,%s), "
                    "aborting\n", name, de->d_name);
            free(ver);
            control_destroy(c);
            goto out_closedir;
        }

        ver->control = c;
        ver->next = pkg->versions;
        pkg->versions = ver;
    }
    closedir(pkg_d);

    /* add package */
    pkg->next = repo->packages;
    repo->packages = pkg;

    return pkg;

out_closedir:
    closedir(pkg_d);
out_error:
    package_destroy(pkg);
    return NULL;
}

void repo_destroy(struct repo *repo)
{
    struct repo_package *pkg, *pkg_next;

    for (pkg = repo->packages; pkg != NULL; pkg = pkg_next) {
        pkg_next = pkg->next;
        package_destroy(pkg);
    }

    close(repo->repofd);
    free(repo);
}

static struct repo_package *package_lookup(struct repo *repo, const char *name)
{
    struct repo_package *pkg;

    for (pkg = repo->packages; pkg != NULL; pkg = pkg->next) {
        if (strcmp(pkg->name, name) == 0) {
            return pkg;
        }
    }
    return NULL;
}

static void package_destroy(struct repo_package *pkg)
{
    struct repo_version *ver, *ver_next;

    for (ver = pkg->versions; ver != NULL; ver = ver_next) {
        ver_next = ver->next;
        free(ver->version);
        free(ver);
    }
    free(pkg->name);
    free(pkg);
}
