
#include <stdio.h>
#include <stdlib.h>
#include "glyph_lexer.h"
#include "glyph_parser.h"
#include "glyph_interpreter.h"

int main(void) {
    const char *filename = "C:\\Users\\srira\\OneDrive\\Desktop\\CD Project\\inputtt.glf";
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open %s\n", filename);
        return 1;
    }

    /* Read the entire file into memory */
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = malloc(fsize + 1);
    fread(buffer, 1, fsize, f);
    buffer[fsize] = 0;
    fclose(f);

    printf("--- Compiling and Executing: input.glf---\n", filename);

    lex_source(buffer);
    Node *ast = parse_program();
    interpret(ast);

    free(buffer);
    return 0;
}
