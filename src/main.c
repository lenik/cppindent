#include "formatter.h"
#include "io.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s [OPTIONS] SOURCES...\n"
            "Options:\n"
            "  -a        Indent after # (default)\n"
            "  -b        Indent before #\n"
            "  -i        Edit files in-place\n"
            "  -w<NUM>   Indent width (spaces, default 4)\n"
            "  -t        Use tabs for indentation\n",
            prog);
}

int main(int argc, char **argv) {
    formatter_options_t opts;
    opts.mode = INDENT_AFTER_HASH;
    opts.use_tabs = 0;
    opts.width = 4;

    int in_place = 0;

    int i = 1;
    for (; i < argc; ++i) {
        const char *arg = argv[i];
        if (arg[0] != '-' || strcmp(arg, "-") == 0) {
            break;
        }
        if (strcmp(arg, "--") == 0) {
            i++;
            break;
        }
        if (arg[1] == 'a' && arg[2] == '\0') {
            opts.mode = INDENT_AFTER_HASH;
        } else if (arg[1] == 'b' && arg[2] == '\0') {
            opts.mode = INDENT_BEFORE_HASH;
        } else if (arg[1] == 'i' && arg[2] == '\0') {
            in_place = 1;
        } else if (arg[1] == 't' && arg[2] == '\0') {
            opts.use_tabs = 1;
        } else if (arg[1] == 'w') {
            const char *num = arg + 2;
            if (*num == '\0') {
                if (i + 1 >= argc) {
                    usage(argv[0]);
                    return 1;
                }
                num = argv[++i];
            }
            char *endp = NULL;
            long v = strtol(num, &endp, 10);
            if (endp == num || v <= 0 || v > 16) {
                usage(argv[0]);
                return 1;
            }
            opts.width = (int)v;
            opts.use_tabs = 0;
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    int exit_code = 0;

    if (i >= argc) {
        formatter_state_t st;
        st.depth = 0;
        int had_warning = 0;
        if (format_stream(stdin, stdout, NULL, &opts, &st, &had_warning) != 0) {
            exit_code = 2;
        }
        return exit_code;
    }

    for (; i < argc; ++i) {
        const char *path = argv[i];
        if (!in_place) {
            FILE *in = io_open_input(path);
            if (!in) {
                fprintf(stderr, "cppindent: cannot open %s: %s\n",
                        path, strerror(errno));
                exit_code = 2;
                continue;
            }
            formatter_state_t st;
            st.depth = 0;
            int had_warning = 0;
            if (format_stream(in, stdout, path, &opts, &st, &had_warning) != 0) {
                exit_code = 2;
            }
            fclose(in);
        } else {
            FILE *in = io_open_input(path);
            if (!in) {
                fprintf(stderr, "cppindent: cannot open %s: %s\n",
                        path, strerror(errno));
                exit_code = 2;
                continue;
            }

            char tmp_path[4096];
            io_state_t io_st;
            FILE *out = io_open_output(path, tmp_path, sizeof(tmp_path), &io_st);
            if (!out) {
                fprintf(stderr, "cppindent: cannot create temp for %s: %s\n",
                        path, strerror(errno));
                fclose(in);
                exit_code = 2;
                continue;
            }

            formatter_state_t st;
            st.depth = 0;
            int had_warning = 0;
            if (format_stream(in, out, path, &opts, &st, &had_warning) != 0) {
                fprintf(stderr, "cppindent: error processing %s\n", path);
                exit_code = 2;
            }

            fclose(in);
            if (fclose(out) != 0) {
                fprintf(stderr, "cppindent: error writing %s\n", tmp_path);
                exit_code = 2;
                continue;
            }

            if (io_finish_output(path, tmp_path) != 0) {
                fprintf(stderr, "cppindent: cannot replace %s: %s\n",
                        path, strerror(errno));
                exit_code = 2;
            }
        }
    }

    return exit_code;
}

