#include <stdio.h>

#include "compiler.h"

int main(int argc, char *argv[]) {
    // if (argc < 2) {
    // 	printf("Usage: %s <filename>\n", argv[0]);
    // 	return 1;
    // }
    //
    // const char *filename = argv[1];
    // const char *out_filename = "out.txt";
    // int flags = 0;

    struct compiler *c = compiler_create("test.c", "out.txt", 0);
    if (!c) {
        printf("Failed to create compiler\n");
        return 1;
    }

    if (compile_file(c) != COMPILER_FILE_COMPILED_OK) {
        printf("Failed to compile file\n");
        return 1;
    }
}
