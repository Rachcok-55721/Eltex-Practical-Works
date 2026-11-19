#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define SHM_BYTES 16384u
#define MAX_VALUES 32u
#define MAGIC 0x53484d34u
#define KEY_SHM ((key_t)0x33440101)
#define KEY_SEM ((key_t)0x33440102)
#define PRODUCER_PAUSE_MS 60
#define DEFAULT_CONSUMER_PAUSE_MS 500

// Заголовок блока: количество чисел и смещение следующего блока.
typedef struct {
    uint32_t count;
    uint32_t next;
    uint32_t capacity;
    uint32_t reserved;
} Block;

// Служебные данные общей памяти и счётчики обработки.
typedef struct {
    uint32_t magic;
    uint32_t used;
    uint32_t first;
    uint32_t last;
    uint32_t total;
    uint32_t processed;
    uint32_t finished;
    uint32_t producer_pid;
} Shared;

static volatile sig_atomic_t stop_requested = 0;

static void on_signal(int sig) { (void)sig; stop_requested = 1; }

static void pause_ms(unsigned ms) {
    struct timespec ts = { ms / 1000, (long)(ms % 1000) * 1000000L };
    while (nanosleep(&ts, &ts) < 0 && errno == EINTR && !stop_requested) {}
}

// Захват семафора для безопасной работы с общей памятью.
static int lock_sem(int id) {
    struct sembuf op = {0, -1, SEM_UNDO};
    while (semop(id, &op, 1) < 0) {
        if (errno == EINTR) { if (stop_requested) return -1; continue; }
        return -1;
    }
    return 0;
}

// Освобождение семафора после критической секции.
static int unlock_sem(int id) {
    struct sembuf op = {0, 1, SEM_UNDO};
    while (semop(id, &op, 1) < 0) {
        if (errno == EINTR) continue;
        return -1;
    }
    return 0;
}

static uint32_t align4(uint32_t n) { return (n + 3u) & ~3u; }

// Получение адреса блока по смещению от начала общей памяти.
static Block *block_at(Shared *s, uint32_t offset) {
    return (Block *)((unsigned char *)s + offset);
}

// Проверка корректности границ и размера блока.
static int valid_block(Shared *s, uint32_t offset) {
    if (offset < sizeof(Shared) || offset > s->used || s->used - offset < sizeof(Block)) return 0;
    Block *b = block_at(s, offset);
    return b->capacity >= 1 && b->capacity <= MAX_VALUES &&
           b->count <= b->capacity &&
           s->used - offset >= sizeof(Block) + b->capacity * sizeof(int32_t);
}

// Производитель создаёт ресурсы IPC и записывает массивы чисел.
static void producer(void) {
    int shmid = shmget(KEY_SHM, SHM_BYTES, IPC_CREAT | IPC_EXCL | 0600);
    if (shmid < 0) { perror("shmget (producer already running or stale segment)"); exit(EXIT_FAILURE); }
    int semid = semget(KEY_SEM, 1, IPC_CREAT | IPC_EXCL | 0600);
    if (semid < 0) {
        perror("semget"); shmctl(shmid, IPC_RMID, NULL); exit(EXIT_FAILURE);
    }
    if (semctl(semid, 0, SETVAL, 1) < 0) {
        perror("semctl SETVAL"); semctl(semid, 0, IPC_RMID); shmctl(shmid, IPC_RMID, NULL); exit(EXIT_FAILURE);
    }
    Shared *s = shmat(shmid, NULL, 0);
    if (s == (void *)-1) {
        perror("shmat"); semctl(semid, 0, IPC_RMID); shmctl(shmid, IPC_RMID, NULL); exit(EXIT_FAILURE);
    }
    memset(s, 0, SHM_BYTES);
    s->magic = MAGIC;
    s->used = align4((uint32_t)sizeof(Shared));
    s->producer_pid = (uint32_t)getpid();
    srand((unsigned)(time(NULL) ^ getpid()));
    printf("Producer PID %ld started. Shared memory: %u bytes.\n", (long)getpid(), SHM_BYTES);
    fflush(stdout);
    while (!stop_requested) {
        if (lock_sem(semid) < 0) { perror("semop lock"); break; }
        uint32_t available = SHM_BYTES - s->used;
        if (available < sizeof(Block) + sizeof(int32_t)) {
            s->finished = 1;
            unlock_sem(semid);
            printf("Memory full. Waiting for consumers...\n");
            break;
        }
        // Генерация массива и добавление нового блока в конец списка.
        uint32_t n = 1u + (uint32_t)(rand() % MAX_VALUES);
        uint32_t max_fit = (available - (uint32_t)sizeof(Block)) / (uint32_t)sizeof(int32_t);
        if (n > max_fit) n = max_fit;
        uint32_t offset = s->used;
        Block *b = block_at(s, offset);
        b->count = n;
        b->capacity = n;
        b->next = 0;
        b->reserved = 0;
        int32_t *values = (int32_t *)(b + 1);
        for (uint32_t i = 0; i < n; ++i) values[i] = (int32_t)(rand() % 2001) - 1000;
        if (s->last) block_at(s, s->last)->next = offset;
        else s->first = offset;
        s->last = offset;
        s->used += (uint32_t)sizeof(Block) + n * (uint32_t)sizeof(int32_t);
        ++s->total;
        printf("Produced set #%u: %u numbers, offset %u, used %u/%u\n",
               s->total, n, offset, s->used, SHM_BYTES);
        fflush(stdout);
        unlock_sem(semid);
        pause_ms(PRODUCER_PAUSE_MS);
    }
    if (lock_sem(semid) == 0) { s->finished = 1; unlock_sem(semid); }
    while (!stop_requested) {
        if (lock_sem(semid) < 0) break;
        uint32_t done = s->processed, total = s->total;
        unlock_sem(semid);
        if (done == total) { printf("All %u sets processed.\n", total); break; }
        pause_ms(PRODUCER_PAUSE_MS);
    }
    // Ожидание отключения потребителей перед удалением ресурсов.
    for (int attempts = 0; attempts < 100 && !stop_requested; ++attempts) {
        struct shmid_ds info;
        if (shmctl(shmid, IPC_STAT, &info) < 0 || info.shm_nattch <= 1) break;
        pause_ms(PRODUCER_PAUSE_MS);
    }
    printf("Producer cleaning up IPC resources.\n");
    shmdt(s);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
}

// Потребитель ищет необработанный блок и вычисляет минимум и максимум.
static void consumer(unsigned delay_ms) {
    int shmid = shmget(KEY_SHM, 0, 0600);
    if (shmid < 0) { perror("shmget (start producer first)"); exit(EXIT_FAILURE); }
    int semid = semget(KEY_SEM, 1, 0600);
    if (semid < 0) { perror("semget"); exit(EXIT_FAILURE); }
    Shared *s = shmat(shmid, NULL, 0);
    if (s == (void *)-1) { perror("shmat"); exit(EXIT_FAILURE); }
    if (s->magic != MAGIC) { fprintf(stderr, "Invalid shared memory format.\n"); shmdt(s); exit(EXIT_FAILURE); }
    printf("Consumer PID %ld started. Delay: %u ms.\n", (long)getpid(), delay_ms);
    fflush(stdout);
    while (!stop_requested) {
        if (lock_sem(semid) < 0) { if (!stop_requested) perror("semop lock"); break; }
        uint32_t offset = s->first;
        uint32_t index = 0;
        int found = 0, corrupt = 0;
        while (offset) {
            if (!valid_block(s, offset)) { corrupt = 1; break; }
            Block *b = block_at(s, offset);
            ++index;
            if (b->count) {
                int32_t *values = (int32_t *)(b + 1);
                int32_t min = values[0], max = values[0];
                uint32_t n = b->count;
                for (uint32_t i = 1; i < n; ++i) {
                    if (values[i] < min) min = values[i];
                    if (values[i] > max) max = values[i];
                }
                // Пометка набора как обработанного под защитой семафора.
                b->count = 0;
                ++s->processed;
                printf("Consumer %ld: set #%u, count=%u, min=%d, max=%d\n",
                       (long)getpid(), index, n, min, max);
                fflush(stdout);
                found = 1;
                break;
            }
            if (b->next && b->next <= offset) { corrupt = 1; break; }
            offset = b->next;
        }
        int finished = s->finished && s->processed == s->total;
        unlock_sem(semid);
        if (corrupt) { fprintf(stderr, "Corrupted linked blocks.\n"); break; }
        if (finished) { printf("Consumer %ld: all sets processed.\n", (long)getpid()); break; }
        (void)found;
        pause_ms(delay_ms);
    }
    shmdt(s);
    printf("Consumer %ld stopped.\n", (long)getpid());
}

// Выбор режима: -p — производитель, -c — потребитель.
int main(int argc, char **argv) {
    unsigned delay_ms = DEFAULT_CONSUMER_PAUSE_MS;
    if (argc < 2 || argc > 3 ||
        (strcmp(argv[1], "-p") && strcmp(argv[1], "-c")) ||
        (!strcmp(argv[1], "-p") && argc != 2)) {
        fprintf(stderr, "Usage: %s -p | -c [delay_ms]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (!strcmp(argv[1], "-c") && argc == 3) {
        char *end = NULL;
        errno = 0;
        unsigned long value = strtoul(argv[2], &end, 10);
        if (errno || end == argv[2] || *end || value < 10 || value > 10000) {
            fprintf(stderr, "Consumer delay must be 10..10000 ms.\n");
            return EXIT_FAILURE;
        }
        delay_ms = (unsigned)value;
    }
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    if (!strcmp(argv[1], "-p")) producer();
    else consumer(delay_ms);
    return 0;
}
