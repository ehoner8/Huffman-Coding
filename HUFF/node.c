#include "node.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

Node *node_create(uint8_t symbol, uint32_t weight) {
    Node *n = (Node *) malloc(sizeof(Node));
    if (!n) {
        return NULL;
    }
    n->symbol = symbol;
    n->weight = weight;
    return n;
}

void node_free(Node **pnode) {
    if (*pnode != NULL) {
        node_free(&(*pnode)->left);
        node_free(&(*pnode)->right);
        free(*pnode);
        *pnode = NULL;
    }
}

void node_print_node(Node *tree, char ch, int indentation) {
    if (tree == NULL) {
        return;
    }
    node_print_node(tree->right, '/', indentation + 3);
    printf("%*cweight = %d", indentation + 1, ch, tree->weight);

    if (tree->left == NULL && tree->right == NULL) {
        if (' ' <= tree->symbol && tree->symbol <= '~') {
            printf(", symbol = '%c'", tree->symbol);
        } else {
            printf(", symbol = 0x%02x", tree->symbol);
        }
    }
    printf("\n");
    node_print_node(tree->left, '\\', indentation + 3);
}

void node_print_tree(Node *tree) {
    node_print_node(tree, '<', 2);
}
