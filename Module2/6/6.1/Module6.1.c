#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contact_list.h"

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

void inputRequired(char* str, int size) {
    do {
        inputString(str, size);

        if (strlen(str) == 0) {
            printf("This field is required. Try again: ");
        }
    } while (strlen(str) == 0);
}

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

// Ввод данных контакта
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

// Добавление контакта
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

// Редактирование контакта
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
    detachNode(node);
    node->data = updated;
    insertSorted(node);

    printf("\nContact updated successfully!\n");
    printf("The list has been sorted again.\n");
}

// Удаление контакта
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
    detachNode(node);
    free(node);

    printf("\nContact deleted successfully!\n");
}

// Главное меню
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

