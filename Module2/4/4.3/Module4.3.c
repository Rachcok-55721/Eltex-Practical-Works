#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 100
#define BALANCE_INTERVAL 5

// Структура контактной информации
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

// Узел бинарного дерева
typedef struct Node {
    Contact data;
    struct Node* left;
    struct Node* right;
    int height;
} Node;

Node* root = NULL;
int count = 0;
int changes = 0;

// Сравнение контактов по ФИО
int compareContacts(const Contact* a, const Contact* b) {
    int result = strcmp(a->surname, b->surname);

    if (result != 0)
        return result;

    result = strcmp(a->name, b->name);

    if (result != 0)
        return result;

    return strcmp(a->patronymic, b->patronymic);
}

int height(Node* node) {
    return node == NULL ? 0 : node->height;
}

int maxInt(int a, int b) {
    return a > b ? a : b;
}

void updateHeight(Node* node) {
    if (node != NULL) {
        node->height = 1 + maxInt(
            height(node->left),
            height(node->right)
        );
    }
}

int getBalance(Node* node) {
    if (node == NULL)
        return 0;

    return height(node->left) - height(node->right);
}

Node* rotateRight(Node* y) {
    Node* x = y->left;
    Node* temp = x->right;

    x->right = y;
    y->left = temp;

    updateHeight(y);
    updateHeight(x);

    return x;
}

Node* rotateLeft(Node* x) {
    Node* y = x->right;
    Node* temp = y->left;

    y->left = x;
    x->right = temp;

    updateHeight(x);
    updateHeight(y);

    return y;
}

// Балансировка одного узла AVL-дерева
Node* balanceNode(Node* node) {
    if (node == NULL)
        return NULL;

    updateHeight(node);

    int balance = getBalance(node);

    if (balance > 1) {
        if (getBalance(node->left) < 0)
            node->left = rotateLeft(node->left);

        return rotateRight(node);
    }

    if (balance < -1) {
        if (getBalance(node->right) > 0)
            node->right = rotateRight(node->right);

        return rotateLeft(node);
    }

    return node;
}

Node* createNode(const Contact* contact) {
    Node* node = (Node*)malloc(sizeof(Node));

    if (node == NULL)
        return NULL;

    node->data = *contact;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;

    return node;
}

// Добавление контакта в бинарное дерево
Node* insertNode(Node* node, Node* newNode) {
    if (node == NULL)
        return newNode;

    if (compareContacts(&newNode->data, &node->data) < 0)
        node->left = insertNode(node->left, newNode);
    else
        node->right = insertNode(node->right, newNode);

    updateHeight(node);
    return node;
}

int countNodes(Node* node) {
    if (node == NULL)
        return 0;

    return 1 + countNodes(node->left)
        + countNodes(node->right);
}

void collectNodes(Node* node, Node** array, int* index) {
    if (node == NULL)
        return;

    Node* right = node->right;

    collectNodes(node->left, array, index);
    array[(*index)++] = node;
    collectNodes(right, array, index);
}

Node* buildBalanced(Node** array, int left, int right) {
    if (left > right)
        return NULL;

    int mid = left + (right - left) / 2;

    Node* node = array[mid];

    node->left = buildBalanced(array, left, mid - 1);
    node->right = buildBalanced(array, mid + 1, right);

    updateHeight(node);

    return node;
}


// Return 1 on success, 0 for an empty tree, -1 for allocation failure.
int balanceTree(void) {
    int total = countNodes(root);
    if (total == 0) return 0;
    Node** array = malloc((size_t)total * sizeof(Node*));
    if (!array) return -1;
    int index = 0;
    collectNodes(root, array, &index);
    root = buildBalanced(array, 0, total - 1);
    free(array);
    return 1;
}

// Return 1 when periodic balancing occurs, -1 on allocation error, 0 otherwise.
int checkBalancing(void) {
    changes++;
    if (changes >= BALANCE_INTERVAL) {
        int result = balanceTree();
        if (result >= 0) changes = 0;
        return result == 1 ? 1 : -1;
    }
    return 0;
}
Node* findMin(Node* node) {
    while (node != NULL && node->left != NULL)
        node = node->left;

    return node;
}

// Удаление узла по контактным данным
Node* deleteNode(Node* node, const Contact* key) {
    if (node == NULL)
        return NULL;

    int result = compareContacts(key, &node->data);

    if (result < 0) {
        node->left = deleteNode(node->left, key);
    }
    else if (result > 0) {
        node->right = deleteNode(node->right, key);
    }
    else {
        if (node->left == NULL || node->right == NULL) {
            Node* child = node->left != NULL
                ? node->left : node->right;

            free(node);
            return child;
        }

        Node* successor = findMin(node->right);

        node->data = successor->data;
        node->right = deleteNode(node->right,
            &node->data);
    }

    updateHeight(node);
    return node;
}

Node* findByNumber(Node* node, int target, int* current) {
    if (node == NULL)
        return NULL;

    Node* result = findByNumber(node->left, target, current);

    if (result != NULL)
        return result;

    (*current)++;

    if (*current == target)
        return node;

    return findByNumber(node->right, target, current);
}

// Проверка AVL-свойства дерева
int checkAVL(Node* node, int* valid) {
    if (node == NULL)
        return 0;

    int left = checkAVL(node->left, valid);
    int right = checkAVL(node->right, valid);

    if (abs(left - right) > 1)
        *valid = 0;

    return 1 + maxInt(left, right);
}

// Освобождение памяти дерева
void freeTree(Node* node) {
    if (node == NULL)
        return;

    freeTree(node->left);
    freeTree(node->right);
    free(node);
}


int addContact(const Contact* contact) {
    Node* newNode = createNode(contact);
    if (!newNode) return 0;
    root = insertNode(root, newNode);
    count++;
    return 1;
}

// Return 1 on success, 0 on invalid index, -1 on allocation failure.
int editContact(int number, const Contact* updated) {
    int current = 0;
    Node* node = findByNumber(root, number, &current);
    if (!node) return 0;
    Contact oldContact = node->data;
    root = deleteNode(root, &oldContact);
    Node* newNode = createNode(updated);
    if (!newNode) {
        newNode = createNode(&oldContact);
        if (newNode) root = insertNode(root, newNode);
        else count--;
        return -1;
    }
    root = insertNode(root, newNode);
    return 1;
}

int deleteContact(int number) {
    int current = 0;
    Node* node = findByNumber(root, number, &current);
    if (!node) return 0;
    Contact key = node->data;
    root = deleteNode(root, &key);
    count--;
    return 1;
}

int main(void) {
    int choice;
    char buffer[100], extra;
    while (1) {
        printf("\n========== PHONE BOOK ==========\n");
        printf("1. Add contact\n2. View all contacts\n3. Edit contact\n");
        printf("4. Delete contact\n5. Balance AVL tree\n6. Tree information\n");
        printf("7. Show tree structure\n0. Exit\n================================\n");
        printf("Enter your choice: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        if (sscanf(buffer, " %d %c", &choice, &extra) != 1) choice = -1;

        // Print contacts in sorted order before editing or deleting.
        if (choice == 2 || choice == 3 || choice == 4) {
            if (!root) {
                printf("\nThe contact list is empty.\n");
                continue;
            }
            printf("\n===== CONTACT LIST =====\n");
            for (int i = 1; i <= count; i++) {
                int current = 0;
                Node* node = findByNumber(root, i, &current);
                if (!node) continue;
                const Contact* c = &node->data;
                printf("\nContact #%d\n", i);
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
            printf("\nTotal contacts: %d\n", count);
            if (choice == 2) continue;
        }

        switch (choice) {
        case 1:
        case 3: {
            int index = 0;
            if (choice == 3) {
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

            int status = choice == 1 ? addContact(&contact) : editContact(index, &contact);
            if (status != 1) {
                printf("\nMemory allocation error!\n"); break;
            }
            printf(choice == 1 ? "\nContact added successfully!\n"
                               : "\nContact updated successfully!\n");
            int balanced = checkBalancing();
            if (balanced != 0) {
                printf("\nPeriodic AVL balancing...\n");
                if (balanced == 1) {
                    printf("\nAVL balancing completed!\n");
                    printf("Tree height: %d\n", height(root));
                } else printf("\nMemory allocation error!\n");
            }
            break;
        }
        case 4: {
            printf("\nEnter the contact number to delete: ");
            int index;
            if (!fgets(buffer, sizeof(buffer), stdin)) goto cleanup;
            if (sscanf(buffer, " %d %c", &index, &extra) != 1 ||
                index < 1 || index > count) {
                printf("\nInvalid contact number!\n"); break;
            }
            if (deleteContact(index)) {
                printf("\nContact deleted successfully!\n");
                int balanced = checkBalancing();
                if (balanced != 0) {
                    printf("\nPeriodic AVL balancing...\n");
                    if (balanced == 1) {
                        printf("\nAVL balancing completed!\n");
                        printf("Tree height: %d\n", height(root));
                    } else printf("\nMemory allocation error!\n");
                }
            }
            break;
        }
        case 5: {
            int status = balanceTree();
            if (status == 0) printf("\nTree is empty!\n");
            else if (status < 0) printf("\nMemory allocation error!\n");
            else {
                changes = 0;
                printf("\nAVL balancing completed!\n");
                printf("Tree height: %d\n", height(root));
            }
            break;
        }
        case 6: {
            int valid = 1;
            int actualHeight = checkAVL(root, &valid);
            printf("\n===== TREE INFORMATION =====\n");
            printf("Total contacts: %d\n", count);
            printf("Tree height: %d\n", actualHeight);
            printf("AVL balanced: %s\n", valid ? "Yes" : "No");
            printf("Changes until balancing: %d\n", BALANCE_INTERVAL - changes);
            break;
        }
        case 7: {
            if (!root) { printf("\nTree is empty!\n"); break; }
            printf("\n========== AVL TREE STRUCTURE ==========\n\n");
            // Right-node-left traversal, exactly as in the original recursive print.
            typedef struct { Node* node; int level; int stage; } Frame;
            size_t capacity = (size_t)count * 3 + 1;
            Frame* stack = malloc(capacity * sizeof(Frame));
            if (!stack) { printf("Memory allocation error!\n"); break; }
            size_t top = 0;
            stack[top++] = (Frame){root, 0, 0};
            while (top) {
                Frame frame = stack[--top];
                if (!frame.node) continue;
                if (frame.stage == 1) {
                    for (int i = 0; i < frame.level; i++) printf("        ");
                    printf("|-- %s %s\n", frame.node->data.surname,
                           frame.node->data.name);
                } else {
                    stack[top++] = (Frame){frame.node->left, frame.level + 1, 0};
                    stack[top++] = (Frame){frame.node, frame.level, 1};
                    stack[top++] = (Frame){frame.node->right, frame.level + 1, 0};
                }
            }
            free(stack);
            printf("\nTree height: %d\n", height(root));
            printf("Total contacts: %d\n", count);
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
    freeTree(root);
    root = NULL;
    return 0;
}
