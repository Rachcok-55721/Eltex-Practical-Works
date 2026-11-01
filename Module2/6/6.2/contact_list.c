#include <stdlib.h>
#include <string.h>
#include "contact_list.h"

Node* head = NULL;
Node* tail = NULL;
int count = 0;
int compareContacts(const Contact* a, const Contact* b) {
    int result;

    result = strcmp(a->surname, b->surname);

    if (result != 0) {
        return result;
    }

    result = strcmp(a->name, b->name);

    if (result != 0) {
        return result;
    }

    return strcmp(a->patronymic, b->patronymic);
}
void insertSorted(Node* newNode) {
    Node* current = head;

    newNode->prev = NULL;
    newNode->next = NULL;
    if (head == NULL) {
        head = newNode;
        tail = newNode;
        count++;
        return;
    }
    while (current != NULL &&
        compareContacts(&current->data, &newNode->data) <= 0) {
        current = current->next;
    }
    if (current == NULL) {
        newNode->prev = tail;
        tail->next = newNode;
        tail = newNode;
    }
    else if (current == head) {
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
    }
    else {
        newNode->next = current;
        newNode->prev = current->prev;

        current->prev->next = newNode;
        current->prev = newNode;
    }

    count++;
}
void detachNode(Node* node) {
    if (node == NULL) {
        return;
    }

    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    else {
        head = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    else {
        tail = node->prev;
    }

    node->prev = NULL;
    node->next = NULL;

    count--;
}
Node* getContactByNumber(int number) {
    Node* current;
    int i;

    if (number < 1 || number > count) {
        return NULL;
    }
    if (number <= count / 2) {
        current = head;

        for (i = 1; i < number; i++) {
            current = current->next;
        }
    }
    else {
        current = tail;

        for (i = count; i > number; i--) {
            current = current->prev;
        }
    }

    return current;
}
void freeContacts(void) {
    Node* current = head;

    while (current != NULL) {
        Node* nextNode = current->next;

        free(current);
        current = nextNode;
    }

    head = NULL;
    tail = NULL;
    count = 0;
}

