#ifndef PEACHCOMPILER_H
#define PEACHCOMPILER_H

#include <stdio.h>
struct pos {
    int line;
    int col;
    const char *filename;
};

enum { COMPILER_FILE_COMPILED_OK, COMPILER_FAILED_WITH_ERRORS };

struct compiler {
    // Instructs details of file compilation
    int flags;

    // Tracks lexer position
    struct pos pos;

    // input file
    struct compile_process_input_file {
        FILE *fp;
        const char *abs_path;
    } cfile;

    // output file.
    FILE *ofile;
};

struct compiler *compiler_create(const char *infile, const char *out_file,
                                 int flags);

int compile_file(struct compiler *c);

#endif  // PEACHCOMPILER_H
