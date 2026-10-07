#include <stdlib.h>
#include "glyph_ast.h"

Node *new_node(NodeType type) {
    Node *n = calloc(1, sizeof(Node)); /* calloc zeroes memory */
    n->type = type;
    return n;
}

void add_stmt(Node *block, Node *stmt) {
    if (!block->body) {
        block->body = stmt;
        return;
    }
    Node *curr = block->body;
    while (curr->next) {
        curr = curr->next;
    }
    curr->next = stmt;
}
