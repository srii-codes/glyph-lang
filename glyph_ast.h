#ifndef GLYPH_AST_H
#define GLYPH_AST_H

typedef enum {
    NODE_NUM, NODE_VAR, NODE_BINOP, NODE_PRINT, NODE_IF, NODE_LOOP, NODE_ASSIGN,
    NODE_FUNC_DEF, NODE_FUNC_CALL, NODE_RETURN, NODE_EQ, NODE_BLOCK,
    NODE_LT, NODE_GT  
} NodeType;

typedef struct Node {
    NodeType type;
    long num_value;
    int id;               /* var_id for VAR/ASSIGN, func_id for FUNC_DEF/CALL */
    int param_id;         /* The parameter var_id used in FUNC_DEF */
    char op;
    struct Node *left;
    struct Node *right;
    struct Node *expr;
    struct Node *body;    /* For IF, LOOP, FUNC_DEF, BLOCK */
    struct Node *next;    /* For linked list of statements in a BLOCK */
} Node;

Node *new_node(NodeType type);
void add_stmt(Node *block, Node *stmt);

#endif
