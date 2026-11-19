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
        return 0;
    }

    Message* newMessage = malloc(sizeof(Message));

    if (newMessage == NULL) {
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


// Remove a node, optionally copying its contents for main() to display.
void removeNode(PriorityQueue* queue, Message* previous,
                Message* current, Message* extracted) {
    if (extracted) *extracted = *current;
    if (previous == NULL) queue->head = current->next;
    else previous->next = current->next;
    free(current);
    queue->count--;
}

int dequeueFirst(PriorityQueue* queue, Message* extracted) {
    if (!queue->head) return 0;
    removeNode(queue, NULL, queue->head, extracted);
    return 1;
}

int dequeuePriority(PriorityQueue* queue, int priority, Message* extracted) {
    Message* previous = NULL;
    for (Message* current = queue->head; current; current = current->next) {
        if (current->priority == priority) {
            removeNode(queue, previous, current, extracted);
            return 1;
        }
        previous = current;
    }
    return 0;
}

int dequeueMinPriority(PriorityQueue* queue, int minPriority, Message* extracted) {
    Message* previous = NULL;
    for (Message* current = queue->head; current; current = current->next) {
        if (current->priority >= minPriority) {
            removeNode(queue, previous, current, extracted);
            return 1;
        }
        previous = current;
    }
    return 0;
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


int main(void) {
    PriorityQueue queue;
    int choice, priority, amount, nextId = 1;
    char text[MAX_TEXT], buffer[128], extra;
    Message extracted;
    initQueue(&queue);
    srand((unsigned int)time(NULL));
    while (1) {
        printf("\n========== PRIORITY QUEUE ==========\n");
        printf("1. Add message\n2. Generate random messages\n");
        printf("3. Extract first message\n4. Extract by exact priority\n");
        printf("5. Extract by minimum priority\n6. Show all messages\n");
        printf("7. Clear queue\n0. Exit\n====================================\n");
        printf("Enter your choice: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        if (sscanf(buffer, " %d %c", &choice, &extra) != 1) {
            printf("Invalid number!\n"); continue;
        }
        switch (choice) {
        case 1:
            printf("Enter priority (0-255): ");
            if (!fgets(buffer, sizeof(buffer), stdin)) goto cleanup;
            if (sscanf(buffer, " %d %c", &priority, &extra) != 1) {
                printf("Invalid number!\n"); break;
            }
            if (priority < 0 || priority > 255) {
                printf("Priority must be 0-255!\n"); break;
            }
            printf("Enter message text: ");
            if (!fgets(text, sizeof(text), stdin)) goto cleanup;
            text[strcspn(text, "\n")] = '\0';
            if (enqueue(&queue, nextId, priority, text)) {
                printf("Message added successfully!\n"); nextId++;
            } else printf("Memory allocation error!\n");
            break;
        case 2:
            printf("Number of messages: ");
            if (!fgets(buffer, sizeof(buffer), stdin)) goto cleanup;
            if (sscanf(buffer, " %d %c", &amount, &extra) != 1) {
                printf("Invalid number!\n"); break;
            }
            if (amount <= 0 || amount > 10000) {
                printf("Enter a number from 1 to 10000.\n"); break;
            }
            for (int i = 0; i < amount; i++) {
                int randomPriority = rand() % 256;
                snprintf(text, sizeof(text), "Generated message %d", nextId);
                if (!enqueue(&queue, nextId, randomPriority, text)) {
                    printf("Memory allocation error!\n"); break;
                }
                nextId++;
            }
            printf("\nMessage generation finished.\n");
            break;
        case 3:
            if (dequeueFirst(&queue, &extracted)) {
                printf("\nExtracted message:\n");
                printf("ID: %d | Priority: %d | Text: %s\n",
                       extracted.id, extracted.priority, extracted.text);
            } else printf("Queue is empty!\n");
            break;
        case 4:
        case 5: {
            printf("%s", choice == 4 ? "Enter exact priority: " : "Enter minimum priority: ");
            if (!fgets(buffer, sizeof(buffer), stdin)) goto cleanup;
            if (sscanf(buffer, " %d %c", &priority, &extra) != 1) {
                printf("Invalid number!\n"); break;
            }
            if (priority < 0 || priority > 255) {
                printf("Priority must be 0-255!\n"); break;
            }
            int found = choice == 4
                ? dequeuePriority(&queue, priority, &extracted)
                : dequeueMinPriority(&queue, priority, &extracted);
            if (found) {
                printf("\nExtracted message:\n");
                printf("ID: %d | Priority: %d | Text: %s\n",
                       extracted.id, extracted.priority, extracted.text);
            } else if (choice == 4)
                printf("No message with priority %d found.\n", priority);
            else printf("No message with priority >= %d found.\n", priority);
            break;
        }
        case 6:
            if (!queue.head) { printf("\nQueue is empty!\n"); break; }
            printf("\n========== PRIORITY QUEUE ==========\n");
            for (const Message* current = queue.head; current; current = current->next)
                printf("ID: %d | Priority: %3d | Text: %s\n",
                       current->id, current->priority, current->text);
            printf("------------------------------------\n");
            printf("Total messages: %d\n", queue.count);
            break;
        case 7:
            clearQueue(&queue);
            printf("Queue cleared!\n");
            break;
        case 0:
            printf("Program finished.\n");
            goto cleanup;
        default:
            printf("Invalid menu option!\n");
        }
    }
cleanup:
    clearQueue(&queue);
    return 0;
}
