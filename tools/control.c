#include "control.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

enum parse_state {
    STATE_NONE,
    STATE_IN_DEPS_RUN,
    STATE_IN_DEPS_BUILD,
    STATE_IN_SOURCES,
};

static char *trim(char *buf);
static inline int has_prefix(const char *string, const char *prefix);

int control_parse(const char *path, struct control **ctrl)
{
    FILE *f;
    unsigned line = 0;
    char *linebuf, *l;
    size_t lb_size;
    ssize_t bs;
    enum parse_state state = STATE_NONE;
    struct control *c;
    const char *msg = NULL;
    struct control_dependency *cd, *cdd;
    struct control_source *cs, *css;


    if ((f = fopen(path, "r")) == NULL) {
        perror("control_parse: fopen failed");
        return -1;
    }

    if ((c = calloc(1, sizeof(*c))) == NULL) {
        perror("control_parse: malloc failed");
        goto error_malloc;
    }
    *ctrl = c;

    lb_size = 128;
    if ((linebuf = malloc(lb_size)) == NULL) {
        perror("control_parse: malloc failed");
        goto error_linebufalloc;
    }

    while ((bs = getline(&linebuf, &lb_size, f)) != -1) {
        line++;
        l = linebuf;

        if (state == STATE_NONE || !isspace(l[0])) {
            /* not currently parsing a field or no whitespace at beginnning */
            state = STATE_NONE;

            /* skip over empty lines */
            if (l[0] == '\n' && l[1] == 0) {
                continue;
            }

            if (has_prefix(l, "Package:")) {
                l = trim(l + strlen("Package:"));
                if (c->package != NULL) {
                    msg = "Package field already set";
                    goto error_parse;
                }
                if ((c->package = strdup(l)) == NULL) {
                    msg = "strdup failed";
                    goto error_parse;
                }
                continue;
            } else if (has_prefix(l, "Version:")) {
                l = trim(l + strlen("Version:"));
                if (c->version != NULL) {
                    msg = "Version field already set";
                    goto error_parse;
                }
                if ((c->version = strdup(l)) == NULL) {
                    msg = "strdup failed";
                    goto error_parse;
                }
                continue;
            } else if (has_prefix(l, "Depends-Run:")) {
                l += strlen("Depends-Run:");
                state = STATE_IN_DEPS_RUN;
                if (c->run_depend != NULL) {
                    msg = "Runtime dependencies field already set";
                    goto error_parse;
                }
            } else if (has_prefix(l, "Depends-Build:")) {
                l += strlen("Depends-Build:");
                state = STATE_IN_DEPS_BUILD;
                if (c->run_depend != NULL) {
                    msg = "Build dependencies field already set";
                    goto error_parse;
                }
            } else if (has_prefix(l, "Sources:")) {
                l += strlen("Depends-Build:");
                state = STATE_IN_SOURCES;
                if (c->run_depend != NULL) {
                    msg = "Build dependencies field already set";
                    goto error_parse;
                }
            } else {
                msg = "Unknown field specified";
                goto error_parse;
            }
        }
        if (state == STATE_IN_DEPS_RUN) {
            l = trim(l);
            if (l[0] == 0) {
                continue;
            }

            if ((cd = malloc(sizeof(*cd))) == NULL) {
                msg = "malloc failed";
                goto error_parse;
            }
            if ((cd->package = strdup(l)) == NULL) {
                msg = "strdup failed";
                goto error_parse;
            }

            /* add to linked list of dependencies */
            cd->next = NULL;
            if (c->run_depend == NULL) {
                c->run_depend = cd;
            } else {
                for (cdd = c->run_depend; cdd->next != NULL; cdd = cdd->next);
                cdd->next = cd;
            }
        } else if (state == STATE_IN_DEPS_BUILD) {
            l = trim(l);
            if (l[0] == 0) {
                continue;
            }

            if ((cd = malloc(sizeof(*cd))) == NULL) {
                msg = "malloc failed";
                goto error_parse;
            }
            if ((cd->package = strdup(l)) == NULL) {
                msg = "strdup failed";
                goto error_parse;
            }

            /* add to linked list of dependencies */
            cd->next = NULL;
            if (c->build_depend == NULL) {
                c->build_depend = cd;
            } else {
                for (cdd = c->build_depend; cdd->next != NULL; cdd = cdd->next);
                cdd->next = cd;
            }
        } else if (state == STATE_IN_SOURCES) {
            l = trim(l);
            if (l[0] == 0) {
                continue;
            }

            if ((cs = malloc(sizeof(*cs))) == NULL) {
                msg = "malloc failed";
                goto error_parse;
            }
            if ((cs->source = strdup(l)) == NULL) {
                msg = "strdup failed";
                goto error_parse;
            }

            /* add to linked list of dependencies */
            cs->next = NULL;
            if (c->sources == NULL) {
                c->sources = cs;
            } else {
                for (css = c->sources; css->next != NULL; css = css->next);
                css->next = cs;
            }
        }
    }

    if (ferror(f)) {
        perror("control_parse: reading line failed");
        goto error_parse;
    }

    if (c->package == NULL) {
        msg = "No package name provided";
        goto error_parse;
    }

    if (c->version == NULL) {
        msg = "No package version provided";
        goto error_parse;
    }

    if (c->sources == NULL) {
        msg = "No package sources provided";
        goto error_parse;
    }


    free(linebuf);
    fclose(f);
    return 0;


error_parse:
    if (msg != NULL) {
        fprintf(stderr, "%s:%u %s\n", path, line, msg);
    }
    free(linebuf);
error_linebufalloc:
    control_destroy(c);
error_malloc:
    fclose(f);
    return -1;
}

void control_destroy(struct control *ctrl)
{
    struct control_dependency *cd, *cdd;
    struct control_source *cs, *css;

    for (cd = ctrl->build_depend; cd != NULL; ) {
        cdd = cd;
        cd = cd->next;
        free(cdd->package);
        free(cdd);
    }
    for (cd = ctrl->run_depend; cd != NULL; ) {
        cdd = cd;
        cd = cd->next;
        free(cdd->package);
        free(cdd);
    }
    for (cs = ctrl->sources; cs != NULL; ) {
        css = cs;
        cs = cs->next;
        free(css->source);
        free(css);
    }

    free(ctrl->package);
    free(ctrl->version);
    free(ctrl);
}

/* Trims all space characters from begining and end of the string.
 * Modifies the * string in place and returns pointer to new beginning.
 */
static char *trim(char *buf)
{
    size_t len;
    char *end;

    /* skip space at the beginning */
    for (; *buf != 0 && isspace(*buf); buf++);

    /* empty string now */
    len = strlen(buf);
    if (len == 0) {
        return buf;
    }

    /* skip space at the end */
    end = buf + len - 1;
    while (end >= buf && isspace(*end)) {
        *end = 0;
        end--;
    }
    return buf;
}

static inline int has_prefix(const char *string, const char *prefix)
{
    return strncmp(string, prefix, strlen(prefix)) == 0;
}
