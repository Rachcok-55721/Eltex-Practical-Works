
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

// Add a new contact (data processing only)
void addContact(Contact contact) {
    if (count < MAX_CONTACTS) {
        contacts[count] = contact;
        count++;
    }
}

// Edit an existing contact (data processing only)
void editContact(int index, Contact contact) {
    if (index >= 1 && index <= count) {
        contacts[index - 1] = contact;
    }
}

// Delete a contact (data processing only)
void deleteContact(int index) {
    if (index < 1 || index > count) return;
    for (int i = index - 1; i < count - 1; i++) {
        contacts[i] = contacts[i + 1];
    }
    count--;
}

// Main program: all keyboard input and screen output are here
int main() {
    int choice;
    char buffer[100];
    char extra;

    do {
        printf("\n========== PHONE BOOK ==========\n");
        printf("1. Add contact\n");
        printf("2. View all contacts\n");
        printf("3. Edit contact\n");
        printf("4. Delete contact\n");
        printf("0. Exit\n");
        printf("================================\n");
        printf("Enter your choice: ");

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
        if (sscanf(buffer, "%d %c", &choice, &extra) != 1) choice = -1;

        switch (choice) {
        case 1:
        case 3: {
            int index = 0;
            if (choice == 1 && count >= MAX_CONTACTS) {
                printf("\nThe phone book is full!\n");
                break;
            }
            if (choice == 3) {
                if (count == 0) {
                    printf("\nThe contact list is empty.\n");
                    break;
                }
                // Display contacts before selecting one to edit
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
                printf("\nEnter the contact number to edit: ");
                if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
                if (sscanf(buffer, "%d %c", &index, &extra) != 1 ||
                    index < 1 || index > count) {
                    printf("\nInvalid contact number!\n");
                    break;
                }
            }

            Contact contact = {0};
            char *fields[] = {
                contact.surname, contact.name, contact.patronymic,
                contact.workplace, contact.position, contact.phone,
                contact.email, contact.social, contact.messenger
            };
            const char *prompts[] = {
                "Last name: ", "First name: ", "Middle name: ",
                "Workplace: ", "Job title: ", "Phone number: ",
                "Email address: ", "Social media profile: ",
                "Messenger profile: "
            };
            if (choice == 1) printf("\n===== ADD NEW CONTACT =====\n");
            else {
                printf("\n===== EDIT CONTACT =====\n");
                printf("Enter new contact information.\n");
            }
            for (int i = 0; i < 9; i++) {
                do {
                    printf("%s", prompts[i]);
                    if (fgets(fields[i], MAX_LEN, stdin) == NULL) return 0;
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
                addContact(contact);
                printf("\nContact added successfully!\n");
            } else {
                editContact(index, contact);
                printf("\nContact updated successfully!\n");
            }
            break;
        }
        case 2:
            if (count == 0) {
                printf("\nThe contact list is empty.\n");
                break;
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
            break;
        case 4: {
            if (count == 0) {
                printf("\nThe contact list is empty.\n");
                break;
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
            printf("\nEnter the contact number to delete: ");
            int index;
            if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
            if (sscanf(buffer, "%d %c", &index, &extra) != 1 ||
                index < 1 || index > count) {
                printf("\nInvalid contact number!\n");
                break;
            }
            deleteContact(index);
            printf("\nContact deleted successfully!\n");
            break;
        }
        case 0:
            printf("\nExiting the phone book. Goodbye!\n");
            break;
        default:
            printf("\nInvalid choice. Please try again.\n");
        }
    } while (choice != 0);
    return 0;
}
