#include "control.h"
#include "controlparse.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

enum parse_state {
    STATE_NONE,
    STATE_IN_DEPS_RUN,
    STATE_IN_DEPS_BUILD,
    STATE_IN_SOURCES,
};

int control_parse(const char *path, struct control **ctrl)
{
    int fd, ret;

    if ((fd = open(path, O_RDONLY)) < 0) {
        perror("control_parse: open failed");
        return -1;
    }

    ret = control_parsefd(fd, ctrl);
    close(fd);
    return ret;
}

int control_parsefd(int fd, struct control **ctrl)
{
    int dep_run, dep_build;
    struct control *c;
    struct control_dependency *cd, *cdd, **first_cd;
    struct control_source *cs, *css;
    struct controlparse cp;
    struct controlparse_para *pp;
    struct controlparse_field *pf;
    struct controlparse_line *pl;

    if (controlparse_init(&cp))
        return -1;
    if (controlparse_parsefd(&cp, fd))
        goto out_err;

    if ((c = calloc(1, sizeof(*c))) == NULL) {
        perror("control_parsefd: malloc failed");
        goto out_err;
    }
    *ctrl = c;

    pp = cp.para_first;
    if (pp == NULL || pp->next != NULL) {
        fprintf(stderr, "control_parsefd: file must have exactly one "
                "paragraph\n");
        goto out_malloc;
    }

    for (pf = pp->field_first; pf != NULL; pf = pf->next) {
        dep_run = !strcmp(pf->name, "Depends-Run");
        dep_build = !strcmp(pf->name, "Depends-Build");
        if (dep_run || dep_build) {
            /* parse run-time and build-time dependencies */
            first_cd = (dep_run ? &c->run_depend : &c->build_depend);
            if (*first_cd != NULL) {
                fprintf(stderr, "control_parsefd: %s set repeatedly\n",
                        pf->name);
                goto out_malloc;
            }
            for (pl = pf->line_first; pl != NULL; pl = pl->next) {
                if (!*pl->line)
                    continue;

                /* allocate dependency struct */
                if ((cd = malloc(sizeof(*cd))) == NULL ||
                    (cd->package = strdup(pl->line)) == NULL)
                {
                    free(cd);
                    perror("control_parsefd: malloc source failed");
                    goto out_malloc;
                }

                /* add to deps linked list */
                cd->next = NULL;
                if (*first_cd == NULL) {
                    *first_cd = cd;
                } else {
                    for (cdd = *first_cd; cdd->next != NULL; cdd = cdd->next);
                    cdd->next = cd;
                }
            }
        } else if (!strcmp(pf->name, "Sources")) {
            /* parse list of source files */
            if (c->sources != NULL) {
                fprintf(stderr, "control_parsefd: Sources set repeatedly\n");
                goto out_malloc;
            }
            for (pl = pf->line_first; pl != NULL; pl = pl->next) {
                if (!*pl->line)
                    continue;

                /* allocate source struct */
                if ((cs = malloc(sizeof(*cs))) == NULL ||
                    (cs->source = strdup(pl->line)) == NULL)
                {
                    free(cs);
                    perror("control_parsefd: malloc source failed");
                    goto out_malloc;
                }

                /* add to sources linked list */
                cs->next = NULL;
                if (c->sources == NULL) {
                    c->sources = cs;
                } else {
                    for (css = c->sources; css->next != NULL; css = css->next);
                    css->next = cs;
                }
            }
        } else if (!strcmp(pf->name, "Package")) {
            if (c->package != NULL) {
                fprintf(stderr, "control_parsefd: Package already set\n");
                goto out_malloc;
            }
            if (pf->line_first == NULL || pf->line_first->next != NULL ||
                    !*pf->line_first->line)
            {
                fprintf(stderr, "control_parsefd: Package needs to be "
                        "non-empty single line\n");
                goto out_malloc;
            }
            if ((c->package = strdup(pf->line_first->line)) == NULL) {
                perror("control_parsefd: strdup failed");
                goto out_malloc;
            }
        } else if (!strcmp(pf->name, "Version")) {
            if (c->version != NULL) {
                fprintf(stderr, "control_parsefd: Version already set\n");
                goto out_malloc;
            }
            if (pf->line_first == NULL || pf->line_first->next != NULL ||
                    !*pf->line_first->line)
            {
                fprintf(stderr, "control_parsefd: Version needs to be "
                        "non-empty single line\n");
                goto out_malloc;
            }
            if ((c->version = strdup(pf->line_first->line)) == NULL) {
                perror("control_parsefd: strdup failed");
                goto out_malloc;
            }
        }
    }

    controlparse_destroy(&cp);
    return 0;

out_malloc:
    control_destroy(c);
out_err:
    controlparse_destroy(&cp);
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
