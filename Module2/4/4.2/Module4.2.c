#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TEXT 100

// Структура сообщения
typedef struct Message {
    int id;
    int priority;
    char text[MAX_TEXT];
    struct Message* next;
} Message;

// Структура очереди
typedef struct {
    Message* head;
    int count;
} PriorityQueue;

// Инициализация очереди
void initQueue(PriorityQueue* queue) {
    queue->head = NULL;
    queue->count = 0;
}

// Добавление сообщения с учетом приоритета
int enqueue(PriorityQueue* queue, int id,
    int priority, const char* text) {
    if (priority < 0 || priority > 255) {
        printf("Invalid priority!\n");
        return 0;
    }

    Message* newMessage = malloc(sizeof(Message));

    if (newMessage == NULL) {
        printf("Memory allocation error!\n");
        return 0;
    }

    newMessage->id = id;
    newMessage->priority = priority;
    snprintf(newMessage->text, MAX_TEXT, "%s", text);
    newMessage->next = NULL;

    if (queue->head == NULL ||
        priority > queue->head->priority) {
        newMessage->next = queue->head;
        queue->head = newMessage;
    }
    else {
        Message* current = queue->head;

        // Поиск позиции по убыванию приоритета
        while (current->next != NULL &&
            current->next->priority >= priority) {
            current = current->next;
        }

        newMessage->next = current->next;
        current->next = newMessage;
    }

    queue->count++;
    return 1;
}

// Удаление выбранного сообщения
void removeNode(PriorityQueue* queue,
    Message* previous, Message* current) {
    if (previous == NULL) {
        queue->head = current->next;
    }
    else {
        previous->next = current->next;
    }

    printf("\nExtracted message:\n");
    printf("ID: %d | Priority: %d | Text: %s\n",
        current->id,
        current->priority,
        current->text);

    free(current);
    queue->count--;
}

// Извлечение первого сообщения
int dequeueFirst(PriorityQueue* queue) {
    if (queue->head == NULL) {
        printf("Queue is empty!\n");
        return 0;
    }

    removeNode(queue, NULL, queue->head);
    return 1;
}

// Извлечение по точному приоритету
int dequeuePriority(PriorityQueue* queue, int priority) {
    Message* current = queue->head;
    Message* previous = NULL;

    while (current != NULL) {
        if (current->priority == priority) {
            removeNode(queue, previous, current);
            return 1;
        }

        previous = current;
        current = current->next;
    }

    printf("No message with priority %d found.\n",
        priority);
    return 0;
}

// Извлечение с приоритетом не ниже заданного
int dequeueMinPriority(PriorityQueue* queue,
    int minPriority) {
    Message* current = queue->head;
    Message* previous = NULL;

    while (current != NULL) {
        if (current->priority >= minPriority) {
            removeNode(queue, previous, current);
            return 1;
        }

        previous = current;
        current = current->next;
    }

    printf("No message with priority >= %d found.\n",
        minPriority);
    return 0;
}

// Вывод содержимого очереди
void printQueue(const PriorityQueue* queue) {
    if (queue->head == NULL) {
        printf("\nQueue is empty!\n");
        return;
    }

    printf("\n========== PRIORITY QUEUE ==========\n");

    const Message* current = queue->head;

    while (current != NULL) {
        printf("ID: %d | Priority: %3d | Text: %s\n",
            current->id,
            current->priority,
            current->text);

        current = current->next;
    }

    printf("------------------------------------\n");
    printf("Total messages: %d\n", queue->count);
}

// Генерация сообщений со случайными приоритетами
void generateMessages(PriorityQueue* queue,
    int amount, int* nextId) {
    char text[MAX_TEXT];

    for (int i = 0; i < amount; i++) {
        int priority = rand() % 256;

        snprintf(text, sizeof(text),
            "Generated message %d", *nextId);

        if (enqueue(queue, *nextId, priority, text)) {
            (*nextId)++;
        }
        else {
            break;
        }
    }

    printf("\nMessage generation finished.\n");
}

// Очистка очереди и освобождение памяти
void clearQueue(PriorityQueue* queue) {
    Message* current = queue->head;

    while (current != NULL) {
        Message* next = current->next;
        free(current);
        current = next;
    }

    queue->head = NULL;
    queue->count = 0;
}

// Проверка ввода целого числа
int readInt(const char* prompt, int* value) {
    char buffer[128];
    char extra;

    printf("%s", prompt);

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }

    if (sscanf(buffer, " %d %c", value, &extra) != 1) {
        printf("Invalid number!\n");
        return 0;
    }

    return 1;
}

// Главное меню программы
int main(void) {
    PriorityQueue queue;
    int choice;
    int priority;
    int amount;
    int nextId = 1;
    char text[MAX_TEXT];

    initQueue(&queue);
    srand((unsigned int)time(NULL));

    while (1) {
        printf("\n========== PRIORITY QUEUE ==========\n");
        printf("1. Add message\n");
        printf("2. Generate random messages\n");
        printf("3. Extract first message\n");
        printf("4. Extract by exact priority\n");
        printf("5. Extract by minimum priority\n");
        printf("6. Show all messages\n");
        printf("7. Clear queue\n");
        printf("0. Exit\n");
        printf("====================================\n");

        if (!readInt("Enter your choice: ", &choice)) {
            if (feof(stdin)) break;
            continue;
        }

        switch (choice) {
        case 1:
            if (!readInt("Enter priority (0-255): ",
                &priority)) {
                break;
            }

            if (priority < 0 || priority > 255) {
                printf("Priority must be 0-255!\n");
                break;
            }

            printf("Enter message text: ");

            if (fgets(text, sizeof(text), stdin) == NULL) {
                break;
            }

            text[strcspn(text, "\n")] = '\0';

            if (enqueue(&queue, nextId, priority, text)) {
                printf("Message added successfully!\n");
                nextId++;
            }
            break;

        case 2:
            if (!readInt("Number of messages: ", &amount)) {
                break;
            }

            if (amount <= 0 || amount > 10000) {
                printf("Enter a number from 1 to 10000.\n");
                break;
            }

            generateMessages(&queue, amount, &nextId);
            break;

        case 3:
            dequeueFirst(&queue);
            break;

        case 4:
            if (readInt("Enter exact priority: ",
                &priority)) {
                if (priority >= 0 && priority <= 255) {
                    dequeuePriority(&queue, priority);
                }
                else {
                    printf("Priority must be 0-255!\n");
                }
            }
            break;

        case 5:
            if (readInt("Enter minimum priority: ",
                &priority)) {
                if (priority >= 0 && priority <= 255) {
                    dequeueMinPriority(&queue, priority);
                }
                else {
                    printf("Priority must be 0-255!\n");
                }
            }
            break;

        case 6:
            printQueue(&queue);
            break;

        case 7:
            clearQueue(&queue);
            printf("Queue cleared!\n");
            break;

        case 0:
            clearQueue(&queue);
            printf("Program finished.\n");
            return 0;

        default:
            printf("Invalid menu option!\n");
        }
    }

    clearQueue(&queue);
    return 0;
}
