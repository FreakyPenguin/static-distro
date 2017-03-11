#include "controlparse.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

int controlparse_init(struct controlparse *cp)
{
    cp->para_first = NULL;
    cp->para_last = NULL;
    cp->in_break = 1;
    return 0;
}

void controlparse_destroy(struct controlparse *c)
{
    struct controlparse_para *cp, *cp_next;
    struct controlparse_field *cf, *cf_next;
    struct controlparse_line *cl, *cl_next;

    for (cp = c->para_first; cp != NULL; cp = cp_next) {
        cp_next = cp->next;
        for (cf = cp->field_first; cf != NULL; cf = cf_next) {
            cf_next = cf->next;
            for (cl = cf->line_first; cl != NULL; cl = cl_next) {
                cl_next = cl->next;
                free(cl);
            }
            free(cf);
        }
        free(cp);
    }

    c->para_first = c->para_last = NULL;
    c->in_break = 1;
}

int controlparse_line(struct controlparse *c, const char *line,
        size_t line_len)
{
    size_t off;
    struct controlparse_para *cp;
    struct controlparse_field *cf;
    struct controlparse_line *cl;
    const char *col;

    /* comment line */
    if (line_len > 0 && line[0] == '#')
        return 0;

    /* skip spaces */
    for (off = 0; off < line_len && isspace(line[off]); off++);

    /* if line is empty start new paragraph */
    if (off == line_len) {
        c->in_break = 1;
        return 0;
    }

    cp = c->para_last;

    /* create new paragraph if needed */
    if (c->in_break) {
        if ((cp = malloc(sizeof(*cp))) == NULL) {
            perror("controlparse_line: malloc failed");
            return -1;
        }
        cp->field_first = cp->field_last = NULL;
        cp->next = NULL;
        if (c->para_last == NULL) {
            c->para_first = c->para_last = cp;
        } else {
            c->para_last->next = cp;
            c->para_last = cp;
        }
        c->in_break = 0;
    }

    /* add new field if necessary */
    if (off == 0) {
        /* adding new field */
        if ((col = strchr(line, ':')) == NULL) {
            fprintf(stderr, "controlparse_line: No colon to separate field "
                    "name\n");
            return -1;
        }
        off = col - line;

        /* allocate and initialize new field */
        if ((cf = malloc(sizeof(*cf) + off + 1)) == NULL) {
            perror("controlparse_line: malloc failed");
            return -1;
        }
        cf->line_first = cf->line_last = NULL;
        cf->next = NULL;
        memcpy(cf->name, line, off);
        cf->name[off] = 0;

        /* append field to list */
        if (cp->field_last == NULL) {
            cp->field_first = cp->field_last = cf;
        } else {
            cp->field_last->next = cf;
            cp->field_last = cf;
        }

        /* skip whitespace after colon */
        for (off++; off < line_len && isspace(line[off]); off++);
    }

    /* trim space off end of line */
    for (; line_len > off && isspace(line[line_len - 1]); line_len--);

    /* adding line to existing field */
    if ((cf = cp->field_last) == NULL){
        fprintf(stderr, "controlparse_line: first non-empty line in "
                "paragraph not starting a new field\n");
        return -1;
    }

    if ((cl = malloc(sizeof(*cl) + (line_len - off) + 1)) == NULL) {
        perror("controlparse_line: malloc failed");
        return -1;
    }
    cl->next = NULL;
    memcpy(cl->line, line + off, line_len - off);
    cl->line[line_len - off] = 0;

    /* append line to field */
    if (cf->line_last == NULL) {
        cf->line_first = cf->line_last = cl;
    } else {
        cf->line_last->next = cl;
        cf->line_last = cl;
    }

    return 0;
}

int controlparse_eof(struct controlparse *cp)
{
    (void) cp;
    return 0;
}

int controlparse_parsefd(struct controlparse *cp, int fd)
{
    FILE *f;
    int dfd;
    unsigned line = 0;
    char *linebuf;
    size_t lb_size;
    ssize_t bs;

    if ((dfd = dup(fd)) == -1) {
        perror("controlparse_parsefd: dup failed");
        return -1;
    }

    if ((f = fdopen(dfd, "r")) == NULL) {
        perror("control_parsefd: fopen failed");
        close(dfd);
        return -1;
    }

    lb_size = 128;
    if ((linebuf = malloc(lb_size)) == NULL) {
        perror("control_parsefd: malloc failed");
        goto error_malloc;
    }

    while ((bs = getline(&linebuf, &lb_size, f)) != -1) {
        line++;

        if (controlparse_line(cp, linebuf, bs) != 0)
            goto error_parse;
    }

    if (ferror(f)) {
        perror("controlparse_parsefd: reading line failed");
        goto error_parse;
    }

    if (controlparse_eof(cp) != 0)
        goto error_free;

    free(linebuf);
    fclose(f);
    return 0;


error_parse:
    fprintf(stderr, "Parsing Line %u failed\n", line);
error_free:
    free(linebuf);
error_malloc:
    fclose(f);
    return -1;
}
