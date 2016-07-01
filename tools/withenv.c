#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
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
/* List of packages to be added to environment */
static struct package *packages = NULL;

static int run_child(void);
static int parse_opts(int argc, char *argv[]);
static int mk_subdir(const char *root, const char *s1, mode_t m);
static int mount_subdir(const char *source, const char *root,
        const char *target);
static int mk_pktdir(const char *root, struct package *p);
static int mount_pktdir(const char *root, struct package *p);

int main(int argc, char *argv[])
{
    int ret = EXIT_SUCCESS;
    char templ[] = "/tmp/nstest_XXXXXX";
    pid_t pid;

    if (parse_opts(argc, argv) != 0) {
        fprintf(stderr, "Usage: withenv [OPTIONS] CMD...\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "    -d DIR: Host packet directory (default: /packages)"
                "\n");
        fprintf(stderr, "    -w DIR: Host work directory (mounted under /work)"
                "\n");
        fprintf(stderr, "    -p PKT,VER: Add version of package to env\n");
        return EXIT_FAILURE;
    }

    /* create temporary directory for root mount point */
    root_path = mkdtemp(templ);
    if (root_path == NULL) {
        perror("mkdtemp failed");
        goto error_mkdtemp;
    }

    /* fork child and unshare namespaces */
    pid = fork();
    if (pid == 0) {
        /* in child  */
        if (unshare(CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWPID) != 0) {
            perror("unshare failed\n");
            return EXIT_FAILURE;
        }

        return run_child();
    } else if (pid < 0) {
        perror("fork failed");
        goto error_fork;
    }

    /* wait for child */
    pid = waitpid(pid, &ret, 0);
    if (pid < 0) {
        perror("waitpid failed");
        ret = EXIT_FAILURE;
    }

    /* remove temporary root directory */
    if (rmdir(root_path) != 0) {
        perror("removing temporary root directory failed");
        ret = EXIT_FAILURE;
    }

    printf("Done\n");

    return ret;

error_fork:
    rmdir(root_path);
error_mkdtemp:
    return EXIT_FAILURE;
}

/* Main functionality in forked child (runs in unshared namespaces) */
int run_child(void)
{
    struct package *p;

    /* make sure our mounts don't propagate outside our namespace */
    if (mount("", "/", "dontcare", MS_SLAVE | MS_REC, "") != 0) {
        perror("mount rshared root failed");
        goto error_mountroot;
    }

    /* mount tmpfs as our root directory */
    if (mount("tmpfs", root_path, "tmpfs", MS_NOATIME, "") != 0) {
        perror("mount root tmpfs failed");
        goto error_mountroot;
    }

    /* create subdirectories in root */
    if (mk_subdir(root_path, "dev", 0555) != 0 ||
            mk_subdir(root_path, "proc", 0555) != 0 ||
            mk_subdir(root_path, "sys", 0555) != 0 ||
            mk_subdir(root_path, "tmp", 0777) != 0 ||
            mk_subdir(root_path, "packages", 0755) != 0)
    {
        fprintf(stderr, "creating directory in root failed\n");
        goto error_rootmkdir;
    }

    /* mount bind dev, proc, and sys into new root */
    if (mount_subdir("/dev", root_path, "dev") ||
            mount_subdir("/proc", root_path, "proc") ||
            mount_subdir("/sys", root_path, "sys"))

    {
        fprintf(stderr, "mounting subdirs failed\n");
        goto error_rootmkdir;
    }

    /* if a work directory was specified, create and bind that as well */
    if (workdir != NULL) {
        if (mk_subdir(root_path, "work", 0755) != 0) {
            fprintf(stderr, "creating work directory failed\n");
            goto error_rootmkdir;
        }

        if (mount_subdir(workdir, root_path, "work") != 0) {
            fprintf(stderr, "mounting work dir failed\n");
            goto error_rootmkdir;
        }
    }

    /* mount bind in packages */
    for (p = packages; p != NULL; p = p->next) {
        if (mk_pktdir(root_path, p) != 0) {
            fprintf(stderr, "Create package dir %s/%s failed\n", p->name,
                    p->version);
            goto error_rootmkdir;
        }
        if (mount_pktdir(root_path, p) != 0) {
            fprintf(stderr, "Mount package dir %s/%s failed\n", p->name,
                    p->version);
            goto error_rootmkdir;
        }
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

    /* Now let's launch the command in the new environment */
    if (execvp(cmd_args[0], cmd_args) != 0) {
        fprintf(stderr, "exec failed\n");
        goto error_execv;
    }

    return EXIT_SUCCESS;

error_execv:
    /* here we're in the chroot */
    return EXIT_FAILURE;
error_chroot:
error_rootmkdir:
error_mountroot:
    /* cleanup is easy, when we die, the mounts are garbage collected and the
     * parent removes our root directory in /tmp */
    return EXIT_FAILURE;
}

/* Parse command line options into global variables */
static int parse_opts(int argc, char *argv[])
{
    int c;
    struct package *p;
    char *ver;

    while ((c = getopt(argc, argv, "d:w:p:")) != -1) {
        switch (c) {
            case 'd':
                packagedir = optarg;
                break;
            case 'w':
                workdir = optarg;
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
                printf("Add package: %s version %s\n", p->name, p->version);
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

static int mk_subdir(const char *root, const char *subdir, mode_t m)
{
    char *path;
    int ret;

    if (asprintf(&path, "%s/%s", root, subdir) == -1) {
        return -1;
    }

    ret = mkdir(path, m);

    free(path);
    return ret;
}

static int mount_subdir(const char *source, const char *root,
        const char *target)
{
    char *path;
    int ret;

    if (asprintf(&path, "%s/%s", root, target) == -1) {
        return -1;
    }

    ret = mount(source, path, "", MS_BIND | MS_SLAVE, "");

    if (ret != 0) {
        perror("mount failed");
    }

    free(path);
    return ret;
}

static int mk_pktdir(const char *root, struct package *p)
{
    char *n_path, *v_path;
    int ret;

    if (asprintf(&n_path, "%s/packages/%s", root, p->name) == -1) {
        return -1;
    }

    if (asprintf(&v_path, "%s/%s", n_path, p->version) == -1) {
        free(n_path);
        return -1;
    }

    ret = mkdir(n_path, 0755);
    if (ret == -1 && errno == EEXIST) {
        ret = 0;
    }

    if (ret == 0) {
        ret = mkdir(v_path, 0755);
    }

    free(v_path);
    free(n_path);
    return ret;
}

static int mount_pktdir(const char *root, struct package *p)
{
    char *src_path = NULL, *dst_path = NULL;
    int ret;

    if (asprintf(&dst_path, "%s/packages/%s/%s", root, p->name, p->version)
            == -1)
    {
        return -1;
    }

    if (asprintf(&src_path, "%s/%s/%s", packagedir, p->name, p->version)
            == -1)
    {
        free(dst_path);
        return -1;
    }

    ret = mount(src_path, dst_path, "", MS_BIND | MS_SLAVE, "");

    free(src_path);
    free(dst_path);
    return ret;

}
