#include "formatter.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Maximum supported line length; we process one line at a time */
#define MAX_LINE 8192

static directive_kind_t classify_directive(const char *keyword, size_t len) {
    if (len == 2 && strncmp(keyword, "if", 2) == 0) return DIR_IF;
    if (len == 5 && strncmp(keyword, "ifdef", 5) == 0) return DIR_IFDEF;
    if (len == 6 && strncmp(keyword, "ifndef", 6) == 0) return DIR_IFNDEF;
    if (len == 4 && strncmp(keyword, "elif", 4) == 0) return DIR_ELIF;
    if (len == 4 && strncmp(keyword, "else", 4) == 0) return DIR_ELSE;
    if (len == 5 && strncmp(keyword, "endif", 5) == 0) return DIR_ENDIF;
    if (len == 6 && strncmp(keyword, "define", 6) == 0) return DIR_OTHER;
    if (len == 5 && strncmp(keyword, "undef", 5) == 0) return DIR_OTHER;
    if (len == 7 && strncmp(keyword, "include", 7) == 0) return DIR_OTHER;
    if (len == 6 && strncmp(keyword, "pragma", 6) == 0) return DIR_OTHER;
    if (len == 5 && strncmp(keyword, "error", 5) == 0) return DIR_OTHER;
    if (len == 7 && strncmp(keyword, "warning", 7) == 0) return DIR_OTHER;
    if (len == 4 && strncmp(keyword, "line", 4) == 0) return DIR_OTHER;
    return DIR_UNKNOWN;
}

static void write_indent(FILE *out, const formatter_options_t *opts, long depth) {
    if (depth <= 0) {
        return;
    }
    if (opts->use_tabs) {
        for (long i = 0; i < depth; ++i) {
            fputc('\t', out);
        }
    } else {
        long total = depth * (opts->width > 0 ? opts->width : 4);
        for (long i = 0; i < total; ++i) {
            fputc(' ', out);
        }
    }
}

int format_stream(FILE *in, FILE *out, const char *filename,
                  const formatter_options_t *opts, formatter_state_t *st,
                  int *had_warning) {
    char buf[MAX_LINE];
    long line_no = 0;

    if (!st) {
        return 1;
    }
    if (had_warning) {
        *had_warning = 0;
    }

    while (fgets(buf, sizeof(buf), in) != NULL) {
        line_no++;
        size_t len = strlen(buf);

        /* Detect if line ends with newline */
        int has_nl = 0;
        char nl = '\0';
        if (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
            has_nl = 1;
            nl = buf[len - 1];
            buf[--len] = '\0';
        }

        /* Detect CRLF: previous char is '\r' when nl is '\n' */
        int had_cr_before_nl = 0;
        if (has_nl && nl == '\n' && len > 0 && buf[len - 1] == '\r') {
            had_cr_before_nl = 1;
            buf[--len] = '\0';
        }

        const char *line = buf;
        const char *p = line;

        /* Skip leading spaces and tabs */
        while (*p == ' ' || *p == '\t') {
            p++;
        }

        if (*p != '#') {
            /* Not a directive line; output original as-is */
            fputs(line, out);
            if (has_nl) {
                if (had_cr_before_nl) {
                    fputc('\r', out);
                }
                fputc('\n', out);
            }
            continue;
        }

        /* At '#', now detect and classify directive keyword */
        const char *after_hash = p + 1;
        while (*after_hash == ' ' || *after_hash == '\t') {
            after_hash++;
        }

        const char *kw_start = after_hash;
        while (*after_hash && (isalpha((unsigned char)*after_hash) || *after_hash == '_')) {
            after_hash++;
        }
        const char *kw_end = after_hash;
        size_t kw_len = (size_t)(kw_end - kw_start);

        directive_kind_t kind = DIR_UNKNOWN;
        if (kw_len > 0) {
            kind = classify_directive(kw_start, kw_len);
        }

        /* Handle depth adjustment for #endif before formatting */
        if (kind == DIR_ENDIF) {
            st->depth--;
            if (st->depth < 0) {
                if (had_warning) {
                    *had_warning = 1;
                }
                st->depth = 0;
                if (filename) {
                    fprintf(stderr,
                            "cppindent: warning: unmatched #endif in %s:%ld\n",
                            filename, line_no);
                } else {
                    fprintf(stderr,
                            "cppindent: warning: unmatched #endif on line %ld\n",
                            line_no);
                }
            }
        }

        /* Build directive tail: everything after '#', skipping leading ws */
        const char *tail = kw_start;

        /* Emit formatted line */
        if (opts->mode == INDENT_BEFORE_HASH) {
            write_indent(out, opts, st->depth);
            fputc('#', out);
            if (*tail && *tail != ' ' && *tail != '\t') {
                fputc(' ', out);
            }
            fputs(tail, out);
        } else { /* INDENT_AFTER_HASH */
            fputc('#', out);
            write_indent(out, opts, st->depth);
            fputs(tail, out);
        }

        if (has_nl) {
            if (had_cr_before_nl) {
                fputc('\r', out);
            }
            fputc('\n', out);
        }

        /* Update depth after formatting */
        if (kind == DIR_IF || kind == DIR_IFDEF || kind == DIR_IFNDEF) {
            st->depth++;
        }
    }

    if (st->depth > 0 && filename) {
        if (had_warning) {
            *had_warning = 1;
        }
        fprintf(stderr,
                "cppindent: warning: missing #endif in %s\n",
                filename);
    }

    return ferror(in) || ferror(out);
}

