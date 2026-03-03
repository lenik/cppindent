#ifndef CPPINDENT_FORMATTER_H
#define CPPINDENT_FORMATTER_H

#include <stdio.h>

typedef enum {
    INDENT_AFTER_HASH = 0,
    INDENT_BEFORE_HASH = 1
} indent_mode_t;

typedef struct {
    indent_mode_t mode;
    int use_tabs;   /* 0 = spaces, 1 = tabs */
    int width;      /* spaces per depth when use_tabs == 0 */
} formatter_options_t;

typedef struct {
    long depth;
} formatter_state_t;

typedef enum {
    DIR_UNKNOWN = 0,
    DIR_IF,
    DIR_IFDEF,
    DIR_IFNDEF,
    DIR_ELIF,
    DIR_ELSE,
    DIR_ENDIF,
    DIR_OTHER
} directive_kind_t;

int format_stream(FILE *in, FILE *out, const char *filename,
                  const formatter_options_t *opts, formatter_state_t *st,
                  int *had_warning);

#endif /* CPPINDENT_FORMATTER_H */

