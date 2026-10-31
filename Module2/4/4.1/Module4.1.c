
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

// Считывание строки с клавиатуры
void inputString(char* str, int size) {
    if (fgets(str, size, stdin) != NULL) {
        size_t len = strcspn(str, "\n");

        if (str[len] == '\n') {
            str[len] = '\0';
        }
        else {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
    }
    else {
        str[0] = '\0';
    }
}

// Ввод обязательного поля
void inputRequired(char* str, int size) {
    do {
        inputString(str, size);

        if (strlen(str) == 0) {
            printf("This field is required. Try again: ");
        }
    } while (strlen(str) == 0);
}

// Ввод корректного номера меню или контакта
int inputContactNumber(void) {
    char buffer[30];
    int number;
    char extra;

    inputString(buffer, sizeof(buffer));

    if (sscanf(buffer, "%d %c", &number, &extra) != 1) {
        return -1;
    }

    return number;
}

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

// Ввод всех полей контакта
void inputContactData(Contact* c) {
    printf("Last name: ");
    inputRequired(c->surname, MAX_LEN);

    printf("First name: ");
    inputRequired(c->name, MAX_LEN);

    printf("Middle name: ");
    inputString(c->patronymic, MAX_LEN);

    printf("Workplace: ");
    inputString(c->workplace, MAX_LEN);

    printf("Job title: ");
    inputString(c->position, MAX_LEN);

    printf("Phone number: ");
    inputString(c->phone, MAX_LEN);

    printf("Email address: ");
    inputString(c->email, MAX_LEN);

    printf("Social media profile: ");
    inputString(c->social, MAX_LEN);

    printf("Messenger profile: ");
    inputString(c->messenger, MAX_LEN);
}

// Вывод одного контакта
void printContact(const Contact* c, int number) {
    printf("\nContact #%d\n", number);
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

// Добавление нового контакта
void addContact(void) {
    Node* newNode = (Node*)malloc(sizeof(Node));

    if (newNode == NULL) {
        printf("\nError: memory allocation failed!\n");
        return;
    }

    printf("\n===== ADD NEW CONTACT =====\n");

    inputContactData(&newNode->data);

    insertSorted(newNode);

    printf("\nContact added successfully!\n");
    printf("The list is sorted alphabetically.\n");
}

// Вывод всех контактов
void showContacts(void) {
    Node* current = head;
    int number = 1;

    if (head == NULL) {
        printf("\nThe contact list is empty.\n");
        return;
    }

    printf("\n===== CONTACT LIST =====\n");

    while (current != NULL) {
        printContact(&current->data, number);

        current = current->next;
        number++;
    }
}

// Редактирование существующего контакта
void editContact(void) {
    int number;
    Node* node;
    Contact updated;

    if (head == NULL) {
        printf("\nThe contact list is empty.\n");
        return;
    }

    showContacts();

    printf("\nEnter the contact number to edit: ");
    number = inputContactNumber();

    node = getContactByNumber(number);

    if (node == NULL) {
        printf("\nInvalid contact number!\n");
        return;
    }

    printf("\n===== EDIT CONTACT =====\n");
    printf("Enter new contact information.\n");

    inputContactData(&updated);

    // Удаление узла из прежней позиции
    detachNode(node);

    // Обновление данных контакта
    node->data = updated;

    // Вставка узла в правильную позицию
    insertSorted(node);

    printf("\nContact updated successfully!\n");
    printf("The list has been sorted again.\n");
}

// Удаление существующего контакта
void deleteContact(void) {
    int number;
    Node* node;

    if (head == NULL) {
        printf("\nThe contact list is empty.\n");
        return;
    }

    showContacts();

    printf("\nEnter the contact number to delete: ");
    number = inputContactNumber();

    node = getContactByNumber(number);

    if (node == NULL) {
        printf("\nInvalid contact number!\n");
        return;
    }

    // Отсоединение узла от списка
    detachNode(node);

    // Освобождение выделенной памяти
    free(node);

    printf("\nContact deleted successfully!\n");
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

// Главная функция программы
int main(void) {
    int choice;

    do {
        printf("\n========== PHONE BOOK ==========\n");
        printf("1. Add contact\n");
        printf("2. View all contacts\n");
        printf("3. Edit contact\n");
        printf("4. Delete contact\n");
        printf("0. Exit\n");
        printf("================================\n");
        printf("Enter your choice: ");

        choice = inputContactNumber();

        switch (choice) {
        case 1:
            addContact();
            break;

        case 2:
            showContacts();
            break;

        case 3:
            editContact();
            break;

        case 4:
            deleteContact();
            break;

        case 0:
            freeContacts();
            printf("\nExiting the phone book. Goodbye!\n");
            break;

        default:
            printf("\nInvalid choice. Please try again.\n");
            break;
        }

    } while (choice != 0);

    return 0;
}
