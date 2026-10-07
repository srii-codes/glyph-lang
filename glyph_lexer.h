#ifndef GLYPH_LEXER_H
#define GLYPH_LEXER_H

typedef enum {
    TOK_PRINT, TOK_NUM, TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV,
    TOK_LOOP, TOK_IF, TOK_ASSIGN, TOK_VAR, TOK_FUNC, TOK_DEF,
    TOK_END, TOK_RETURN, TOK_EQ, TOK_EOF, TOK_UNKNOWN,
    TOK_LT, TOK_GT    
} TokType;

typedef struct {
    TokType type;
    long value;
    int id;
} Token;

void lex_source(const char *source);
Token peek_token(void);
Token advance_token(void);

#endif
