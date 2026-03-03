#ifndef CPPINDENT_IO_H
#define CPPINDENT_IO_H

#include <stdio.h>

typedef enum {
    NL_UNKNOWN = 0,
    NL_LF,
    NL_CRLF
} newline_style_t;

typedef struct {
    newline_style_t style;
} io_state_t;

FILE *io_open_input(const char *path);
FILE *io_open_output(const char *path, char *tmp_path, size_t tmp_size, io_state_t *st);
int io_finish_output(const char *orig_path, const char *tmp_path);
int io_copy_stream(FILE *in, FILE *out);

#endif /* CPPINDENT_IO_H */

