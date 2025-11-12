#pragma once
#include "common.h" 

typedef struct LLNode {
    Value value;
    struct LLNode* next;
} LLNode;

struct LinkedList {
    LLNode* head;
    LLNode* tail;
    int count;
};

LinkedList* linkedlist_new(void);
void linkedlist_free(LinkedList* list);
void linkedlist_append(LinkedList* list, Value val);
void linkedlist_prepend(LinkedList* list, Value val);
Value linkedlist_remove_first(LinkedList* list);
int linkedlist_size(LinkedList* list);