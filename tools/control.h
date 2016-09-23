struct control_dependency;
struct control_source;

struct control {
    char *package;
    char *version;
    struct control_dependency *build_depend;
    struct control_dependency *run_depend;
    struct control_source *sources;
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
void control_destroy(struct control *ctrl);
