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

/* Command to be executed in environment */
static char **cmd_args;
/* User id to use */
static int env_uid;
/* Group id to use */
static int env_gid;
static int new_uid = 0;
static int new_gid = 0;

static int parse_opts(int argc, char *argv[]);
static int run_child(void);
static int setgroups_deny(void);
static int set_idmap(const char *path, int new_id, int env_id);

int main(int argc, char *argv[])
{
    int ret = EXIT_SUCCESS;
    pid_t pid;

    env_uid = getuid();
    env_gid = getgid();
    if (parse_opts(argc, argv) != 0) {
        fprintf(stderr, "Usage: withenv [OPTIONS] CMD...\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "    -u UID: New UID (default: 0)"
                "\n");
        fprintf(stderr, "    -g GID: New GID (default: 0)"
                "\n");
        return EXIT_FAILURE;
    }

    /* fork child and unshare namespaces */
    pid = fork();
    if (pid == 0) {
        if (unshare(CLONE_NEWUSER | CLONE_NEWIPC | CLONE_NEWNS |
              CLONE_NEWPID) != 0)
        {
          perror("unshare user namespace failed");
          return EXIT_FAILURE;
        }

        if (setgroups_deny() != 0) {
          perror("setgroups deny failed");
          return EXIT_FAILURE;
        }

        if (set_idmap("/proc/self/uid_map", new_uid, env_uid) != 0) {
          perror("map uid failed");
          return EXIT_FAILURE;
        }

        if (set_idmap("/proc/self/gid_map", new_gid, env_gid) != 0) {
          perror("map gid failed");
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

    if (WIFEXITED(ret)) {
        ret = WEXITSTATUS(ret);
    } else {
        ret = -1;
    }

    return ret;

error_fork:
    return EXIT_FAILURE;
}

/* Parse command line options into global variables */
static int parse_opts(int argc, char *argv[])
{
    int c;
    char *end;

    while ((c = getopt(argc, argv, "+u:g:")) != -1) {
        switch (c) {
            case 'u':
                new_uid = strtol(optarg, &end, 10);
                if (*end != 0) {
                    fprintf(stderr, "User option expects a UID integer\n");
                    return -1;
                }
                break;
            case 'g':
                new_gid = strtol(optarg, &end, 10);
                if (*end != 0) {
                    fprintf(stderr, "Group option expects a GID integer\n");
                    return -1;
                }
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
    pid_t pid;
    int ret;

    /* make sure no permissions are masked */
    umask(0);

    /* Now let's launch the command in the new environment.
     * Note: while we don't need the parent process, we fork here so the created
     * process gets PID 1.
     */
    pid = fork();
    if (pid == 0) {
        /* need to mount procfs here so we get a private one */
        if (mount("proc", "/proc", "proc", 0, "") != 0) {
            perror("mounting /proc failed");
            goto error_execv;
        }

        /* reset umask */
        umask(0022);

        /* change uid and gid */
        if (setgid(new_gid) != 0) {
            perror("setgid failed");
            goto error_execv;
        }
        if (setuid(new_uid) != 0) {
            perror("setuid failed");
            goto error_execv;
        }

        /* actually run command now */
        if (execvp(cmd_args[0], cmd_args) != 0) {
            fprintf(stderr, "exec failed\n");
            goto error_execv;
        }
    } else {
        if ((pid = waitpid(pid, &ret, 0)) == -1) {
            perror("waitpid for env command failed");
            goto error_execv;
        }

        if (WIFEXITED(ret)) {
            ret = WEXITSTATUS(ret);
        } else {
            ret = -1;
        }
    }

    return ret;

error_execv:
    /* here we're in the chroot */
    return EXIT_FAILURE;
}

static int setgroups_deny(void)
{
    int fd;
    ssize_t ret, len;
    const char *deny_str = "deny";

    if ((fd = open("/proc/self/setgroups", O_WRONLY)) == -1) {
        perror("setgroups_deny: open failed");
        return -1;
    }

    len = strlen(deny_str);
    ret = write(fd, deny_str, len);
    close(fd);
    if (ret < 0) {
        perror("setgroups_deny: write failed");
        return -1;
    } else if (ret != len) {
        perror("setgroups_deny: partial write");
        return -1;
    }

    return 0;
}

static int set_idmap(const char *path, int new_id, int env_id)
{
    int fd;
    ssize_t ret, len;
    char str[64];

    if (snprintf(str, sizeof(str), "%u %u 1", new_id, env_id) >=
        (ssize_t) sizeof(str))
    {
        perror("set_idmap: buffer too small");
        return -1;
    }
    len = strlen(str);

    if ((fd = open(path, O_WRONLY)) == -1) {
        perror("set_idmap: open failed");
        return -1;
    }

    ret = write(fd, str, len);
    close(fd);
    if (ret < 0) {
        perror("set_idmap: write failed");
        return -1;
    } else if (ret != len) {
        perror("set_idmap: partial write");
        return -1;
    }

    return 0;
}
