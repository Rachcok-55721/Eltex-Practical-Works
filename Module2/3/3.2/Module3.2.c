
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <locale.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <inttypes.h>

// Преобразование строки в IPv4-адрес
int parseIP(const char* str, uint32_t* ip) {
    unsigned int a, b, c, d;
    char extra;

    if (sscanf(str, "%u.%u.%u.%u%c",
        &a, &b, &c, &d, &extra) != 4) {
        return 0;
    }

    if (a > 255 || b > 255 ||
        c > 255 || d > 255) {
        return 0;
    }

    *ip = ((uint32_t)a << 24) |
        ((uint32_t)b << 16) |
        ((uint32_t)c << 8) |
        (uint32_t)d;

    return 1;
}

// Преобразование числа в IPv4-адрес
void printIP(uint32_t ip) {
    printf("%" PRIu32 ".%" PRIu32 ".%" PRIu32 ".%" PRIu32,
        (ip >> 24) & 255u,
        (ip >> 16) & 255u,
        (ip >> 8) & 255u,
        ip & 255u);
}

// Проверка корректности маски подсети
int isValidMask(uint32_t mask) {
    uint32_t inverted = ~mask;

    return (inverted & (inverted + 1u)) == 0;
}

// Проверка принадлежности одной подсети
int isSameSubnet(uint32_t gateway,
    uint32_t destination,
    uint32_t mask) {

    return (gateway & mask) ==
        (destination & mask);
}

// Генерация случайного 32-битного числа
uint32_t randomIP(void) {
    uint32_t result = 0;

    // Формируем IPv4 из четырёх октетов
    for (int i = 0; i < 4; i++) {
        uint32_t octet = (uint32_t)(rand() % 256);
        result = (result << 8) | octet;
    }

    return result;
}

// Вывод справки
void showUsage(const char* program) {
    printf("Использование:\n");
    printf("%s <IP шлюза> <Маска> <Количество>\n",
        program);

    printf("\nПример:\n");
    printf("%s 192.168.1.1 255.255.255.0 10\n",
        program);
}

int main(int argc, char* argv[]) {
    uint32_t gateway;
    uint32_t mask;
    uint32_t network;

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "");

    unsigned long packets;
    unsigned long localCount = 0;
    unsigned long externalCount = 0;

    char* end;

    // Проверка количества аргументов
    if (argc != 4) {
        printf("Ошибка: неверное количество аргументов!\n\n");
        showUsage(argv[0]);
        return 1;
    }

    // Проверка IP-адреса шлюза
    if (!parseIP(argv[1], &gateway)) {
        printf("Ошибка: неверный IP-адрес шлюза!\n");
        return 1;
    }

    // Проверка маски подсети
    if (!parseIP(argv[2], &mask) ||
        !isValidMask(mask)) {
        printf("Ошибка: неверная маска подсети!\n");
        return 1;
    }

    // Проверка количества пакетов
    errno = 0;
    packets = strtoul(argv[3], &end, 10);

    if (errno == ERANGE ||
        argv[3][0] == '\0' ||
        argv[3][0] == '-' ||
        *end != '\0' ||
        packets == 0 ||
        packets > 1000000UL) {

        printf("Ошибка: количество пакетов должно "
            "быть от 1 до 1000000!\n");
        return 1;
    }

    // Определение адреса сети
    network = gateway & mask;

    // Инициализация генератора
    srand((unsigned int)time(NULL));

    printf("\n========== МАРШРУТИЗАЦИЯ IPv4 ==========\n");

    printf("IP-адрес шлюза: ");
    printIP(gateway);

    printf("\nМаска подсети: ");
    printIP(mask);

    printf("\nАдрес сети: ");
    printIP(network);

    printf("\nКоличество пакетов: %lu\n", packets);

    printf("========================================\n");

    // Обработка пакетов
    for (unsigned long i = 0; i < packets; i++) {
        uint32_t destination = randomIP();

        int local = isSameSubnet(
            gateway, destination, mask
        );

        printf("Пакет %lu | IP назначения: ",
            i + 1);

        printIP(destination);

        if (local) {
            printf(" | Своя подсеть\n");
            localCount++;
        }
        else {
            printf(" | Другая подсеть\n");
            externalCount++;
        }
    }

    // Расчёт статистики
    double localPercent =
        (double)localCount * 100.0 / packets;

    double externalPercent =
        (double)externalCount * 100.0 / packets;

    printf("\n========== СТАТИСТИКА ==========\n");

    printf("Всего пакетов: %lu\n", packets);

    printf("Своя подсеть: %lu (%.2f%%)\n",
        localCount, localPercent);

    printf("Другие подсети: %lu (%.2f%%)\n",
        externalCount, externalPercent);

    printf("================================\n");

    return 0;
}
