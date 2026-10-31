
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

int inputNumber(void) {
    char buffer[30];
    int number;
    char extra;

    inputString(buffer, sizeof(buffer));

    if (sscanf(buffer, "%d %c", &number, &extra) != 1) {
        return -1;
    }

    return number;
}

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

// Периодическая AVL-балансировка всего дерева
void balanceTree(void) {
    int total = countNodes(root);

    if (total == 0) {
        printf("\nTree is empty!\n");
        return;
    }

    Node** array = malloc((size_t)total * sizeof(Node*));

    if (array == NULL) {
        printf("\nMemory allocation error!\n");
        return;
    }

    int index = 0;

    collectNodes(root, array, &index);
    root = buildBalanced(array, 0, total - 1);

    free(array);

    printf("\nAVL balancing completed!\n");
    printf("Tree height: %d\n", height(root));
}

void checkBalancing(void) {
    changes++;

    if (changes >= BALANCE_INTERVAL) {
        printf("\nPeriodic AVL balancing...\n");
        balanceTree();
        changes = 0;
    }
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

// Симметричный обход дерева
void printTree(Node* node, int* number) {
    if (node == NULL)
        return;

    printTree(node->left, number);
    printContact(&node->data, (*number)++);
    printTree(node->right, number);
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

void addContact(void) {
    Contact contact;

    printf("\n===== ADD NEW CONTACT =====\n");
    inputContactData(&contact);

    Node* newNode = createNode(&contact);

    if (newNode == NULL) {
        printf("\nMemory allocation error!\n");
        return;
    }

    root = insertNode(root, newNode);
    count++;

    printf("\nContact added successfully!\n");

    checkBalancing();
}

void showContacts(void) {
    if (root == NULL) {
        printf("\nThe contact list is empty.\n");
        return;
    }

    int number = 1;

    printf("\n===== CONTACT LIST =====\n");
    printTree(root, &number);

    printf("\nTotal contacts: %d\n", count);
}

// Редактирование существующего контакта
void editContact(void) {
    if (root == NULL) {
        printf("\nThe contact list is empty.\n");
        return;
    }

    showContacts();

    printf("\nEnter the contact number to edit: ");
    int number = inputNumber();

    if (number < 1 || number > count) {
        printf("\nInvalid contact number!\n");
        return;
    }

    int current = 0;
    Node* node = findByNumber(root, number, &current);

    if (node == NULL)
        return;

    Contact oldContact = node->data;
    Contact updated;

    printf("\n===== EDIT CONTACT =====\n");
    printf("Enter new contact information.\n");

    inputContactData(&updated);

    root = deleteNode(root, &oldContact);

    Node* newNode = createNode(&updated);

    if (newNode == NULL) {
        printf("\nMemory allocation error!\n");

        newNode = createNode(&oldContact);

        if (newNode != NULL)
            root = insertNode(root, newNode);
        else
            count--;

        return;
    }

    root = insertNode(root, newNode);

    printf("\nContact updated successfully!\n");

    checkBalancing();
}

void deleteContact(void) {
    if (root == NULL) {
        printf("\nThe contact list is empty.\n");
        return;
    }

    showContacts();

    printf("\nEnter the contact number to delete: ");
    int number = inputNumber();

    if (number < 1 || number > count) {
        printf("\nInvalid contact number!\n");
        return;
    }

    int current = 0;
    Node* node = findByNumber(root, number, &current);

    if (node == NULL)
        return;

    Contact key = node->data;

    root = deleteNode(root, &key);
    count--;

    printf("\nContact deleted successfully!\n");

    checkBalancing();
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

void showTreeInfo(void) {
    int valid = 1;

    int actualHeight = checkAVL(root, &valid);

    printf("\n===== TREE INFORMATION =====\n");
    printf("Total contacts: %d\n", count);
    printf("Tree height: %d\n", actualHeight);
    printf("AVL balanced: %s\n", valid ? "Yes" : "No");
    printf("Changes until balancing: %d\n",
        BALANCE_INTERVAL - changes);
}

// Освобождение памяти дерева
void freeTree(Node* node) {
    if (node == NULL)
        return;

    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

// Отображение структуры бинарного дерева
void printTreeStructure(Node* node, int level) {
    if (node == NULL)
        return;

    printTreeStructure(node->right, level + 1);

    for (int i = 0; i < level; i++) {
        printf("        ");
    }

    printf("|-- %s %s\n",
        node->data.surname,
        node->data.name);

    printTreeStructure(node->left, level + 1);
}

void showTreeStructure(void) {
    if (root == NULL) {
        printf("\nTree is empty!\n");
        return;
    }

    printf("\n========== AVL TREE STRUCTURE ==========\n\n");

    printTreeStructure(root, 0);

    printf("\nTree height: %d\n", height(root));
    printf("Total contacts: %d\n", count);
}

// Главное меню программы
int main(void) {
    int choice;

    do {
        printf("\n========== PHONE BOOK ==========\n");
        printf("1. Add contact\n");
        printf("2. View all contacts\n");
        printf("3. Edit contact\n");
        printf("4. Delete contact\n");
        printf("5. Balance AVL tree\n");
        printf("6. Tree information\n");
        printf("7. Show tree structure\n");
        printf("0. Exit\n");
        printf("================================\n");
        printf("Enter your choice: ");

        choice = inputNumber();

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

        case 5:
            balanceTree();
            changes = 0;
            break;

        case 6:
            showTreeInfo();
            break;

        case 7:
            showTreeStructure();
            break;

        case 0:
            freeTree(root);
            root = NULL;
            printf("\nExiting the phone book. Goodbye!\n");
            break;

        default:
            printf("\nInvalid choice. Please try again.\n");
            break;
        }

    } while (choice != 0);

    return 0;
}
