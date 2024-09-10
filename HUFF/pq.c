#include "pq.h"

#include "node.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct ListElement ListElement;

struct ListElement {
    Node *tree;
    ListElement *next;
};

struct PriorityQueue {
    ListElement *list;
};

PriorityQueue *pq_create(void) {
    PriorityQueue *pq = (PriorityQueue *) malloc(sizeof(PriorityQueue));
    pq->list = NULL;
    return pq;
}

void pq_free(PriorityQueue **q) {
    free(*q);
    *q = NULL;
}

bool pq_is_empty(PriorityQueue *q) {
    if (q->list == NULL) {
        return true;
    }
    return false;
}

bool pq_size_is_1(PriorityQueue *q) {
    if (q->list == NULL) {
        return false;
    }
    if (q->list->next == NULL) {
        return true;
    }
    return false;
}

bool pq_less_than(ListElement *e1, ListElement *e2) {
    if (e1->tree->weight < e2->tree->weight) {
        return true;
    }
    if (e1->tree->weight == e2->tree->weight) {
        if (e1->tree->symbol < e2->tree->symbol) {
            return true;
        }
    }
    return false;
}

void enqueue(PriorityQueue *q, Node *tree) {
    ListElement *l = (ListElement *) malloc(sizeof(ListElement));
    l->tree = tree;
    l->next = NULL;
    if (pq_is_empty(q)) {
        q->list = l;
        return;
    }
    uint8_t counter = 0;
    ListElement *currentElement = q->list;
    ListElement *prevElement = currentElement;
    while (pq_less_than(currentElement, l)) {
        prevElement = currentElement;
        currentElement = currentElement->next;
        if (currentElement == NULL) {
            prevElement->next = l;
            return;
        }
        counter += 2;
    }
    if (counter == 0) {
        l->next = currentElement;
        q->list = l;
        return;
    }
    l->next = currentElement;
    prevElement->next = l;
}

Node *dequeue(PriorityQueue *q) {
    assert(!(pq_is_empty(q)));
    ListElement *tracker = q->list;
    q->list = tracker->next;
    Node *n = tracker->tree;
    free(tracker);
    return n;
}

void pq_print(PriorityQueue *q) {
    assert(q != NULL);
    ListElement *e = q->list;
    int position = 1;
    while (e != NULL) {
        if (position++ == 1) {
            printf("===========================================\n");
        } else {
            printf("-------------------------------------------\n");
        }
        node_print_tree(e->tree);
        e = e->next;
    }
    printf("===========================================\n");
}
