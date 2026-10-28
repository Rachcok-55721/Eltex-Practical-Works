#define _CRT_SECURE_NO_WARNINGS

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <windows.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_LEN 512

// Текущие права доступа
unsigned int currentMode = 0;
int hasMode = 0;

// Ввод строки
int inputString(char* str, int size) {
    if (fgets(str, size, stdin) == NULL)
        return 0;

    size_t len = strlen(str);

    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
    else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
    }

    return 1;
}

// Перевод прав в буквенный формат
void toSymbolic(unsigned int mode, char result[10]) {
    const char symbols[] = "rwxrwxrwx";

    for (int i = 0; i < 9; i++) {
        unsigned int bit = 1u << (8 - i);

        result[i] = (mode & bit) ? symbols[i] : '-';
    }

    result[9] = '\0';
}

// Вывод прав доступа
void showPermissions(unsigned int mode) {
    char symbolic[10];

    toSymbolic(mode, symbolic);

    printf("\n===== ПРАВА ДОСТУПА =====\n");
    printf("Буквенное представление: %s\n", symbolic);
    printf("Цифровое представление: %03o\n", mode & 0777);

    printf("Битовое представление: ");

    for (int i = 8; i >= 0; i--) {
        printf("%u", (mode >> i) & 1u);

        if (i == 6 || i == 3)
            printf(" ");
    }

    printf("\n=========================\n");
}

// Проверка цифрового формата
int parseNumeric(const char* text, unsigned int* mode) {
    if (strlen(text) != 3)
        return 0;

    unsigned int value = 0;

    for (int i = 0; i < 3; i++) {
        if (text[i] < '0' || text[i] > '7')
            return 0;

        value = value * 8 + (text[i] - '0');
    }

    *mode = value;
    return 1;
}

// Проверка буквенного формата
int parseSymbolic(const char* text, unsigned int* mode) {
    if (strlen(text) != 9)
        return 0;

    const char expected[] = "rwxrwxrwx";
    unsigned int value = 0;

    for (int i = 0; i < 9; i++) {
        if (text[i] == expected[i]) {
            value |= 1u << (8 - i);
        }
        else if (text[i] != '-') {
            return 0;
        }
    }

    *mode = value;
    return 1;
}

// Ввод прав вручную
void enterPermissions() {
    char input[MAX_LEN];
    unsigned int mode;

    printf("\n===== ВВОД ПРАВ ДОСТУПА =====\n");
    printf("Примеры: 755, 644, rwxr-xr-x\n");
    printf("Введите права доступа: ");

    if (!inputString(input, MAX_LEN))
        return;

    if (!parseNumeric(input, &mode) &&
        !parseSymbolic(input, &mode)) {

        printf("\nОшибка: неверный формат прав!\n");
        return;
    }

    currentMode = mode;
    hasMode = 1;

    showPermissions(currentMode);
}

// Получение прав файла через stat
void readFilePermissions() {
    char filename[MAX_LEN];
    struct stat fileInfo;

    printf("\n===== ПРАВА ДОСТУПА К ФАЙЛУ =====\n");
    printf("Введите имя или путь к файлу: ");

    if (!inputString(filename, MAX_LEN))
        return;

    if (stat(filename, &fileInfo) != 0) {
        perror("Ошибка получения информации о файле");
        return;
    }

    currentMode = fileInfo.st_mode & 0777;
    hasMode = 1;

    printf("\nФайл: %s\n", filename);
    showPermissions(currentMode);

    printf("Для сравнения выполните команду:\n");
    printf("ls -l -- \"%s\"\n", filename);
}

// Изменение прав по команде chmod
int applyModification(const char* text, unsigned int* mode) {
    unsigned int who = 0;
    int i = 0;
    int specified = 0;

    // Определение категории пользователей
    while (text[i] == 'u' || text[i] == 'g' ||
        text[i] == 'o' || text[i] == 'a') {

        specified = 1;

        switch (text[i]) {
        case 'u': who |= 0700; break;
        case 'g': who |= 0070; break;
        case 'o': who |= 0007; break;
        case 'a': who |= 0777; break;
        }

        i++;
    }

    if (!specified)
        who = 0777;

    // Определение действия
    char operation = text[i];

    if (operation != '+' &&
        operation != '-' &&
        operation != '=')
        return 0;

    i++;

    unsigned int permissions = 0;

    // Определение прав доступа
    while (text[i] != '\0') {
        unsigned int bits = 0;

        switch (text[i]) {
        case 'r': bits = 0444; break;
        case 'w': bits = 0222; break;
        case 'x': bits = 0111; break;
        default: return 0;
        }

        permissions |= bits;
        i++;
    }

    if (operation != '=' && permissions == 0)
        return 0;

    permissions &= who;

    // Применение изменений к маске
    if (operation == '+') {
        *mode |= permissions;
    }
    else if (operation == '-') {
        *mode &= ~permissions;
    }
    else {
        *mode = (*mode & ~who) | permissions;
    }

    *mode &= 0777;

    return 1;
}

// Изменение текущих прав
void modifyPermissions() {
    if (!hasMode) {
        printf("\nСначала задайте права доступа "
            "или выберите файл!\n");
        return;
    }

    char input[MAX_LEN];
    unsigned int newMode = currentMode;

    printf("\n===== ИЗМЕНЕНИЕ ПРАВ =====\n");
    printf("Текущие права доступа:\n");

    showPermissions(currentMode);

    printf("\nПримеры команд:\n");
    printf("u+x - добавить выполнение владельцу\n");
    printf("g-w - убрать запись у группы\n");
    printf("o=r - оставить остальным только чтение\n");
    printf("a+r - добавить чтение всем\n");
    printf("755 - установить права 755\n");

    printf("\nВведите команду: ");

    if (!inputString(input, MAX_LEN))
        return;

    if (!parseNumeric(input, &newMode)) {
        if (!applyModification(input, &newMode)) {
            printf("\nОшибка: неверная команда!\n");
            return;
        }
    }

    currentMode = newMode;

    printf("\nПрава успешно изменены в памяти!\n");
    printf("Реальные права файла не изменялись.\n");

    showPermissions(currentMode);
}

// Главное меню

int main(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    setlocale(LC_ALL, "Russian_Russia.1251");

    char input[MAX_LEN];
    int choice;
    char extra;

    while (1) {
        printf("\n========== ПРАВА ДОСТУПА ==========\n");
        printf("1. Ввести права доступа вручную\n");
        printf("2. Получить права доступа к файлу\n");
        printf("3. Изменить права доступа\n");
        printf("4. Показать текущие права\n");
        printf("0. Выход\n");
        printf("===================================\n");
        printf("Ваш выбор: ");

        if (!inputString(input, MAX_LEN))
            break;

        if (sscanf(input, " %d %c", &choice, &extra) != 1) {
            printf("\nОшибка: неверный пункт меню!\n");
            continue;
        }

        switch (choice) {
        case 1:
            enterPermissions();
            break;

        case 2:
            readFilePermissions();
            break;

        case 3:
            modifyPermissions();
            break;

        case 4:
            if (hasMode) {
                showPermissions(currentMode);
            }
            else {
                printf("\nПрава доступа не заданы!\n");
            }
            break;

        case 0:
            printf("\nЗавершение программы.\n");
            return 0;

        default:
            printf("\nНеверный пункт меню!\n");
            break;
        }
    }

    return 0;
}

