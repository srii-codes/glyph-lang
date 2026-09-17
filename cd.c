#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TOK_PRINT, TOK_NUM, TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV,
    TOK_LOOP, TOK_IF, TOK_EOF, TOK_UNKNOWN
} TokType;

typedef struct {
    TokType type;
    long value;
} Token;

#define MAX_TOKENS 128
static Token tokens[MAX_TOKENS];
static int tok_count = 0;
static int tok_pos = 0;

static uint32_t decode_utf8(const unsigned char *s, int *len) {
    unsigned char c = s[0];
    if (c < 0x80) { *len = 1; return c; }
    else if ((c & 0xE0) == 0xC0) {
        *len = 2;
        return ((c & 0x1F) << 6) | (s[1] & 0x3F);
    } else if ((c & 0xF0) == 0xE0) {
        *len = 3;
        return ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
    } else if ((c & 0xF8) == 0xF0) {
        *len = 4;
        return ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12)
             | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
    }
    *len = 1;
    return c;
}

static TokType codepoint_to_tok(uint32_t cp) {
    switch (cp) {
        case 0x1F5A8: return TOK_PRINT; /* 🖨 */
        case 0x2795:  return TOK_PLUS;  /* ➕ */
        case 0x2796:  return TOK_MINUS; /* ➖ */
        case 0x2716:  return TOK_MUL;   /* ✖ */
        case 0x2797:  return TOK_DIV;   /* ➗ */
        case 0x1F501: return TOK_LOOP;  /* 🔁 */
        case 0x2753:  return TOK_IF;    /* ❓ */
        default:      return TOK_UNKNOWN;
    }
}

/* ---------- Lexer ---------- */
static void lex_line(const char *line) {
    tok_count = 0;
    tok_pos = 0;
    const unsigned char *p = (const unsigned char *)line;
    int i = 0, n = (int)strlen(line);

    while (i < n && tok_count < MAX_TOKENS - 1) {
        if (p[i] == '\n' || p[i] == '\r' || p[i] == ' ') { i++; continue; }

        if (p[i] >= '0' && p[i] <= '9') {
            int start = i;
            while (i < n && p[i] >= '0' && p[i] <= '9') i++;
            long val = strtol((const char *)p + start, NULL, 10);
            tokens[tok_count].type = TOK_NUM;
            tokens[tok_count].value = val;
            tok_count++;
            continue;
        }

        int len;
        uint32_t cp = decode_utf8(p + i, &len);
        TokType t = codepoint_to_tok(cp);
        if (t == TOK_UNKNOWN) {
            fprintf(stderr, "Lexer error: unrecognized token U+%04X\n", cp);
        } else {
            tokens[tok_count].type = t;
            tokens[tok_count].value = 0;
            tok_count++;
        }
        i += len;
    }
    tokens[tok_count].type = TOK_EOF;
    tok_count++;
}

/* ---------- AST ---------- */
typedef enum { NODE_NUM, NODE_BINOP, NODE_PRINT, NODE_IF, NODE_LOOP } NodeType;

typedef struct Node {
    NodeType type;
    long num_value;        
    char op;                
    struct Node *left;      
    struct Node *right;    
    struct Node *expr;      
    struct Node *body;     
} Node;

static Node *new_node(NodeType type) {
    Node *n = calloc(1, sizeof(Node));
    n->type = type;
    return n;
}

/* ---------- Parser ---------- */
static Token peek(void) { return tokens[tok_pos]; }
static Token advance(void) { return tokens[tok_pos++]; }

static Node *parse_expr(void);  
static Node *parse_statement(void);

static Node *parse_factor(void) {
    Token t = advance();
    if (t.type != TOK_NUM) {
        fprintf(stderr, "Parse error: expected NUMBER, got token type %d\n", t.type);
        exit(1);
    }
    Node *n = new_node(NODE_NUM);
    n->num_value = t.value;
    return n;
}

static Node *parse_term(void) {
    Node *left = parse_factor();
    while (peek().type == TOK_MUL || peek().type == TOK_DIV) {
        Token op = advance();
        Node *n = new_node(NODE_BINOP);
        n->op = (op.type == TOK_MUL) ? '*' : '/';
        n->left = left;
        n->right = parse_factor();
        left = n;
    }
    return left;
}

static Node *parse_expr(void) {
    Node *left = parse_term();
    while (peek().type == TOK_PLUS || peek().type == TOK_MINUS) {
        Token op = advance();
        Node *n = new_node(NODE_BINOP);
        n->op = (op.type == TOK_PLUS) ? '+' : '-';
        n->left = left;
        n->right = parse_term();
        left = n;
    }
    return left;
}

static Node *parse_statement(void) {
    Token t = peek();

    if (t.type == TOK_PRINT) {
        advance();
        Node *n = new_node(NODE_PRINT);
        n->expr = parse_expr();
        return n;
    }
    if (t.type == TOK_IF) {
        advance();
        Node *n = new_node(NODE_IF);
        n->expr = parse_expr();      
        n->body = parse_statement(); 
        return n;
    }
    if (t.type == TOK_LOOP) {
        advance();
        Node *n = new_node(NODE_LOOP);
        n->expr = parse_expr();     
        n->body = parse_statement(); 
        return n;
    }

    fprintf(stderr, "Parse error: unexpected token type %d at statement start\n", t.type);
    exit(1);
}

/* ---------- AST pretty-printer ---------- */
static void print_ast(Node *n, int depth) {
    if (!n) return;
    for (int i = 0; i < depth; i++) printf("  ");

    switch (n->type) {
        case NODE_NUM:
            printf("NUM(%ld)\n", n->num_value);
            break;
        case NODE_BINOP:
            printf("BINOP(%c)\n", n->op);
            print_ast(n->left, depth + 1);
            print_ast(n->right, depth + 1);
            break;
        case NODE_PRINT:
            printf("PRINT\n");
            print_ast(n->expr, depth + 1);
            break;
        case NODE_IF:
            printf("IF\n");
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("COND:\n");
            print_ast(n->expr, depth + 2);
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("BODY:\n");
            print_ast(n->body, depth + 2);
            break;
        case NODE_LOOP:
            printf("LOOP\n");
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("COUNT:\n");
            print_ast(n->expr, depth + 2);
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("BODY:\n");
            print_ast(n->body, depth + 2);
            break;
    }
}

int main(void) {
    const char *filename = "input.txt";

    FILE *in = fopen(filename, "rb");
    if (!in) {
        fprintf(stderr, "Could not open %s — make sure it's in the same folder as the .exe\n", filename);
        return 1;
    }
    printf("Reading Glyph source from: %s\n", filename);

    char line[256];
    int stmt_num = 0;
    while (fgets(line, sizeof(line), in)) {
        if (line[0] == '\n' || line[0] == '\r') break;

        lex_line(line);
        Node *ast = parse_statement();

        printf("\n--- Statement %d: AST ---\n", ++stmt_num);
        print_ast(ast, 0);
    }

    fclose(in);
    return 0;
}

