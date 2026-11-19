#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BLOCK_SIZE 4096
#define MAX_NAME 4096
#define MSG_FILE 1u
#define MSG_END 2u
#define MSG_READY 3u
#define MSG_RESULT 4u

typedef struct { uint32_t type; uint32_t name_len; uint64_t size; } Header;
typedef struct { uint32_t type; int32_t status; } Reply;

// Передача указанного количества байтов.
static int write_all(int fd, const void *buf, size_t len) {
    const unsigned char *p = buf;
    while (len) {
        ssize_t n = write(fd, p, len);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        p += n; len -= (size_t)n;
    }
    return 0;
}

// Чтение указанного количества байтов.
static int read_all(int fd, void *buf, size_t len) {
    unsigned char *p = buf;
    while (len) {
        ssize_t n = read(fd, p, len);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        p += n; len -= (size_t)n;
    }
    return 0;
}

// Создание файла-копии в дочернем процессе.
static int receive_file(int fd, const char *name, uint64_t size) {
    size_t len = strlen(name);
    if (len > SIZE_MAX - 6) return EINVAL;
    char *copy = malloc(len + 6);
    if (!copy) return ENOMEM;
    memcpy(copy, name, len);
    memcpy(copy + len, ".copy", 6);
    int out = open(copy, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    int error = out < 0 ? errno : 0;
    unsigned char buffer[BLOCK_SIZE];
    while (size) {
        size_t chunk = size < BLOCK_SIZE ? (size_t)size : BLOCK_SIZE;
        if (read_all(fd, buffer, chunk) < 0) {
            if (!error) error = EIO;
            break;
        }
        if (out >= 0 && !error && write_all(out, buffer, chunk) < 0) error = errno ? errno : EIO;
        size -= chunk;
    }
    if (out >= 0 && close(out) < 0 && !error) error = errno;
    if (error) fprintf(stderr, "Cannot copy '%s': %s\n", name, strerror(error));
    else printf("Created: %s\n", copy);
    free(copy);
    return error;
}

// Обработка сообщений в дочернем процессе.
static int child_run(int input, int output) {
    int failed = 0;
    for (;;) {
        Reply ready = { MSG_READY, 0 };
        if (write_all(output, &ready, sizeof ready) < 0) return 1;
        Header h;
        if (read_all(input, &h, sizeof h) < 0) return 1;
        if (h.type == MSG_END) break;
        if (h.type != MSG_FILE || h.name_len == 0 || h.name_len > MAX_NAME) return 1;
        char name[MAX_NAME + 1];
        if (read_all(input, name, h.name_len) < 0) return 1;
        name[h.name_len] = '\0';
        if (strlen(name) != h.name_len) return 1;
        int err = receive_file(input, name, h.size);
        Reply result = { MSG_RESULT, err };
        if (write_all(output, &result, sizeof result) < 0) return 1;
        if (err) failed = 1;
    }
    return failed;
}

// Передача файла родительским процессом.
static int send_file(int output, const char *name, int fd, uint64_t size) {
    size_t name_len = strlen(name);
    if (!name_len || name_len > MAX_NAME) return -1;
    Header h = { MSG_FILE, (uint32_t)name_len, size };
    if (write_all(output, &h, sizeof h) < 0 || write_all(output, name, name_len) < 0) return -1;
    unsigned char buffer[BLOCK_SIZE];
    while (size) {
        size_t chunk = size < BLOCK_SIZE ? (size_t)size : BLOCK_SIZE;
        if (read_all(fd, buffer, chunk) < 0 || write_all(output, buffer, chunk) < 0) return -1;
        size -= chunk;
    }
    return 0;
}

// Создание каналов для двустороннего обмена.
static int setup_channels(const char *fifo, int to_child[2], int to_parent[2], char **path1, char **path2) {
    if (!fifo) {
        if (pipe(to_child) < 0) return -1;
        if (pipe(to_parent) < 0) { close(to_child[0]); close(to_child[1]); return -1; }
        return 0;
    }
    size_t len = strlen(fifo);
    *path1 = malloc(len + 5);
    *path2 = malloc(len + 5);
    if (!*path1 || !*path2) return -1;
    snprintf(*path1, len + 5, "%s.p2c", fifo);
    snprintf(*path2, len + 5, "%s.c2p", fifo);
    if (mkfifo(*path1, 0600) < 0) return -1;
    if (mkfifo(*path2, 0600) < 0) { unlink(*path1); return -1; }
    /* O_RDWR не блокируется при открытии FIFO в Linux. */
    int a = open(*path1, O_RDWR);
    int b = open(*path2, O_RDWR);
    if (a < 0 || b < 0) {
        if (a >= 0) close(a);
        if (b >= 0) close(b);
        unlink(*path1); unlink(*path2);
        return -1;
    }
    to_child[0] = a; to_child[1] = dup(a);
    to_parent[0] = b; to_parent[1] = dup(b);
    if (to_child[1] < 0 || to_parent[1] < 0) {
        close(a); close(b);
        if (to_child[1] >= 0) close(to_child[1]);
        if (to_parent[1] >= 0) close(to_parent[1]);
        unlink(*path1); unlink(*path2);
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    const char *fifo = NULL;
    int first = 1;
    if (argc > 1 && strcmp(argv[1], "-p") == 0) {
        if (argc < 4 || !argv[2][0]) {
            fprintf(stderr, "Usage: %s [-p fifo_name] file1 [file2 ...]\n", argv[0]);
            return 1;
        }
        fifo = argv[2]; first = 3;
    }
    if (first >= argc) {
        fprintf(stderr, "Usage: %s [-p fifo_name] file1 [file2 ...]\n", argv[0]);
        return 1;
    }
    int to_child[2], to_parent[2];
    char *path1 = NULL, *path2 = NULL;
    if (setup_channels(fifo, to_child, to_parent, &path1, &path2) < 0) {
        perror("Cannot create channels");
        free(path1); free(path2);
        return 1;
    }
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
        close(to_child[1]); close(to_parent[0]);
        int rc = child_run(to_child[0], to_parent[1]);
        close(to_child[0]); close(to_parent[1]);
        _exit(rc);
    }
    close(to_child[0]); close(to_parent[1]);
    int failed = 0;
    for (int i = first; i < argc; ++i) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) { fprintf(stderr, "Cannot open '%s': %s\n", argv[i], strerror(errno)); failed = 1; continue; }
        struct stat st;
        if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode) || st.st_size < 0) {
            fprintf(stderr, "Not a readable regular file: %s\n", argv[i]);
            close(fd); failed = 1; continue;
        }
        if (strlen(argv[i]) > MAX_NAME - 5) {
            fprintf(stderr, "Filename too long: %s\n", argv[i]);
            close(fd); failed = 1; continue;
        }
        Reply ready, result;
        if (read_all(to_parent[0], &ready, sizeof ready) < 0 || ready.type != MSG_READY ||
            send_file(to_child[1], argv[i], fd, (uint64_t)st.st_size) < 0 ||
            read_all(to_parent[0], &result, sizeof result) < 0 || result.type != MSG_RESULT) {
            fprintf(stderr, "Communication error\n");
            close(fd); failed = 1; break;
        }
        if (result.status) failed = 1;
        close(fd);
    }
    Reply ready;
    if (read_all(to_parent[0], &ready, sizeof ready) == 0 && ready.type == MSG_READY) {
        Header end = { MSG_END, 0, 0 };
        if (write_all(to_child[1], &end, sizeof end) < 0) failed = 1;
    } else failed = 1;
    close(to_child[1]); close(to_parent[0]);
    int status;
    if (waitpid(pid, &status, 0) < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) failed = 1;
    if (fifo) { unlink(path1); unlink(path2); }
    free(path1); free(path2);
    return failed;
}
