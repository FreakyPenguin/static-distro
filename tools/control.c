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
        if (!strcmp(pf->name, "Depends-Run")) {
            /* parse run-time and build-time dependencies */
            if (parse_deplist(pf, &c->run_depend) != 0)
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
        } else {
            fprintf(stderr, "control_parsefd: Unexpected field: '%s'\n",
                pf->name);
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

int source_control_parsefd(int fd, struct source_control **sc)
{
    struct source_control *c;
    struct source_control_bin *cb, *cbb;
    struct control_dependency *run_deps = NULL, *rd, *cd, *cdd;
    struct controlparse cp;
    struct controlparse_para *pp;
    struct controlparse_field *pf;

    if (controlparse_init(&cp))
        return -1;
    if (controlparse_parsefd(&cp, fd))
        goto out_err;

    if ((c = calloc(1, sizeof(*c))) == NULL) {
        perror("source_control_parsefd: malloc failed");
        goto out_err;
    }
    *sc = c;

    pp = cp.para_first;
    if (pp == NULL) {
        fprintf(stderr, "source_control_parsefd: file must have at least "
                "one paragraph\n");
        goto out_malloc;
    }

    /* parse first paragraph */
    for (pf = pp->field_first; pf != NULL; pf = pf->next) {
        if (!strcmp(pf->name, "Depends-Build")) {
            if (parse_deplist(pf, &c->build_depend) != 0)
                goto out_malloc;
        } else if (!strcmp(pf->name, "Depends-Unpack")) {
            if (parse_deplist(pf, &c->unpack_depend) != 0)
                goto out_malloc;
        } else if (!strcmp(pf->name, "Depends-Run")) {
            if (parse_deplist(pf, &run_deps) != 0)
                goto out_malloc;
        } else if (!strcmp(pf->name, "Sources")) {
            if (parse_sourcelist(pf, &c->sources) != 0)
                goto out_malloc;
        } else if (!strcmp(pf->name, "Source")) {
            if (parse_singleline(pf, &c->source) != 0)
                goto out_malloc;
        } else if (!strcmp(pf->name, "Version")) {
            if (parse_singleline(pf, &c->version) != 0)
                goto out_malloc;
        } else {
            fprintf(stderr, "source_control_parsefd: Unexpected field: '%s'\n",
                pf->name);
            goto out_malloc;
        }
    }

    /* parse additional binary packet paragraph */
    for (pp = pp->next; pp != NULL; pp = pp->next) {
        /* add source control bin struct to list */
        if ((cb = calloc(1, sizeof(*cb))) == NULL) {
            perror("source_control_parsefd: malloc failed");
            goto out_malloc;
        }
        if (c->bins == NULL) {
            c->bins = cb;
        } else {
            for (cbb = c->bins; cbb->next != NULL; cbb = cbb->next);
            cbb->next = cb;
        }

        for (pf = pp->field_first; pf != NULL; pf = pf->next) {
            if (!strcmp(pf->name, "Depends-Run")) {
                if (parse_deplist(pf, &cb->run_depend) != 0)
                    goto out_malloc;
            } else if (!strcmp(pf->name, "Package")) {
                if (parse_singleline(pf, &cb->package) != 0)
                    goto out_malloc;
            } else {
                fprintf(stderr, "source_control_parsefd: Unexpected field: "
                    "'%s'\n", pf->name);
                goto out_malloc;
            }
        }

        /* copy global run dependencies */
        for (cdd = cb->run_depend; cdd != NULL && cdd->next != NULL;
            cdd = cdd->next);
        for (rd = run_deps; rd != NULL; rd = rd->next) {
            if ((cd = calloc(1, sizeof(*cd))) == NULL ||
                (cd->package = strdup(rd->package)) == NULL)
            {
                free(cd);
                perror("source_control_parsefd: malloc failed");
                goto out_malloc;
            }

            /* append to list of already existing dependencies */
            if (cdd == NULL) {
                cb->run_depend = cd;
                cdd = cd;
            } else {
                cdd->next = cd;
                cdd = cd;
            }
        }
    }

    controlparse_destroy(&cp);
    return 0;

out_malloc:
    source_control_destroy(c);
out_err:
    controlparse_destroy(&cp);
    return -1;


    return -1;
}

void source_control_destroy(struct source_control *sc)
{
    struct control_dependency *cd, *cdd;
    struct control_source *cs, *css;
    struct source_control_bin *sb, *sbb;

    for (cd = sc->build_depend; cd != NULL; ) {
        cdd = cd;
        cd = cd->next;
        free(cdd->package);
        free(cdd);
    }
    for (cs = sc->sources; cs != NULL; ) {
        css = cs;
        cs = cs->next;
        free(css->source);
        free(css);
    }
    for (sb = sc->bins; sb != NULL; ) {
        sbb = sb;
        sb = sb->next;

        for (cd = sbb->run_depend; cd != NULL; ) {
            cdd = cd;
            cd = cd->next;
            free(cdd->package);
            free(cdd);
        }

        free(sbb->package);
        free(sbb);
    }

    free(sc->source);
    free(sc->version);
    free(sc);
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
