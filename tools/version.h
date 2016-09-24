enum vercmp_result {
    VER_LT = -1,
    VER_EQ = 0,
    VER_GT = 1,
    VER_FAILED = 2,
};

int version_validate(const char *v);
enum vercmp_result version_cmp(const char *v1, const char *v2);
