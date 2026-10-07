#include <stdio.h>
#include <stdlib.h>
#include "glyph_lexer.h"
#include "glyph_parser.h"

static Node *parse_expr(void);
static Node *parse_statement(void);

static Node *parse_factor(void) {
    Token t = advance_token();
    if (t.type == TOK_NUM) { Node *n = new_node(NODE_NUM); n->num_value = t.value; return n; }
    if (t.type == TOK_VAR) { Node *n = new_node(NODE_VAR); n->id = t.id; return n; }
    if (t.type == TOK_FUNC) {
        Node *n = new_node(NODE_FUNC_CALL);
        n->id = t.id;
        n->expr = parse_expr(); /* Function argument */
        return n;
    }
    fprintf(stderr, "Parse error: expected NUM, VAR, or FUNC\n"); exit(1);
}

static Node *parse_term(void) {
    Node *left = parse_factor();
    while (peek_token().type == TOK_MUL || peek_token().type == TOK_DIV) {
        Token op = advance_token();
        Node *n = new_node(NODE_BINOP);
        n->op = (op.type == TOK_MUL) ? '*' : '/';
        n->left = left; n->right = parse_factor(); left = n;
    }
    return left;
}

static Node *parse_expr(void) {
    Node *left = parse_term();
    while (peek_token().type == TOK_PLUS || peek_token().type == TOK_MINUS) {
        Token op = advance_token();
        Node *n = new_node(NODE_BINOP);
        n->op = (op.type == TOK_PLUS) ? '+' : '-';
        n->left = left; n->right = parse_term(); left = n;
    }
    return left;
}

static Node *parse_statement(void) {
    Token t = peek_token();

    if (t.type == TOK_DEF) {
        advance_token();
        Token f = advance_token(); /* Expect FUNC */
        Token v = advance_token(); /* Expect VAR (Parameter) */
        Node *n = new_node(NODE_FUNC_DEF);
        n->id = f.id;
        n->param_id = v.id;
        n->body = new_node(NODE_BLOCK);
        while (peek_token().type != TOK_END && peek_token().type != TOK_EOF) {
            add_stmt(n->body, parse_statement());
        }
        advance_token(); /* consume END */
        return n;
    }
       if (t.type == TOK_IF || t.type == TOK_LOOP) {
        advance_token();
        Node *n = new_node(t.type == TOK_IF ? NODE_IF : NODE_LOOP);
        Node *left = parse_expr();
        
        Token op_tok = advance_token(); /* consume EQ, LT, or GT */
        Node *right = parse_expr();
        
        Node *cond;
        if (op_tok.type == TOK_EQ) cond = new_node(NODE_EQ);
        else if (op_tok.type == TOK_LT) cond = new_node(NODE_LT);
        else if (op_tok.type == TOK_GT) cond = new_node(NODE_GT);
        else { fprintf(stderr, "Parse error: Expected condition operator\n"); exit(1); }
        
        cond->left = left; cond->right = right;
        n->expr = cond;
        
        n->body = new_node(NODE_BLOCK);
        while (peek_token().type != TOK_END && peek_token().type != TOK_EOF) {
            add_stmt(n->body, parse_statement());
        }
        advance_token(); /* consume END */
        return n;
    }
    if (t.type == TOK_RETURN) { advance_token(); Node *n = new_node(NODE_RETURN); n->expr = parse_expr(); return n; }
    if (t.type == TOK_PRINT) { advance_token(); Node *n = new_node(NODE_PRINT); n->expr = parse_expr(); return n; }
    if (t.type == TOK_ASSIGN) { advance_token(); Token v = advance_token(); Node *n = new_node(NODE_ASSIGN); n->id = v.id; n->expr = parse_expr(); return n; }

    /* If a function is called on a line by itself, parse it as an expression */
    if (t.type == TOK_FUNC) { return parse_expr(); }

    fprintf(stderr, "Parse error: unexpected token\n"); exit(1);
}

Node *parse_program(void) {
    Node *program = new_node(NODE_BLOCK);
    while (peek_token().type != TOK_EOF) { add_stmt(program, parse_statement()); }
    return program;
}
