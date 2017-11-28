#ifndef CONTROL_H_
#define CONTROL_H_

struct control_dependency;
struct control_source;
struct source_control_bin;

struct control {
    char *package;
    char *version;
    struct control_dependency *run_depend;
    struct control_source *sources;
};

struct source_control {
    char *source;
    char *version;
    struct control_dependency *unpack_depend;
    struct control_dependency *build_depend;
    struct control_source *sources;
    struct source_control_bin *bins;
};

struct source_control_bin {
    char *package;
    struct control_dependency *run_depend;
    struct source_control_bin *next;
};

struct control_dependency {
    char *package;
    struct control_dependency *next;
};

struct control_source {
    char *source;
    struct control_source *next;
};

int control_parse(const char *path, struct control **ctrl);
int control_parsefd(int fd, struct control **ctrl);
void control_destroy(struct control *ctrl);

int source_control_parsefd(int fd, struct source_control **sc);
void source_control_destroy(struct source_control *sc);

#endif /* ndef CONTROL_H_ */
