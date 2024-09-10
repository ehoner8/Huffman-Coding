#include "bitwriter.h"
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

typedef struct Code {
    uint64_t code;
    uint8_t code_length;
} Code;

uint32_t fill_histogram(FILE *fin, uint32_t *histogram) {
    uint32_t filesize = 0;
    int c;
    //assert((sizeof(histogram)/sizeof(histogram[0])) == 256);

    for (int i = 0; i < 256; i++) {
        histogram[i] = 0;
    }

    ++histogram[0x00];
    ++histogram[0xff];

    while ((c = fgetc(fin)) != EOF) {
        ++histogram[c];
        ++filesize;
    }
    return filesize;
}

Node *create_tree(uint32_t *histogram, uint16_t *num_leaves) {
    PriorityQueue *pq = pq_create();
    Node *n;
    *num_leaves = 0;
    for (int i = 0; i < 256; i++) {
        if (histogram[i] != 0) {
            n = node_create((uint8_t) i, histogram[i]);
            *num_leaves += 1;
            enqueue(pq, n);
        }
    }

    Node *left;
    Node *right;
    Node *newNode;

    //pq_print(pq);

    while (!(pq_is_empty(pq)) && !(pq_size_is_1(pq))) {
        left = dequeue(pq);
        //printf("left symbol: %u\n", left->symbol);
        right = dequeue(pq);
        //printf("right symbol: %u\n", right->symbol);
        newNode = node_create(0, left->weight + right->weight);
        newNode->left = left;
        newNode->right = right;
        enqueue(pq, newNode);
    }
    Node *head = dequeue(pq);
    //node_print_tree(head);
    pq_free(&pq);
    return head;
}

void fill_code_table(Code *code_table, Node *node, uint64_t code, uint8_t code_length) {
    //assert((sizeof(code_table))/(sizeof(code_table[0])) == 256);
    /*for (int i=0; i<256; i++){
		(code_table[i])->code = 0;
		(code_table[i])->code_length = 0;
	}*/

    if (node->left || node->right) {
        //printf("%u\n", node->left->symbol);
        //code = code << 1;
        fill_code_table(code_table, node->left, code, code_length + 1);
        code |= (uint64_t) 1 << code_length;
        //code |= 1;
        fill_code_table(code_table, node->right, code, code_length + 1);
    } else {
        code_table[node->symbol].code = code;
        code_table[node->symbol].code_length = code_length;
        //printf("code for %c: %lu\n", (char) node->symbol, code);
        //printf("code length: %u\n\n", code_length);
    }
}

void huff_write_tree(BitWriter *outbuf, Node *node) {
    if (node->left == NULL) {
        bit_write_bit(outbuf, 1);
        bit_write_uint8(outbuf, node->symbol);
        //printf("%d:", 1);
        //printf("%u", node->symbol);
    } else {
        huff_write_tree(outbuf, node->left);
        huff_write_tree(outbuf, node->right);
        bit_write_bit(outbuf, 0);
        //printf("%d:", 0);
    }
}

void huff_compress_file(BitWriter *outbuf, FILE *fin, uint32_t filesize, uint16_t num_leaves,
    Node *code_tree, Code *code_table) {
    int b;
    uint64_t code;
    uint8_t code_length;
    bit_write_uint8(outbuf, 'H');
    bit_write_uint8(outbuf, 'C');
    bit_write_uint32(outbuf, filesize);
    bit_write_uint16(outbuf, num_leaves);
    huff_write_tree(outbuf, code_tree);
    //huff_write_tree(outbuf, code_tree);
    //printf("\n");
    //node_print_tree(code_tree);
    while (1) {
        b = fgetc(fin);
        if (b == EOF) {
            break;
        }
        code = code_table[b].code;
        code_length = code_table[b].code_length;
        for (uint8_t i = 0; i < code_length; i++) {
            bit_write_bit(outbuf, (code & 1));
            code >>= 1;
        }
    }
}

/*
void huff_write_tree(Bitwriter *outbuf, Node *node){
	if (node->left == NULL){
		bit_write_bit(outbuf, 1);
		bit_write_uint8(outbuf, node->symbol);
	}
	else {
		huff_write_tree(outbuf, node->left);
		huff_write_tree(outbuf, node->right);
		bit_write_bit(outbuf, 0);
	}
}

*/

int main(int argc, char **argv) {

    char helpmessage[]
        = "Usage: huff -i infile -o outfile\n\thuff -v -i infile -o outfile\n\thuff -h";

    int opt;
    opterr = 0;
    char *inputf;
    char *outputf;
    uint32_t filesize;
    uint32_t histo[256];
    uint16_t num_leaves;
    Node *treeHead;
    Code table[256];

    for (int i = 0; i < 256; i++) {
        histo[i] = 0;
        table[i].code = 0;
        table[i].code_length = 0;
    }

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

    FILE *in = fopen(inputf, "r");
    if (!in) {
        return 1;
    }

    BitWriter *outb = bit_write_open(outputf);

    filesize = fill_histogram(in, histo);

    treeHead = create_tree(histo, &num_leaves);

    fill_code_table(table, treeHead, 0, 0);

    rewind(in);

    huff_compress_file(outb, in, filesize, num_leaves, treeHead, table);

    bit_write_close(&outb);
    fclose(in);

    return 0;
}
