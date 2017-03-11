#include "control.h"
#include "controlparse.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

static int parse_deplist(struct controlparse_field *pf,
        struct control_dependency **pcd);
static int parse_sourcelist(struct controlparse_field *pf,
        struct control_source **pcs);
static int parse_singleline(struct controlparse_field *pf, char **ps);

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
    struct control_dependency **first_cd;
    struct controlparse cp;
    struct controlparse_para *pp;
    struct controlparse_field *pf;

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
            if (parse_deplist(pf, first_cd) != 0)
                goto out_malloc;
        } else if (!strcmp(pf->name, "Sources")) {
            if (parse_sourcelist(pf, &c->sources) != 0)
                goto out_malloc;
        } else if (!strcmp(pf->name, "Package")) {
            if (parse_singleline(pf, &c->package) != 0)
                goto out_malloc;
        } else if (!strcmp(pf->name, "Version")) {
            if (parse_singleline(pf, &c->version) != 0)
                goto out_malloc;
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

static int parse_deplist(struct controlparse_field *pf,
        struct control_dependency **pcd)
{
    struct control_dependency *cd, *cdd;
    struct controlparse_line *pl;

    if (*pcd != NULL) {
        fprintf(stderr, "parse_deplist: %s set repeatedly\n", pf->name);
        return -1;
    }

    /* parse run-time and build-time dependencies */
    for (pl = pf->line_first; pl != NULL; pl = pl->next) {
        if (!*pl->line)
            continue;

        /* allocate dependency struct */
        if ((cd = malloc(sizeof(*cd))) == NULL ||
            (cd->package = strdup(pl->line)) == NULL)
        {
            free(cd);
            perror("parse_deplist: malloc source failed");
            return -1;
        }

        /* add to deps linked list */
        cd->next = NULL;
        if (*pcd == NULL) {
            *pcd = cd;
        } else {
            for (cdd = *pcd; cdd->next != NULL; cdd = cdd->next);
            cdd->next = cd;
        }
    }

    return 0;
}

static int parse_sourcelist(struct controlparse_field *pf,
        struct control_source **pcs)
{
    struct control_source *cs, *css;
    struct controlparse_line *pl;

    if (*pcs != NULL) {
        fprintf(stderr, "parse_sourcelist: %s set repeatedly\n", pf->name);
        return -1;
    }

    for (pl = pf->line_first; pl != NULL; pl = pl->next) {
        if (!*pl->line)
            continue;

        /* allocate source struct */
        if ((cs = malloc(sizeof(*cs))) == NULL ||
            (cs->source = strdup(pl->line)) == NULL)
        {
            free(cs);
            perror("parse_sourcelist: malloc source failed");
            return -1;
        }

        /* add to sources linked list */
        cs->next = NULL;
        if (*pcs == NULL) {
            *pcs = cs;
        } else {
            for (css = *pcs; css->next != NULL; css = css->next);
            css->next = cs;
        }
    }

    return 0;
}

static int parse_singleline(struct controlparse_field *pf, char **ps)
{
    if (*ps != NULL) {
        fprintf(stderr, "parse_singleline: %s has already been set\n",
                pf->name);
        return -1;
    }
    if (pf->line_first == NULL || pf->line_first->next != NULL ||
            !*pf->line_first->line)
    {
        fprintf(stderr, "parse_singleline: %s needs to be "
                "non-empty single line\n", pf->name);
        return -1;
    }
    if ((*ps = strdup(pf->line_first->line)) == NULL) {
        perror("parse_singleline: strdup failed");
        return -1;
    }

    return 0;
}
