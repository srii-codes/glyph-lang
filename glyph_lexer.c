#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "glyph_lexer.h"

#define MAX_TOKENS 2048
static Token tokens[MAX_TOKENS];
static int tok_count = 0;
static int tok_pos = 0;

Token peek_token(void) { return tokens[tok_pos]; }
Token advance_token(void) { return tokens[tok_pos++]; }

static uint32_t decode_utf8(const unsigned char *s, int *len) {
    unsigned char c = s[0];
    if (c < 0x80) { *len = 1; return c; }
    else if ((c & 0xE0) == 0xC0) { *len = 2; return ((c & 0x1F) << 6) | (s[1] & 0x3F); }
    else if ((c & 0xF0) == 0xE0) { *len = 3; return ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F); }
    else if ((c & 0xF8) == 0xF0) { *len = 4; return ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F); }
    *len = 1; return c;
}

static TokType codepoint_to_tok(uint32_t cp) {
    switch (cp) {
        case 0x1F5A8: return TOK_PRINT;  /* 🖨 */
        case 0x2795:  return TOK_PLUS;   /* ➕ */
        case 0x2796:  return TOK_MINUS;  /* ➖ */
        case 0x2716:  return TOK_MUL;    /* ✖ */
        case 0x2797:  return TOK_DIV;    /* ➗ */
        case 0x1F501: return TOK_LOOP;   /* 🔁 */
        case 0x2753:  return TOK_IF;     /* ❓ */
        case 0x1F4E5: return TOK_ASSIGN; /* 📥 */
        case 0x1F7F0: return TOK_EQ;     /* 🟰 */
        case 0x1F3ED: return TOK_DEF;    /* 🏭 */
        case 0x1F51A: return TOK_END;    /* 🔚 */
        case 0x1F519: return TOK_RETURN; /* 🔙 */
        case 0x1F4C9: return TOK_LT;     /* 📉 Less Than */
        case 0x1F4C8: return TOK_GT;     /* 📈 Greater Than */
        default:      return TOK_UNKNOWN;
    }
}

static int codepoint_to_var(uint32_t cp) {
    switch (cp) {
        case 0x1F431: return 0; /* 🐱 */
        case 0x1F436: return 1; /* 🐶 */
        case 0x1F438: return 2; /* 🐸 */
        default:      return -1;
    }
}

static int codepoint_to_func(uint32_t cp) {
    switch (cp) {
        case 0x1F98A: return 0; /* 🦊 */
        case 0x1F981: return 1; /* 🦁 */
        case 0x1F43B: return 2; /* 🐻 */
        default:      return -1;
    }
}

void lex_source(const char *source) {
    tok_count = 0; tok_pos = 0;
    const unsigned char *p = (const unsigned char *)source;
    int i = 0, n = (int)strlen(source);

    while (i < n && tok_count < MAX_TOKENS - 1) {
        if (p[i] == '\n' || p[i] == '\r' || p[i] == ' ') { i++; continue; }

        if (p[i] >= '0' && p[i] <= '9') {
            int start = i;
            while (i < n && p[i] >= '0' && p[i] <= '9') i++;
            tokens[tok_count].type = TOK_NUM;
            tokens[tok_count].value = strtol((const char *)p + start, NULL, 10);
            tok_count++; continue;
        }

        int len;
        uint32_t cp = decode_utf8(p + i, &len);
        if (cp == 0x1F522) { i += len; continue; } /* Ignore NUM_PREFIX 🔢 */

        int vid = codepoint_to_var(cp);
        if (vid != -1) { tokens[tok_count].type = TOK_VAR; tokens[tok_count].id = vid; tok_count++; i += len; continue; }

        int fid = codepoint_to_func(cp);
        if (fid != -1) { tokens[tok_count].type = TOK_FUNC; tokens[tok_count].id = fid; tok_count++; i += len; continue; }

        TokType t = codepoint_to_tok(cp);
        if (t != TOK_UNKNOWN) { tokens[tok_count].type = t; tok_count++; }
        else { fprintf(stderr, "Lexer error: unknown token U+%04X\n", cp); }
        i += len;
    }
    tokens[tok_count].type = TOK_EOF;
}
