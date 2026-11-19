#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX_TEXT 512
#define QUEUE_DEPTH 10
#define NORMAL_PRIORITY 1u
#define EXIT_PRIORITY 10u

static mqd_t incoming = (mqd_t)-1;
static mqd_t outgoing = (mqd_t)-1;
static char queue1[256], queue2[256];
static volatile sig_atomic_t stop_requested = 0;
static pthread_t receiver_thread;
static int owner = 0;
static int peer_left = 0;
static pthread_mutex_t state_lock = PTHREAD_MUTEX_INITIALIZER;

// Запрос завершения по Ctrl+C.
static void on_sigint(int signum) {
    (void)signum;
    stop_requested = 1;
}

static void set_peer_left(void) {
    pthread_mutex_lock(&state_lock);
    peer_left = 1;
    pthread_mutex_unlock(&state_lock);
}

static int has_peer_left(void) {
    int result;
    pthread_mutex_lock(&state_lock);
    result = peer_left;
    pthread_mutex_unlock(&state_lock);
    return result;
}

// Приём сообщений в отдельном потоке.
static void *receive_messages(void *unused) {
    (void)unused;
    char buffer[MAX_TEXT + 1];
    while (!stop_requested) {
        struct timespec deadline;
        if (clock_gettime(CLOCK_REALTIME, &deadline) == -1) break;
        deadline.tv_sec += 1;
        unsigned priority = 0;
        ssize_t n = mq_timedreceive(incoming, buffer, MAX_TEXT, &priority, &deadline);
        if (n < 0) {
            if (errno == EINTR || errno == ETIMEDOUT) continue;
            perror("mq_timedreceive");
            stop_requested = 1;
            break;
        }
        buffer[n] = '\0';
        if (priority == EXIT_PRIORITY) {
            printf("\nPeer disconnected. Press Enter to exit.\n");
            fflush(stdout);
            set_peer_left();
            stop_requested = 1;
            break;
        }
        printf("\nPeer: %s\nYou: ", buffer);
        fflush(stdout);
    }
    return NULL;
}

// Отправка сообщения с ограничением времени ожидания.
static int send_message(const char *message, unsigned priority) {
    struct timespec deadline;
    if (clock_gettime(CLOCK_REALTIME, &deadline) == -1) return -1;
    deadline.tv_sec += 2;
    if (mq_timedsend(outgoing, message, strlen(message) + 1, priority, &deadline) == -1) {
        perror("mq_timedsend");
        return -1;
    }
    return 0;
}

// Создание очередей первым участником или подключение вторым.
static int connect_queues(void) {
    struct mq_attr attr = {0};
    attr.mq_maxmsg = QUEUE_DEPTH;
    attr.mq_msgsize = MAX_TEXT;

    mqd_t first = mq_open(queue1, O_CREAT | O_EXCL | O_RDWR, 0600, &attr);
    if (first != (mqd_t)-1) {
        owner = 1;
        mqd_t second = mq_open(queue2, O_CREAT | O_EXCL | O_RDWR, 0600, &attr);
        if (second == (mqd_t)-1) {
            perror("mq_open second queue");
            mq_close(first);
            mq_unlink(queue1);
            return -1;
        }
        incoming = first;
        outgoing = second;
        printf("Created queues. Waiting for peer...\n");
        return 0;
    }
    if (errno != EEXIST) {
        perror("mq_open first queue");
        return -1;
    }
    incoming = mq_open(queue2, O_RDONLY);
    if (incoming == (mqd_t)-1) {
        perror("mq_open incoming");
        return -1;
    }
    outgoing = mq_open(queue1, O_WRONLY);
    if (outgoing == (mqd_t)-1) {
        perror("mq_open outgoing");
        mq_close(incoming);
        incoming = (mqd_t)-1;
        return -1;
    }
    printf("Connected to existing queues.\n");
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2 || argv[1][0] != '/' || strlen(argv[1]) < 2 ||
        strchr(argv[1] + 1, '/') || strlen(argv[1]) > 250) {
        fprintf(stderr, "Usage: %s /chat_name\n", argv[0]);
        return EXIT_FAILURE;
    }
    snprintf(queue1, sizeof queue1, "%s_1", argv[1]);
    snprintf(queue2, sizeof queue2, "%s_2", argv[1]);

    struct sigaction sa = {0};
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    if (connect_queues() != 0) return EXIT_FAILURE;

    int rc = pthread_create(&receiver_thread, NULL, receive_messages, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(rc));
        goto cleanup;
    }

    printf("Chat started. Type /exit or press Ctrl+C to leave.\n");
    char *line = NULL;
    size_t capacity = 0;
    while (!stop_requested) {
        printf("You: ");
        fflush(stdout);
        struct pollfd input = { .fd = STDIN_FILENO, .events = POLLIN };
        int ready;
        do {
            ready = poll(&input, 1, 250);
        } while (ready == 0 && !stop_requested);
        if (stop_requested) break;
        if (ready < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }
        errno = 0;
        ssize_t n = getline(&line, &capacity, stdin);
        if (n < 0) {
            if (errno == EINTR && !stop_requested) {
                clearerr(stdin);
                continue;
            }
            break;
        }
        if (stop_requested) break;
        if (n && line[n - 1] == '\n') line[--n] = '\0';
        if (strcmp(line, "/exit") == 0) break;
        if (n == 0) continue;
        if (n >= MAX_TEXT) {
            fprintf(stderr, "Message too long (max %d bytes).\n", MAX_TEXT - 1);
            continue;
        }
        if (send_message(line, NORMAL_PRIORITY) != 0) break;
    }
    free(line);
    if (!has_peer_left()) send_message("EXIT", EXIT_PRIORITY);
    stop_requested = 1;
    pthread_join(receiver_thread, NULL);

cleanup:
    if (incoming != (mqd_t)-1) mq_close(incoming);
    if (outgoing != (mqd_t)-1) mq_close(outgoing);
    if (owner) {
        mq_unlink(queue1);
        mq_unlink(queue2);
        printf("Queues removed.\n");
    }
    printf("Chat stopped.\n");
    return EXIT_SUCCESS;
}
