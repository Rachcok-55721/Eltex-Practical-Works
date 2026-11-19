#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>

#define QUEUE_KEY ((key_t)0x33433201)
#define MAX_TEXT 2048
#define MAX_TOPIC 128

// Сообщение System V: тип определяет получателя, text содержит данные.
typedef struct {
    long mtype;
    char text[MAX_TEXT];
} Message;

// Односвязный список подписок: PID и интересующая тема.
typedef struct Subscriber {
    pid_t pid;
    char *topic;
    struct Subscriber *next;
} Subscriber;

// Список издателей для уведомления о завершении работы.
typedef struct Publisher {
    pid_t pid;
    struct Publisher *next;
} Publisher;

static volatile sig_atomic_t stop_requested = 0;

// Обработчик сигнала устанавливает флаг завершения.
static void on_signal(int signum) {
    (void)signum;
    stop_requested = 1;
}

static void install_signals(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

static void pause_briefly(void) {
    struct timespec ts = {0, 100000000L};
    nanosleep(&ts, NULL);
}

static int valid_topic(const char *topic) {
    return topic && *topic && strlen(topic) < MAX_TOPIC &&
           !strpbrk(topic, ",\r\n");
}

// Отправка сообщения в очередь System V.
static int send_text(int qid, long type, const char *text, int flags) {
    Message msg;
    size_t len = strlen(text);
    if (len >= sizeof(msg.text)) {
        errno = EMSGSIZE;
        return -1;
    }
    msg.mtype = type;
    memcpy(msg.text, text, len + 1);
    return msgsnd(qid, &msg, len + 1, flags);
}

// Получение сообщения заданного типа без блокировки.
static int receive_text(int qid, long type, Message *msg) {
    ssize_t n = msgrcv(qid, msg, sizeof(msg->text) - 1, type, IPC_NOWAIT | MSG_NOERROR);
    if (n < 0) return -1;
    msg->text[n] = '\0';
    return 0;
}

static int parse_request(char *line, char **action, pid_t *pid, char **topic) {
    char *first = strchr(line, ',');
    if (!first) return -1;
    *first++ = '\0';
    char *second = strchr(first, ',');
    if (!second) return -1;
    *second++ = '\0';
    char *end;
    errno = 0;
    long value = strtol(first, &end, 10);
    if (errno || *end || value <= 1 || value > INT_MAX) return -1;
    *action = line;
    *pid = (pid_t)value;
    *topic = second;
    return 0;
}

static void remember_publisher(Publisher **list, pid_t pid) {
    for (Publisher *p = *list; p; p = p->next)
        if (p->pid == pid) return;
    Publisher *p = malloc(sizeof(*p));
    if (!p) { perror("malloc"); return; }
    p->pid = pid;
    p->next = *list;
    *list = p;
}

// Добавление подписки в односвязный список.
static void subscribe(Subscriber **list, pid_t pid, const char *topic) {
    for (Subscriber *s = *list; s; s = s->next)
        if (s->pid == pid && strcmp(s->topic, topic) == 0) return;
    Subscriber *s = malloc(sizeof(*s));
    if (!s) { perror("malloc"); return; }
    s->topic = strdup(topic);
    if (!s->topic) { free(s); perror("strdup"); return; }
    s->pid = pid;
    s->next = *list;
    *list = s;
    printf("Subscribed: PID %ld -> %s\n", (long)pid, topic);
    fflush(stdout);
}

// Удаление подписки и освобождение памяти.
static void unsubscribe(Subscriber **list, pid_t pid, const char *topic) {
    Subscriber **cur = list;
    while (*cur) {
        Subscriber *s = *cur;
        if (s->pid == pid && strcmp(s->topic, topic) == 0) {
            *cur = s->next;
            free(s->topic);
            free(s);
        } else cur = &s->next;
    }
    printf("Unsubscribed: PID %ld -> %s\n", (long)pid, topic);
    fflush(stdout);
}

// Обработка команд и рассылка сообщений по темам.
static void broker_message(int qid, Message *msg, Subscriber **subs, Publisher **pubs) {
    char *line_end = strchr(msg->text, '\n');
    if (line_end) *line_end++ = '\0';
    char *action, *topic;
    pid_t pid;
    if (parse_request(msg->text, &action, &pid, &topic) < 0) {
        fprintf(stderr, "Broker: invalid request\n");
        return;
    }
    if (strcmp(action, "register") == 0) {
        remember_publisher(pubs, pid);
    } else if (strcmp(action, "subscribe") == 0 && valid_topic(topic)) {
        subscribe(subs, pid, topic);
    } else if (strcmp(action, "unsubscribe") == 0 && valid_topic(topic)) {
        unsubscribe(subs, pid, topic);
    } else if (strcmp(action, "send") == 0 && valid_topic(topic) && line_end) {
        remember_publisher(pubs, pid);
        char delivered[MAX_TEXT];
        int n = snprintf(delivered, sizeof(delivered), "From PID %ld | Topic: %s | Message: %s",
                         (long)pid, topic, line_end);
        if (n < 0 || (size_t)n >= sizeof(delivered)) return;
        printf("Broker received: PID %ld, topic %s\n", (long)pid, topic);
        fflush(stdout);
        for (Subscriber *s = *subs; s; s = s->next) {
            if (strcmp(s->topic, topic) == 0) {
                if (send_text(qid, (long)s->pid, delivered, IPC_NOWAIT) < 0)
                    perror("Broker delivery");
            }
        }
    } else {
        fprintf(stderr, "Broker: unknown or invalid command\n");
    }
}

// Создание очереди и основной цикл брокера.
static int run_broker(void) {
    int qid = msgget(QUEUE_KEY, IPC_CREAT | IPC_EXCL | 0600);
    if (qid < 0) {
        if (errno == EEXIST)
            fprintf(stderr, "Broker already running (or queue exists).\n");
        else perror("msgget");
        return 1;
    }
    install_signals();
    Subscriber *subs = NULL;
    Publisher *pubs = NULL;
    printf("Broker started. PID: %ld. Press Ctrl+C to stop.\n", (long)getpid());
    fflush(stdout);
    while (!stop_requested) {
        Message msg;
        if (receive_text(qid, 1, &msg) == 0) {
            broker_message(qid, &msg, &subs, &pubs);
        } else if (errno == ENOMSG || errno == EINTR) {
            pause_briefly();
        } else {
            perror("Broker msgrcv");
            break;
        }
    }
    puts("Broker shutting down...");
    for (Publisher *p = pubs; p; p = p->next) kill(p->pid, SIGINT);
    for (Subscriber *s = subs; s; s = s->next) kill(s->pid, SIGINT);
    for (int i = 0; i < 20; ++i) pause_briefly();
    if (msgctl(qid, IPC_RMID, NULL) < 0) perror("msgctl IPC_RMID");
    while (subs) { Subscriber *next = subs->next; free(subs->topic); free(subs); subs = next; }
    while (pubs) { Publisher *next = pubs->next; free(pubs); pubs = next; }
    puts("Queue removed. Broker stopped.");
    return 0;
}

static int connect_queue(void) {
    int qid = msgget(QUEUE_KEY, 0600);
    if (qid < 0) perror("Broker queue not available");
    return qid;
}

static int send_command(int qid, const char *action, const char *topic, const char *payload) {
    char text[MAX_TEXT];
    int n = snprintf(text, sizeof(text), "%s,%ld,%s%s%s", action, (long)getpid(), topic,
                     payload ? "\n" : "", payload ? payload : "");
    if (n < 0 || (size_t)n >= sizeof(text)) { errno = EMSGSIZE; return -1; }
    return send_text(qid, 1, text, IPC_NOWAIT);
}

// Регистрация издателя и отправка сообщений пользователя.
static int run_publisher(const char *topic) {
    if (!valid_topic(topic)) { fprintf(stderr, "Invalid topic.\n"); return 1; }
    int qid = connect_queue();
    if (qid < 0) return 1;
    install_signals();
    if (send_command(qid, "register", topic, NULL) < 0) { perror("register"); return 1; }
    printf("Publisher PID %ld. Topic: %s\nType messages (Ctrl+D to exit):\n", (long)getpid(), topic);
    fflush(stdout);
    char line[MAX_TEXT - MAX_TOPIC - 80];
    while (!stop_requested) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        struct timeval timeout = {0, 200000};
        int ready = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &timeout);
        if (ready < 0) {
            if (errno == EINTR) continue;
            perror("select"); return 1;
        }
        if (!ready) {
            struct msqid_ds info;
            if (msgctl(qid, IPC_STAT, &info) < 0) {
                fprintf(stderr, "Queue unavailable. Publisher exiting.\n");
                return 1;
            }
            continue;
        }
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = '\0';
        if (!*line) continue;
        if (send_command(qid, "send", topic, line) < 0) {
            perror("Publisher msgsnd"); return 1;
        }
        puts("Sent.");
    }
    puts("Publisher stopped.");
    return 0;
}

// Подписка на темы и получение адресованных сообщений.
static int run_subscriber(int count, char **topics) {
    for (int i = 0; i < count; ++i)
        if (!valid_topic(topics[i])) { fprintf(stderr, "Invalid topic: %s\n", topics[i]); return 1; }
    int qid = connect_queue();
    if (qid < 0) return 1;
    install_signals();
    for (int i = 0; i < count; ++i) {
        if (send_command(qid, "subscribe", topics[i], NULL) < 0) {
            perror("subscribe"); return 1;
        }
    }
    printf("Subscriber PID %ld. Listening on %d topic(s). Ctrl+C to exit.\n",
           (long)getpid(), count);
    fflush(stdout);
    int result = 0;
    while (!stop_requested) {
        Message msg;
        if (receive_text(qid, (long)getpid(), &msg) == 0) {
            printf("Received: %s\n", msg.text);
            fflush(stdout);
        } else if (errno == ENOMSG || errno == EINTR) {
            pause_briefly();
        } else {
            fprintf(stderr, "Queue unavailable. Subscriber exiting.\n");
            result = 1;
            break;
        }
    }
    for (int i = 0; i < count; ++i)
        if (send_command(qid, "unsubscribe", topics[i], NULL) < 0 &&
            errno != EIDRM && errno != EINVAL)
            perror("unsubscribe");
    puts("Subscriber stopped.");
    return result;
}

// Выбор роли программы по аргументам командной строки.
int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "-b") == 0) return run_broker();
    if (argc == 3 && strcmp(argv[1], "-p") == 0) return run_publisher(argv[2]);
    if (argc >= 3 && strcmp(argv[1], "-s") == 0) return run_subscriber(argc - 2, argv + 2);
    fprintf(stderr, "Usage:\n  %s -b\n  %s -p <topic>\n  %s -s <topic1> [topic2 ...]\n", argv[0], argv[0], argv[0]);
    return 1;
}
