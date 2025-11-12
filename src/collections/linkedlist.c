#include "collections/linkedlist.h"
#include "value.h" 
#include <stdlib.h>

LinkedList* linkedlist_new(void) {
    LinkedList* list = malloc(sizeof(LinkedList));
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    return list;
}

void linkedlist_free(LinkedList* list) {
    LLNode* current = list->head;
    while (current != NULL) {
        LLNode* next = current->next;
       
        free(current);
        current = next;
    }
    free(list);
}

void linkedlist_append(LinkedList* list, Value val) {
    LLNode* node = malloc(sizeof(LLNode));
    node->value = copy_value(val);
    node->next = NULL;

    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->count++;
}

void linkedlist_prepend(LinkedList* list, Value val) {
    LLNode* node = malloc(sizeof(LLNode));
    node->value = copy_value(val);
    node->next = list->head;

    list->head = node;
    if (list->tail == NULL) {
        list->tail = node;
    }
    list->count++;
}

Value linkedlist_remove_first(LinkedList* list) {
    if (list->head == NULL) return (Value){VAL_NIL, {0}};

    LLNode* old_head = list->head;
    Value val = old_head->value; 
    
    list->head = old_head->next;
    if (list->head == NULL) {
        list->tail = NULL;
    }
    
    free(old_head); 
    list->count--;
    return val;
}

int linkedlist_size(LinkedList* list) {
    return list->count;
}