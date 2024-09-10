#include "bitreader.h"
#include "node.h"
#include "pq.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define OPTIONS "hi:o:"

typedef struct stack {
    uint8_t capacity;
    uint8_t top;
    Node **items;
} Stack;

Stack *stack_create(uint8_t capacity) {
    Stack *s = (Stack *) malloc(sizeof(Stack));
    s->capacity = capacity;
    s->top = 0;
    s->items = calloc(s->capacity, sizeof(Node *));

    return s;
}

void stack_free(Stack **sp) {
    if (sp != NULL && *sp != NULL) {
        if ((*sp)->items) {
            free((*sp)->items);
            (*sp)->items = NULL;
        }

        free(*sp);
    }

    if (sp != NULL) {
        *sp = NULL;
    }
}

bool stack_push(Stack *s, Node *n) {
    if (s->top == s->capacity) {
        return false;
    }
    s->items[s->top] = n;
    s->top++;
    return true;
}

bool stack_pop(Stack *s, Node **n) {
    if (s->top == 0) {
        return false;
    }
    *n = s->items[s->top - 1];
    s->top--;
    return true;
}

void dehuff_decompress_file(FILE *fout, BitReader *inbuf) {
    uint8_t type1;
    uint8_t type2;
    uint32_t filesize;
    uint16_t num_leaves;
    uint16_t num_nodes;
    uint8_t b;
    uint8_t symbol;
    Stack *s = stack_create(64);
    Node *node;
    Node *node2;
    type1 = bit_read_uint8(inbuf);
    assert(type1 == 'H');
    type2 = bit_read_uint8(inbuf);
    assert(type2 == 'C');
    filesize = bit_read_uint32(inbuf);
    num_leaves = bit_read_uint16(inbuf);
    num_nodes = 2 * num_leaves - 1;
    for (uint16_t i = 0; i < num_nodes; i++) {
        b = bit_read_bit(inbuf);
        if (b == 1) {
            //printf("0");
            symbol = bit_read_uint8(inbuf);
            node = node_create(symbol, 0);
        } else {
            node = node_create(0, 0);
            //printf("1\n");
            assert(stack_pop(s, &node2));
            node->right = node2;
            assert(stack_pop(s, &node2));
            node->left = node2;
        }
        stack_push(s, node);
    }
    assert(stack_pop(s, &node2));
    Node *code_tree = node2;
    for (uint32_t i = 0; i < filesize; i++) {
        node = code_tree;
        while (1) {
            b = bit_read_bit(inbuf);
            if (b == 0) {
                node = node->left;
            } else {
                node = node->right;
            }
            if (!(node->left) && !(node->right)) {
                break;
            }
        }
        fputc(node->symbol, fout);
    }
    node_free(&code_tree);
    stack_free(&s);
}

int main(int argc, char **argv) {

    char helpmessage[]
        = "Usage: dehuff -i infile -o outfile\n\thuff -v -i infile -o outfile\n\thuff -h";

    int opt;
    opterr = 0;
    char *inputf;
    char *outputf;
    //uint32_t filesize;
    //uint32_t histo[256];
    //uint16_t num_leaves;
    //Node *treeHead;
    //Code table[256];

    /*
        for (int i=0; i<256; i++){
                histo[i] = 0;
                table[i].code = 0;
                table[i].code_length = 0;
        }
	*/

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'h': printf("%s\n", helpmessage); return 0;
        case 'i': inputf = optarg; break;
        case 'o': outputf = optarg; break;
        default:
            fprintf(stderr, "huff:  unknown or poorly formatted option -%c\n", optopt);
            return 1;
        }
    }

    FILE *out = fopen(outputf, "w");
    if (!out) {
        return 1;
    }

    BitReader *r = bit_read_open(inputf);

    dehuff_decompress_file(out, r);

    fclose(out);
    bit_read_close(&r);

    //printf("Hello World\n");
}
