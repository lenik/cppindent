## cppindent

`cppindent` is a small command-line tool that normalizes the indentation of
preprocessor directives in C and C++ source files. It does **not** touch
normal C code; only lines whose first non-whitespace character is `#` are
considered.

Two indentation styles are supported:

- **After hash (default)**: indentation appears between `#` and the directive.
- **Before hash**: indentation appears before `#`.

### Build and install (Meson)

```bash
meson setup build
ninja -C build
sudo ninja -C build install
```

This installs:

- the `cppindent` binary (usually under `/usr/local/bin`),
- the man page `cppindent(1)` (under `share/man/man1`),
- bash completion for `cppindent` (under `share/bash-completion/completions`).

### Command-line usage

```bash
cppindent [OPTIONS] SOURCES...
```

- `-a`: indent after `#` (default).
- `-b`: indent before `#`.
- `-i`: edit files in-place.
- `-wNUM`: use `NUM` spaces per nesting level (default 4).
- `-t`: use tab characters per nesting level (overrides `-w`).

If no sources are given, `cppindent` reads from stdin and writes to stdout.

### Examples

Basic stdin usage with the default "after hash" style:

```bash
cppindent << 'EOF'
#ifdef A
#ifdef B
#define X
#endif
#endif
EOF
```

Produces:

```c
#ifdef A
#    ifdef B
#        define X
#    endif
#endif
```

Before-hash style with 2-space indentation:

```bash
cppindent -b -w2 << 'EOF'
#if A
#define X
#endif
EOF
```

Produces:

```c
#if A
  #define X
#endif
```

In-place formatting of one or more files:

```bash
cppindent -i src/foo.c src/bar.c
```

Only preprocessor directive lines are touched; all other code remains unchanged.

### Debian packaging

A minimal `debian/` directory is provided for building `.deb` packages using
`debhelper` and the Meson build system.

### License

`cppindent` is free software, released under the GNU General Public License,
version 3 or (at your option) any later version. See `COPYING` for details.

### Contact

For bugs, patches, or questions, you can reach the authors at
`cppindent@bodz.net`.

