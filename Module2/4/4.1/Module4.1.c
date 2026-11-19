#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 100

// Информация о контакте
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

// Указатели на первый и последний узлы списка
Node* head = NULL;
Node* tail = NULL;
int count = 0;

// Сравнение двух контактов по алфавиту
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

// Вставка узла в упорядоченный список
void insertSorted(Node* newNode) {
    Node* current = head;

    newNode->prev = NULL;
    newNode->next = NULL;

    // Проверка на пустой список
    if (head == NULL) {
        head = newNode;
        tail = newNode;
        count++;
        return;
    }

    // Поиск правильной позиции для вставки
    while (current != NULL &&
        compareContacts(&current->data, &newNode->data) <= 0) {
        current = current->next;
    }

    // Вставка в конец списка
    if (current == NULL) {
        newNode->prev = tail;
        tail->next = newNode;
        tail = newNode;
    }
    // Вставка в начало списка
    else if (current == head) {
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
    }
    // Вставка между двумя узлами
    else {
        newNode->next = current;
        newNode->prev = current->prev;

        current->prev->next = newNode;
        current->prev = newNode;
    }

    count++;
}

// Отсоединение узла от списка
// Память при этом не освобождается
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

// Поиск контакта по его порядковому номеру
Node* getContactByNumber(int number) {
    Node* current;
    int i;

    if (number < 1 || number > count) {
        return NULL;
    }

    // Поиск с начала списка
    if (number <= count / 2) {
        current = head;

        for (i = 1; i < number; i++) {
            current = current->next;
        }
    }
    // Поиск с конца списка
    else {
        current = tail;

        for (i = count; i > number; i--) {
            current = current->prev;
        }
    }

    return current;
}


// Add a contact to the sorted doubly linked list.
int addContact(const Contact* contact) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) return 0;
    newNode->data = *contact;
    insertSorted(newNode);
    return 1;
}

// Edit a contact and reinsert it at its alphabetical position.
int editContact(int number, const Contact* updated) {
    Node* node = getContactByNumber(number);
    if (!node) return 0;
    detachNode(node);
    node->data = *updated;
    insertSorted(node);
    return 1;
}

int deleteContact(int number) {
    Node* node = getContactByNumber(number);
    if (!node) return 0;
    detachNode(node);
    free(node);
    return 1;
}
// Освобождение памяти всех узлов перед завершением
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


int main(void) {
    int choice;
    char buffer[100], extra;
    while (1) {
        printf("\n========== PHONE BOOK ==========\n");
        printf("1. Add contact\n2. View all contacts\n3. Edit contact\n");
        printf("4. Delete contact\n0. Exit\n================================\n");
        printf("Enter your choice: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        if (sscanf(buffer, " %d %c", &choice, &extra) != 1) choice = -1;
        switch (choice) {
        case 1:
        case 3: {
            int index = 0;
            if (choice == 3) {
                if (!head) { printf("\nThe contact list is empty.\n"); break; }
                printf("\n===== CONTACT LIST =====\n");
                int number = 1;
                for (Node* node = head; node != NULL; node = node->next) {
                    const Contact* c = &node->data;
                    printf("\nContact #%d\n", number++);
                    printf("-----------------------------\n");
                    printf("Last name: %s\n", c->surname);
                    printf("First name: %s\n", c->name);
                    printf("Middle name: %s\n", c->patronymic);
                    printf("Workplace: %s\n", c->workplace);
                    printf("Job title: %s\n", c->position);
                    printf("Phone number: %s\n", c->phone);
                    printf("Email address: %s\n", c->email);
                    printf("Social media: %s\n", c->social);
                    printf("Messenger: %s\n", c->messenger);
                    printf("-----------------------------\n");
                }
                printf("\nEnter the contact number to edit: ");
                if (!fgets(buffer, sizeof(buffer), stdin)) goto cleanup;
                if (sscanf(buffer, " %d %c", &index, &extra) != 1 ||
                    index < 1 || index > count) {
                    printf("\nInvalid contact number!\n"); break;
                }
            }

            Contact contact = {0};
            char* fields[] = {
                contact.surname, contact.name, contact.patronymic,
                contact.workplace, contact.position, contact.phone,
                contact.email, contact.social, contact.messenger
            };
            const char* prompts[] = {
                "Last name: ", "First name: ", "Middle name: ",
                "Workplace: ", "Job title: ", "Phone number: ",
                "Email address: ", "Social media profile: ", "Messenger profile: "
            };
            printf("\n===== %s CONTACT =====\n", choice == 1 ? "ADD NEW" : "EDIT");
            if (choice == 3) printf("Enter new contact information.\n");
            for (int i = 0; i < 9; i++) {
                do {
                    printf("%s", prompts[i]);
                    if (!fgets(fields[i], MAX_LEN, stdin)) goto cleanup;
                    size_t len = strcspn(fields[i], "\n");
                    if (fields[i][len] == '\n') fields[i][len] = '\0';
                    else {
                        int ch;
                        while ((ch = getchar()) != '\n' && ch != EOF) {}
                    }
                    if (i < 2 && fields[i][0] == '\0')
                        printf("This field is required. Try again: ");
                } while (i < 2 && fields[i][0] == '\0');
            }

            if (choice == 1) {
                if (addContact(&contact)) {
                    printf("\nContact added successfully!\n");
                    printf("The list is sorted alphabetically.\n");
                } else printf("\nError: memory allocation failed!\n");
            } else {
                if (editContact(index, &contact)) {
                    printf("\nContact updated successfully!\n");
                    printf("The list has been sorted again.\n");
                } else {
                    printf("\nInvalid contact number!\n");
                }
            }
            break;
        }
        case 2: {
            if (!head) {
                printf("\nThe contact list is empty.\n");
                break;
            }
            printf("\n===== CONTACT LIST =====\n");
                int number = 1;
                for (Node* node = head; node != NULL; node = node->next) {
                    const Contact* c = &node->data;
                    printf("\nContact #%d\n", number++);
                    printf("-----------------------------\n");
                    printf("Last name: %s\n", c->surname);
                    printf("First name: %s\n", c->name);
                    printf("Middle name: %s\n", c->patronymic);
                    printf("Workplace: %s\n", c->workplace);
                    printf("Job title: %s\n", c->position);
                    printf("Phone number: %s\n", c->phone);
                    printf("Email address: %s\n", c->email);
                    printf("Social media: %s\n", c->social);
                    printf("Messenger: %s\n", c->messenger);
                    printf("-----------------------------\n");
                }
            break;
        }
        case 4: {
            if (!head) { printf("\nThe contact list is empty.\n"); break; }
            printf("\n===== CONTACT LIST =====\n");
                int number = 1;
                for (Node* node = head; node != NULL; node = node->next) {
                    const Contact* c = &node->data;
                    printf("\nContact #%d\n", number++);
                    printf("-----------------------------\n");
                    printf("Last name: %s\n", c->surname);
                    printf("First name: %s\n", c->name);
                    printf("Middle name: %s\n", c->patronymic);
                    printf("Workplace: %s\n", c->workplace);
                    printf("Job title: %s\n", c->position);
                    printf("Phone number: %s\n", c->phone);
                    printf("Email address: %s\n", c->email);
                    printf("Social media: %s\n", c->social);
                    printf("Messenger: %s\n", c->messenger);
                    printf("-----------------------------\n");
                }
            printf("\nEnter the contact number to delete: ");
            int index;
            if (!fgets(buffer, sizeof(buffer), stdin)) goto cleanup;
            if (sscanf(buffer, " %d %c", &index, &extra) != 1 ||
                index < 1 || index > count) {
                printf("\nInvalid contact number!\n"); break;
            }
            if (deleteContact(index))
                printf("\nContact deleted successfully!\n");
            else
                printf("\nInvalid contact number!\n");
            break;
        }
        case 0:
            printf("\nExiting the phone book. Goodbye!\n");
            goto cleanup;
        default:
            printf("\nInvalid choice. Please try again.\n");
        }
    }
cleanup:
    freeContacts();
    return 0;
}
