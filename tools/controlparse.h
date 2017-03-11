#ifndef CONTROLPARSE_H_
#define CONTROLPARSE_H_

#include <stddef.h>

struct controlparse_para;
struct controlparse_field;
struct controlparse_line;

struct controlparse {
    struct controlparse_para *para_first;
    struct controlparse_para *para_last;

    /* private members */
    int in_break;
};

struct controlparse_para {
    struct controlparse_field *field_first;
    struct controlparse_field *field_last;
    struct controlparse_para *next;
};

struct controlparse_field {
    struct controlparse_line *line_first;
    struct controlparse_line *line_last;
    struct controlparse_field *next;
    char name[];
};

struct controlparse_line {
    struct controlparse_line *next;
    char line[];
};

int controlparse_init(struct controlparse *cp);
void controlparse_destroy(struct controlparse *cp);
int controlparse_line(struct controlparse *cp, const char *line,
        size_t line_len);
int controlparse_eof(struct controlparse *cp);
int controlparse_parsefd(struct controlparse *cp, int fd);

#endif /* ndef CONTROLPARSE_H_ */
