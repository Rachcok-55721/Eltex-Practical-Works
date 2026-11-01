#ifndef CONTACT_LIST_H
#define CONTACT_LIST_H

#define MAX_LEN 100

#ifdef _WIN32
    #ifdef CONTACT_LIST_BUILD
        #define CONTACT_API __declspec(dllexport)
    #else
        #define CONTACT_API __declspec(dllimport)
    #endif
#else
    #define CONTACT_API
#endif

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

CONTACT_API extern Node* head;
CONTACT_API extern Node* tail;
CONTACT_API extern int count;

CONTACT_API int compareContacts(const Contact* a, const Contact* b);
CONTACT_API void insertSorted(Node* newNode);
CONTACT_API void detachNode(Node* node);
CONTACT_API Node* getContactByNumber(int number);
CONTACT_API void freeContacts(void);

#endif
