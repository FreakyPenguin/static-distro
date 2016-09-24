#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "version.h"

static int parse_coarse(const char *v, unsigned *epoch, const char **upstream,
        size_t *us_len, const char **rev, size_t *rev_len);
static inline int is_revchar(char c);
static inline int is_uschar(char c, int has_epoch, int has_rev);
static int cmp_verstr(const char *s1, size_t s1_len, const char *s2,
        size_t s2_len);
static inline int cmp_nondig(const char *s1, size_t s1_len, const char *s2,
        size_t s2_len);
static inline int cmp_dig(const char *s1, size_t s1_len, const char *s2,
        size_t s2_len);
static inline int cmp_verchar(char c1, char c2);

enum vercmp_result version_cmp(const char *v1, const char *v2)
{
    unsigned e1, e2;
    const char *us1, *us2, *rev1, *rev2;
    size_t us1l, us2l, rev1l, rev2l;
    int ret;

    if (parse_coarse(v1, &e1, &us1, &us1l, &rev1, &rev1l) != 0 ||
        parse_coarse(v2, &e2, &us2, &us2l, &rev2, &rev2l) != 0)
    {
        return VER_FAILED;
    }

    /* compare epochs */
    if (e1 < e2) {
        return VER_LT;
    } else if (e1 > e2) {
        return VER_GT;
    }

    /* compare upstream */
    ret = cmp_verstr(us1, us1l, us2, us2l);
    if (ret < 0) {
        return VER_LT;
    } else if (ret > 0) {
        return VER_GT;
    }

    /* compare revisions */
    ret = cmp_verstr(rev1, rev1l, rev2, rev2l);
    if (ret < 0) {
        return VER_LT;
    } else if (ret > 0) {
        return VER_GT;
    }

    return VER_EQ;
}

static int parse_coarse(const char *v, unsigned *epoch, const char **upstream,
        size_t *us_len, const char **rev, size_t *rev_len)
{
    char *col, *end, *hyp;
    const char *c;
    unsigned long u;
    size_t len = strlen(v);

    /* parse epoch */
    if ((col = strchr(v, ':')) != NULL) {
        /* only unsigned integers allowed */
        if (v[0] == '-') {
            return -1;
        }

        errno = 0;
        u = strtoul(v, &end, 10);
        if (errno != 0 || end != col || u > UINT_MAX) {
            return -1;
        }
        *epoch = u;
        *upstream = col + 1;
    } else {
        *epoch = 0;
        *upstream = v;
    }

    /* parse revision */
    if ((hyp = strchr(v, '-')) != NULL) {
        *rev = hyp + 1;
        *rev_len = len - ((hyp + 1) - v);
        *us_len = (hyp - *upstream);

        /* ensure that there are no invalid characters */
        for (c = hyp + 1; *c != 0; c++) {
            if (!is_revchar(*c)) {
                return -1;
            }
        }
    } else {
        *rev = v + len;
        *rev_len = 0;
        *us_len = (*rev - *upstream);
    }

    c = *upstream;

    /* ensure that there are no invalid characters */
    for (; *c != 0 && c != hyp; c++) {
        if (!is_uschar(*c, col != NULL, hyp != NULL)) {
            return -1;
        }
    }

    return 0;
}

static inline int is_revchar(char c)
{
    return isalnum(c) || (c == '+') || (c == '.') || (c == '~');
}

static inline int is_uschar(char c, int has_epoch, int has_rev)
{
    return isalnum(c) || (c == '+') || (c == '.') || (c == '~') ||
        (has_epoch && c == ':') || (has_rev && c == '-');
}

static int cmp_verstr(const char *s1, size_t s1_len, const char *s2,
        size_t s2_len)
{
    size_t i1 = 0, i2 = 0, l1, l2;
    const char *c1, *c2;
    int ret;

    while (i1 < s1_len || i2 < s2_len) {
        /* grab parts consisting of non-digits from both strings */
        c1 = s1 + i1;
        for (l1 = 0; i1 < s1_len && !isdigit(s1[i1]); i1++, l1++);
        c2 = s2 + i2;
        for (l2 = 0; i2 < s2_len && !isdigit(s2[i2]); i2++, l2++);

        /* compare non-digit parts */
        ret = cmp_nondig(c1, l1, c2, l2);
        if (ret != 0) {
            return ret;
        }

        /* grab parts consisting of digits from both strings */
        c1 = s1 + i1;
        for (l1 = 0; i1 < s1_len && isdigit(s1[i1]); i1++, l1++);
        c2 = s2 + i2;
        for (l2 = 0; i2 < s2_len && isdigit(s2[i2]); i2++, l2++);

        /* compare digit parts */
        ret = cmp_dig(c1, l1, c2, l2);
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}

static inline int cmp_nondig(const char *s1, size_t s1_len, const char *s2,
        size_t s2_len)
{
    size_t i;
    char c1, c2;
    int ret;

    for (i = 0; i < s1_len || i < s2_len; i++) {
        c1 = (i < s1_len ? s1[i] : 0);
        c2 = (i < s2_len ? s2[i] : 0);
        ret = cmp_verchar(c1, c2);
        if (ret < 0) {
            return -1;
        } else if (ret > 0) {
            return 1;
        }
    }

    return 0;
}

static inline int cmp_verchar(char c1, char c2)
{
    if (c1 == c2) {
        return 0;
    }

    /* tilde comes before anything else */
    if (c1 == '~') {
        return -1;
    } else if (c2 == '~') {
        return 1;
    }

    /* alpha before other characters */
    if (isalpha(c1) && isalpha(c2)) {
        /* if both are alpha, just use ASCII values */
        return (c1 < c2 ? -1 : 1);
    } else if (isalpha(c1)) {
        return -1;
    } else if (isalpha(c2)) {
        return 1;
    }

    /* otherwise just compare ASCII values */
    return (c1 < c2 ? -1 : 1);
}

static inline int cmp_dig(const char *s1, size_t s1_len, const char *s2,
        size_t s2_len)
{
    unsigned long long u1, u2;

    /* a little hackish: we ignore legnths, because we know that there won't be
     * digits after s1/2_len and the strings are always 0-terminated, so this
     * works out with strtoull stopping at the first non-digit */
    (void) s1_len;
    (void) s2_len;

    u1 = strtoull(s1, NULL, 10);
    u2 = strtoull(s2, NULL, 10);
    if (u1 < u2) {
        return -1;
    } else if (u1 > u2) {
        return 1;
    } else {
        return 0;
    }
}
