
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CONTACTS 100
#define MAX_LEN 100

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

Contact contacts[MAX_CONTACTS];
int count = 0;

// Read a string from the keyboard
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

// Read a required field
void inputRequired(char* str, int size) {
    do {
        inputString(str, size);

        if (strlen(str) == 0) {
            printf("This field is required. Try again: ");
        }
    } while (strlen(str) == 0);
}

// Read a valid contact number
int inputContactNumber() {
    char buffer[30];
    int number;
    char extra;

    inputString(buffer, sizeof(buffer));

    if (sscanf(buffer, "%d %c", &number, &extra) != 1) {
        return -1;
    }

    return number;
}

// Add a new contact
void addContact() {
    if (count >= MAX_CONTACTS) {
        printf("\nThe phone book is full!\n");
        return;
    }

    Contact* c = &contacts[count];

    printf("\n===== ADD NEW CONTACT =====\n");

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

    count++;

    printf("\nContact added successfully!\n");
}

// Display all contacts
void showContacts() {
    if (count == 0) {
        printf("\nThe contact list is empty.\n");
        return;
    }

    printf("\n===== CONTACT LIST =====\n");

    for (int i = 0; i < count; i++) {
        printf("\nContact #%d\n", i + 1);
        printf("-----------------------------\n");
        printf("Last name: %s\n", contacts[i].surname);
        printf("First name: %s\n", contacts[i].name);
        printf("Middle name: %s\n", contacts[i].patronymic);
        printf("Workplace: %s\n", contacts[i].workplace);
        printf("Job title: %s\n", contacts[i].position);
        printf("Phone number: %s\n", contacts[i].phone);
        printf("Email address: %s\n", contacts[i].email);
        printf("Social media: %s\n", contacts[i].social);
        printf("Messenger: %s\n", contacts[i].messenger);
        printf("-----------------------------\n");
    }
}

// Edit an existing contact
void editContact() {
    if (count == 0) {
        printf("\nThe contact list is empty.\n");
        return;
    }

    showContacts();

    printf("\nEnter the contact number to edit: ");
    int index = inputContactNumber();

    if (index < 1 || index > count) {
        printf("\nInvalid contact number!\n");
        return;
    }

    Contact* c = &contacts[index - 1];

    printf("\n===== EDIT CONTACT =====\n");
    printf("Enter new contact information.\n");

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

    printf("\nContact updated successfully!\n");
}

// Delete a contact
void deleteContact() {
    if (count == 0) {
        printf("\nThe contact list is empty.\n");
        return;
    }

    showContacts();

    printf("\nEnter the contact number to delete: ");
    int index = inputContactNumber();

    if (index < 1 || index > count) {
        printf("\nInvalid contact number!\n");
        return;
    }

    for (int i = index - 1; i < count - 1; i++) {
        contacts[i] = contacts[i + 1];
    }

    count--;

    printf("\nContact deleted successfully!\n");
}

// Main program
int main() {
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
            printf("\nExiting the phone book. Goodbye!\n");
            break;

        default:
            printf("\nInvalid choice. Please try again.\n");
        }

    } while (choice != 0);

    return 0;
}
