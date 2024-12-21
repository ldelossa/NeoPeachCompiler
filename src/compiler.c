#include "compiler.h"
#include "lexer.h"

#include <stdarg.h>
#include <stdlib.h>

void compiler_warning(struct compiler *compiler, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    fprintf(stderr, "on line %i, col %i in file %s\n", compiler->pos.line,
            compiler->pos.col, compiler->cfile.abs_path);
}

void compiler_error(struct compiler *compiler, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    compiler_warning(compiler, msg, args);
    va_end(args);
    exit(-1);
}

struct compiler *compiler_create(const char *infile, const char *out_file,
                                 int flags) {
    struct compiler *c = calloc(1, sizeof(struct compiler));
    c->flags = flags;
    c->pos.line = 1;
    c->pos.col = 1;
    c->pos.filename = infile;
    c->cfile.abs_path = infile;

    c->cfile.fp = fopen(infile, "r");
    if (c->cfile.fp == NULL) {
		printf("Error opening input file\n");
        exit(EXIT_FAILURE);
    }

    c->ofile = fopen(out_file, "w");
    if (c->ofile == NULL) {
		compiler_error(c, "Error opening output file\n");
        fclose(c->cfile.fp);  // Close the input file before exiting
        exit(EXIT_FAILURE);
    }

    return c;
}

int compile_file(struct compiler *c) {

	struct lexer *l = lexer_create(c);
	if (!l)
		return COMPILER_FAILED_WITH_ERRORS;

	if (lexer_lex(l) != LEXICAL_ANALYSIS_ALL_OK)
		return COMPILER_FAILED_WITH_ERRORS;

    return COMPILER_FILE_COMPILED_OK;
}
