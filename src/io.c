#include "io.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

FILE *io_open_input(const char *path) {
    if (!path || strcmp(path, "-") == 0) {
        return stdin;
    }
    return fopen(path, "rb");
}

static newline_style_t detect_newline_style(FILE *f) {
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c == '\r') {
            int d = fgetc(f);
            if (d == '\n') {
                return NL_CRLF;
            }
            ungetc(d, f);
        } else if (c == '\n') {
            return NL_LF;
        }
    }
    return NL_LF;
}

FILE *io_open_output(const char *path, char *tmp_path, size_t tmp_size, io_state_t *st) {
    if (!path || strcmp(path, "-") == 0) {
        if (st) {
            st->style = NL_LF;
        }
        return stdout;
    }

    if (!tmp_path || tmp_size == 0) {
        errno = EINVAL;
        return NULL;
    }

    const char *slash = strrchr(path, '/');
    const char *base = slash ? slash + 1 : path;
    size_t dir_len = slash ? (size_t)(slash - path) : 0;

    if (dir_len + 1 + strlen(base) + 6 + 1 > tmp_size) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    if (dir_len > 0) {
        memcpy(tmp_path, path, dir_len);
        tmp_path[dir_len] = '/';
        tmp_path[dir_len + 1] = '\0';
    } else {
        tmp_path[0] = '\0';
    }

    strcat(tmp_path, base);
    strcat(tmp_path, ".tmp");

    FILE *f = fopen(tmp_path, "wb");
    if (!f) {
        return NULL;
    }

    if (st) {
        FILE *orig = fopen(path, "rb");
        if (orig) {
            st->style = detect_newline_style(orig);
            fclose(orig);
        } else {
            st->style = NL_LF;
        }
    }

    return f;
}

int io_finish_output(const char *orig_path, const char *tmp_path) {
    if (!orig_path || strcmp(orig_path, "-") == 0) {
        return 0;
    }

    struct stat st;
    if (stat(orig_path, &st) == 0) {
        chmod(tmp_path, st.st_mode);
    }

    if (rename(tmp_path, orig_path) != 0) {
        return -1;
    }

    return 0;
}

int io_copy_stream(FILE *in, FILE *out) {
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            return -1;
        }
    }
    return ferror(in) ? -1 : 0;
}

