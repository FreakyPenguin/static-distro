#define _GNU_SOURCE
#include <sched.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>

struct package {
    const char *name;
    const char *version;
    struct package *next;
};

/* Path to temporary directory used for the root mount */
static char *root_path;

/* Command to be executed in environment */
static char **cmd_args;
/* Path to work directory on home (if set) */
static const char *workdir = NULL;
/* Path to packages dir on host */
static const char *packagedir = "/packages";
/* Working directory to chdir to before running command */
static const char *new_cwdir = NULL;
/* List of packages to be added to environment */
static struct package *packages = NULL;

static int parse_opts(int argc, char *argv[]);
static int run_child(void);
static int bind_packages(void);
static int link_packages(void);
static int link_package_tree(struct package *p, const char *tree, int dst_fd);
static int create_usr_links(void);
static int link_tree(int src_fd, char *src_path, size_t len, int dst_fd);
static char *path_parts(const char **parts);
static int mkdir_parts(const char **parts, mode_t m);
static int mountbind_parts(const char **src, const char **target, int ro);

int main(int argc, char *argv[])
{
    char templ[] = "/tmp/nstest_XXXXXX";

    if (parse_opts(argc, argv) != 0) {
        fprintf(stderr, "Usage: withenv [OPTIONS] CMD...\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "    -d DIR: Host package directory (default: /packages)"
                "\n");
        fprintf(stderr, "    -w DIR: Host work directory (mounted under /work)"
                "\n");
        fprintf(stderr, "    -c DIR: Chdir to DIR before running CMD\n");
        fprintf(stderr, "    -p PKT,VER: Add version of package to env\n");
        return EXIT_FAILURE;
    }

    /* create temporary directory for root mount point */
    root_path = mkdtemp(templ);
    if (root_path == NULL) {
        perror("mkdtemp failed");
        return EXIT_FAILURE;
    }

    return run_child();
}

/* Parse command line options into global variables */
static int parse_opts(int argc, char *argv[])
{
    int c;
    struct package *p;
    char *ver;

    while ((c = getopt(argc, argv, "+d:w:p:c:")) != -1) {
        switch (c) {
            case 'd':
                packagedir = optarg;
                break;
            case 'w':
                workdir = optarg;
                break;
            case 'c':
                new_cwdir = optarg;
                break;
            case 'p':
                if ((ver = strchr(optarg, ',')) == NULL) {
                    fprintf(stderr, "Packet option requires version to be "
                            "specified separated with a comma.\n");
                    return -1;
                }
                if ((p = malloc(sizeof(*p))) == NULL) {
                    fprintf(stderr, "malloc package struct failed\n");
                    return -1;
                }
                *ver = 0;
                p->name = optarg;
                p->version = ver + 1;
                p->next = packages;
                packages = p;
                break;
            case '?':
                fprintf(stderr, "Unknown option: %c\n", optopt);
                return -1;
                break;

            default:
                abort();
        }
    }

    if (optind  >= argc) {
        fprintf(stderr, "No command specfied\n");
        return -1;
    }

    cmd_args = argv + optind;
    return 0;
}

/* Main functionality in forked child (runs in unshared namespaces) */
int run_child(void)
{
    int ret;

    /* make sure no permissions are masked */
    umask(0);

    /* mount tmpfs as our root directory */
    if (mount("tmpfs", root_path, "tmpfs", MS_NOATIME, "") != 0) {
        perror("mount root tmpfs failed");
        goto error_mountroot;
    }

    /* create subdirectories in root */
    const char *root_dev[] = { root_path, "dev", NULL };
    const char *root_proc[] = { root_path, "proc", NULL };
    const char *root_sys[] = { root_path, "sys", NULL };
    const char *root_tmp[] = { root_path, "tmp", NULL };
    const char *root_packages[] = { root_path, "packages", NULL };
    if (mkdir_parts(root_dev, 0555) != 0 ||
            mkdir_parts(root_proc, 0555) != 0 ||
            mkdir_parts(root_sys, 0555) != 0 ||
            mkdir_parts(root_tmp, 0777) != 0 ||
            mkdir_parts(root_packages, 0755) != 0)
    {
        perror("creating directory in root failed");
        goto error_rootmkdir;
    }

    /* mount bind dev, proc, and sys into new root */
    const char *host_dev[] = { "/dev", NULL };
    const char *host_sys[] = { "/sys", NULL };
    if (mountbind_parts(host_dev, root_dev, 0) ||
            mountbind_parts(host_sys, root_sys, 0))
    {
        perror("mounting subdirs failed");
        goto error_rootmkdir;
    }

    /* if a work directory was specified, create and bind that as well */
    if (workdir != NULL) {
        const char *root_work[] = { root_path, "work", NULL };
        if (mkdir_parts(root_work, 0755) != 0) {
            perror("creating work directory failed");
            goto error_rootmkdir;
        }

        const char *host_work[] = { workdir, NULL };
        if (mountbind_parts(host_work, root_work, 0) != 0) {
            perror("mounting work dir failed");
            goto error_rootmkdir;
        }
    }

    /* mount bind in packages */
    if (bind_packages() != 0) {
        goto error_installpackages;
    }

    /* chroot into our new environment */
    if (chroot(root_path) != 0) {
        fprintf(stderr, "chroot failed\n");
        goto error_chroot;
    }

    /* make sure chdir is "/" */
    if (chdir("/") != 0) {
        fprintf(stderr, "chdir failed\n");
        goto error_execv;
    }

    /* link packages */
    if (link_packages() != 0) {
        goto error_execv;
    }

    /* need to mount procfs here so we get a private one */
    if (mount("proc", "/proc", "proc", 0, "") != 0) {
        perror("mounting /proc failed");
        goto error_execv;
    }

    /* create usr links */
    if (create_usr_links() != 0) {
        fprintf(stderr, "creating /usr links failed\n");
        goto error_execv;
    }

    /* reset umask */
    umask(0022);

    /* change work directory if required */
    if (new_cwdir != NULL && chdir(new_cwdir) != 0) {
        perror("chdir failed");
        goto error_execv;
    }

    /* actually run command now */
    if (execvp(cmd_args[0], cmd_args) != 0) {
        fprintf(stderr, "exec failed\n");
        goto error_execv;
    }

    return ret;

error_execv:
    /* here we're in the chroot */
    return EXIT_FAILURE;
error_chroot:
error_installpackages:
error_rootmkdir:
error_mountroot:
    /* cleanup is easy, when we die, the mounts are garbage collected and the
     * parent removes our root directory in /tmp */
    return EXIT_FAILURE;
}

/* mount bind package directories from host */
static int bind_packages(void)
{
    struct package *p;

    for (p = packages; p != NULL; p = p->next) {
        const char *parts[5] = { root_path, "packages", p->name, NULL };
        const char *host_parts[4] = { packagedir, p->name, p->version, NULL };

        /* create /packages/name */
        if (mkdir_parts(parts, 0555) != 0 && errno != EEXIST) {
            perror("mkdir failed");
            fprintf(stderr, "Creating /packages/%s failed\n", p->name);
            return -1;
        }

        /* create /packages/name/version */
        parts[3] = p->version;
        parts[4] = NULL;
        if (mkdir_parts(parts, 0555) != 0) {
            perror("mkdir failed");
            fprintf(stderr, "Creating /packages/%s/%s failed\n", p->name,
                    p->version);
            return -1;
        }

        if (mountbind_parts(host_parts, parts, 1) != 0) {
            perror("mount failed");
            fprintf(stderr, "Mount package dir %s/%s failed\n", p->name,
                    p->version);
            return -1;
        }
    }

    return 0;
}

/* link package contents to /bin, /lib, /include */
static int link_packages(void)
{
    struct package *p;
    int bin_fd, lib_fd, inc_fd;
    int ret = 0;

    if (mkdir("/bin", 0755) != 0 || mkdir("/lib", 0755) != 0 ||
            mkdir("/include", 0755) != 0)
    {
        perror("mkdir /bin, /lib, or /include failed");
        return -1;
    }

    if ((bin_fd = open("/bin", O_RDONLY | O_DIRECTORY)) == -1 ||
            (lib_fd = open("/lib", O_RDONLY | O_DIRECTORY)) == -1 ||
            (inc_fd = open("/include", O_RDONLY | O_DIRECTORY)) == -1)
    {
        perror("Opening /bin, /lib, or /include failed");
        return -1;
    }

    for (p = packages; p != NULL; p = p->next) {
        if (link_package_tree(p, "bin", bin_fd) != 0 ||
                link_package_tree(p, "lib", lib_fd) != 0 ||
                link_package_tree(p, "include", inc_fd) != 0)
        {
            fprintf(stderr, "Linking packet contents %s/%s/ failed\n",
                    p->name, p->version);
            ret = -1;
            goto exit;
        }
    }

exit:
    close(bin_fd);
    close(lib_fd);
    close(inc_fd);
    return ret;
}

/* link entries in packagedir/tree to directory pointed to by dst_fd */
static int link_package_tree(struct package *p, const char *tree, int dst_fd)
{
    int ret, pkt_fd;
    const char *parts[5] = { "/packages", p->name, p->version, tree, NULL };
    char *path;

    if ((path = path_parts(parts)) == NULL) {
        perror("link_package_tree: path_parts failed");
        return -1;
    }

    pkt_fd = open(path, O_RDONLY | O_DIRECTORY);
    if (pkt_fd == -1 && errno == ENOENT) {
        /* directory does not exist -> ignore */
        free(path);
        return 0;
    } else if (pkt_fd == -1) {
        perror("open failed");
        free(path);
        fprintf(stderr, "Opening %s/%s/%s failed\n", p->name, p->version, tree);
        return -1;
    }

    ret = link_tree(pkt_fd, path, strlen(path), dst_fd);

    close(pkt_fd);
    free(path);
    return ret;
}

/* create /usr, and link subdirs bin, lib, include to / */
static int create_usr_links(void)
{
    if (mkdir("/usr", 0755) != 0) {
        perror("mkdir failed");
        return -1;
    }

    if (symlink("/bin", "/usr/bin") != 0 ||
            symlink("/lib", "/usr/lib") != 0 ||
            symlink("/include", "/usr/include") != 0)
    {
        perror("symlink failed");
        return -1;
    }

    return 0;
}

/* traverse src directory (both path and fd, need to match) and add symlinks for
 * entries to dst directory. For entries that are directories, create directory
 * in destination directory and then recurse. len has to match strlen(src_path).
 */
static int link_tree(int src_fd, char *src_path, size_t len, int dst_fd)
{
    DIR *src_d;
    int ret = 0, fd, nsrc_fd, ndst_fd, x;
    struct dirent *de;
    struct stat st;
    size_t elen;

    if ((fd = dup(src_fd)) == -1) {
        perror("dup failed");
        fprintf(stderr, "link_tree: duplicating source fd failed on %s\n",
                src_path);
        return -1;
    }

    if ((src_d = fdopendir(fd)) == NULL) {
        close(fd);
        perror("fdopendir failed");
        fprintf(stderr, "fdopendir failed on %s\n", src_path);
        return -1;
    }

    errno = 0;
    while ((de = readdir(src_d)) != NULL) {
        /* skip hidden files */
        if (de->d_name[0] == '.') {
            errno = 0;
            continue;
        }

        /* stat to figure out if it's a directory */
        if (fstatat(src_fd, de->d_name, &st, 0) != 0) {
            perror("fstatat failed");
            fprintf(stderr, "link_tree: fstatat failed on entry %s/%s\n",
                    src_path, de->d_name);
            ret = -1;
            goto exit;
        }

        /* check that we can build up src path */
        elen = strlen(de->d_name);
        if (len + 1 + elen > PATH_MAX) {
            fprintf(stderr, "link_tree: path too long on entry %s/%s\n",
                    src_path, de->d_name);
            ret = -1;
            goto exit;

        }

        /* append to src_path */
        src_path[len] = '/';
        memcpy(src_path + len + 1, de->d_name, elen + 1);

        if (S_ISDIR(st.st_mode)) {
            /* is a directory, create dir if necessary then recurse */

            /* check whether destination directory exists, otherwise mkdir */
            if ((x = fstatat(dst_fd, de->d_name, &st, 0)) != 0 &&
                    errno != ENOENT)
            {
                perror("fstatat failed");
                fprintf(stderr, "link_tree: fstatat failed on dst entry "
                        "%s\n", src_path);
                ret = -1;
                goto exit;
            }
            if (x == 0 && !S_ISDIR(st.st_mode)) {
                /* destination entry exists but is not directory */
                fprintf(stderr, "link_tree: destination entry for dir %s exists"
                        " but is not directory\n", src_path);
                ret = -1;
                goto exit;
            } else if (x != 0 && mkdirat(dst_fd, de->d_name, 0755) != 0) {
                perror("mkdirat failed");
                fprintf(stderr, "link_tree: creating destination dir for %s "
                        "failed\n", src_path);
                ret = -1;
                goto exit;
            }

            /* get fd for src */
            if ((nsrc_fd = openat(src_fd, de->d_name, O_RDONLY | O_DIRECTORY))
                    == -1)
            {
                perror("openat failed");
                fprintf(stderr, "link_tree: openat of src %s failed\n",
                        src_path);
                ret = -1;
                goto exit;
            }

            /* get fd for dst */
            if ((ndst_fd = openat(dst_fd, de->d_name, O_RDONLY | O_DIRECTORY))
                    == -1)
            {
                perror("openat failed");
                close(nsrc_fd);
                fprintf(stderr, "link_tree: openat of dst for %s failed\n",
                        src_path);
                ret = -1;
                goto exit;
            }

            /* recurse to subtree */
            x = link_tree(nsrc_fd, src_path, len + 1 + elen, ndst_fd);
            close(nsrc_fd);
            close(ndst_fd);
            if (x != 0) {
                fprintf(stderr, "link_tree: linking subtree at %s failed\n",
                        src_path);
                ret = -1;
                goto exit;
            }

        } else {
            /* just create a symlink */
            if (symlinkat(src_path, dst_fd, de->d_name) != 0) {
                perror("symlinkat failed");
                fprintf(stderr, "link_tree: linking entry %s failed\n",
                        src_path);
                ret = -1;
                goto exit;
            }
        }
        errno = 0;
    }

    if (errno != 0) {
        perror("readdir failed");
        ret = -1;
    }

exit:
    closedir(src_d);
    src_path[len] = 0;
    return ret;
}

/* allocate and build path by concatenating the parts, separated by slash. The
 * returned buffer will always have size PATH_MAX + 1. */
static char *path_parts(const char **parts)
{
    char *path, *cur;
    size_t avail = PATH_MAX, len;
    size_t prev;

    if ((path = malloc(avail + 1)) == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    cur = path;

    prev = 0;
    while (*parts != NULL) {
        len = strlen(*parts);
        if (len + prev > avail) {
            free(path);
            errno = ENAMETOOLONG;
            return NULL;
        }

        /* separate parts by slash */
        if (prev) {
            *cur = '/';
            cur++;
        } else {
            prev = 1;
        }

        memcpy(cur, *parts, len);
        cur += len;

        parts++;
    }

    *cur = '\0';
    return path;
}

/* Assemble parts of path by concatenating them with /, then use path for
 * mkdir. */
static int mkdir_parts(const char **parts, mode_t m)
{
    char *path;
    int ret;

    if ((path = path_parts(parts)) == NULL) {
        return -1;
    }

    ret = mkdir(path, m);

    free(path);
    return ret;
}

/* Assemble parts of both paths by concatenating them with /, then use the paths
 * for bind mounting from src to target. */
static int mountbind_parts(const char **src, const char **target, int ro)
{
    char *src_path, *target_path;
    int ret;

    if ((src_path = path_parts(src)) == NULL ||
            (target_path = path_parts(target)) == NULL)
    {
        free(src_path);
        return -1;
    }

    ret = mount(src_path, target_path, "", MS_BIND | MS_REC, "");
    if (ret != 0) {
      perror("mount failed");
    }
    /* remount read-only if needed */
    if (ro != 0 && ret == 0) {
        ret = mount("", target_path, "", MS_REMOUNT | MS_BIND | MS_RDONLY, "");
    }

    free(target_path);
    free(src_path);
    return ret;
}
