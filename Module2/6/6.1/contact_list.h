#ifndef CONTACT_LIST_H
#define CONTACT_LIST_H

#define MAX_LEN 100

// Данные контакта
typedef struct {
    char surname[MAX_LEN];
    char name[MAX_LEN];
    char patronymic[MAX_LEN];
    char workplace[MAX_LEN];
    char position[MAX_LEN];
    char phone[MAX_LEN];
    char email[MAX_LEN];
    char social[MAX_LEN];
    char messenger[MAX_LEN];
} Contact;

// Узел двухсвязного списка
typedef struct Node {
    Contact data;
    struct Node* prev;
    struct Node* next;
} Node;

extern Node* head;
extern Node* tail;
extern int count;

int compareContacts(const Contact* a, const Contact* b);
void insertSorted(Node* newNode);
void detachNode(Node* node);
Node* getContactByNumber(int number);
void freeContacts(void);

#endif
