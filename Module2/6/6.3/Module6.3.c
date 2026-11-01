#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "plugin_api.h"

#define PLUGIN_DIR "plugins"
#define MAX_PLUGINS 100

typedef struct {
    char filename[MAX_PATH];
    char name[100];
    int arguments;
    int enabled;
    Operation function;
    HMODULE library;
} Command;

static Command commands[MAX_PLUGINS];
static int commandCount = 0;

// Безопасный ввод целого числа
static int inputInt(void) {
    char buffer[128], extra;
    int value;
    for (;;) {
        if (!fgets(buffer, sizeof(buffer), stdin)) exit(0);
        if (sscanf(buffer, " %d %c", &value, &extra) == 1) return value;
        printf("Invalid input. Enter an integer: ");
    }
}

// Безопасный ввод вещественного числа
static double inputDouble(void) {
    char buffer[128], extra;
    double value;
    for (;;) {
        if (!fgets(buffer, sizeof(buffer), stdin)) exit(0);
        if (sscanf(buffer, " %lf %c", &value, &extra) == 1 && isfinite(value)) return value;
        printf("Invalid input. Enter a number: ");
    }
}

// Поиск и загрузка DLL из каталога plugins
static void loadPlugins(void) {
    WIN32_FIND_DATAA data;
    HANDLE search = FindFirstFileA(PLUGIN_DIR "\\*.dll", &data);
    if (search == INVALID_HANDLE_VALUE) {
        printf("No DLL plugins found in '%s'.\n", PLUGIN_DIR);
        return;
    }
    do {
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (commandCount >= MAX_PLUGINS) {
            printf("Plugin limit reached.\n");
            break;
        }
        char path[MAX_PATH];
        int length = snprintf(path, sizeof(path), ".\\%s\\%s", PLUGIN_DIR, data.cFileName);
        if (length < 0 || length >= (int)sizeof(path)) continue;

        HMODULE library = LoadLibraryA(path);
        if (!library) {
            printf("Cannot load: %s (error %lu)\n", data.cFileName, GetLastError());
            continue;
        }
        GetPluginInfo getInfo = (GetPluginInfo)(void*)GetProcAddress(library, "getPluginInfo");
        if (!getInfo) {
            printf("Skipped %s: missing getPluginInfo.\n", data.cFileName);
            FreeLibrary(library);
            continue;
        }
        const PluginInfo* info = getInfo();
        if (!info || !info->name || !info->function ||
            (info->arguments != 1 && info->arguments != 2) ||
            strlen(info->name) >= sizeof(commands[0].name)) {
            printf("Skipped %s: invalid plugin.\n", data.cFileName);
            FreeLibrary(library);
            continue;
        }
        Command* command = &commands[commandCount++];
        snprintf(command->filename, sizeof(command->filename), "%s", data.cFileName);
        snprintf(command->name, sizeof(command->name), "%s", info->name);
        command->arguments = info->arguments;
        command->enabled = 1;
        command->function = info->function;
        command->library = library;
        printf("Loaded: %s (%s)\n", command->name, command->filename);
    } while (FindNextFileA(search, &data));
    FindClose(search);
}

// Вывод меню загруженных операций
static void showMenu(void) {
    printf("\n========== CALCULATOR ==========\n");
    for (int i = 0; i < commandCount; i++) {
        printf("%d. %s%s\n", i + 1, commands[i].name,
            commands[i].enabled ? "" : " [OFF]");
    }
    printf("%d. Manage commands\n", commandCount + 1);
    printf("0. Exit\n================================\n");
}

// Включение и отключение операций
static void manageCommands(void) {
    for (;;) {
        printf("\n===== MANAGE COMMANDS =====\n");
        for (int i = 0; i < commandCount; i++) {
            printf("%d. %s [%s]\n", i + 1, commands[i].name,
                commands[i].enabled ? "ON" : "OFF");
        }
        printf("0. Back\nSelect command to toggle: ");
        int choice = inputInt();
        if (choice == 0) return;
        if (choice < 1 || choice > commandCount) {
            printf("Invalid choice!\n");
            continue;
        }
        Command* command = &commands[choice - 1];
        command->enabled = !command->enabled;
        printf("%s is now %s.\n", command->name,
            command->enabled ? "ON" : "OFF");
    }
}

// Выполнение операции через указатель из DLL
static void executeCommand(Command* command) {
    double a, b = 0.0;
    printf("\n===== %s =====\n", command->name);
    printf("Enter first number: ");
    a = inputDouble();
    if (command->arguments == 2) {
        printf("Enter second number: ");
        b = inputDouble();
    }
    if (strcmp(command->name, "Division") == 0 && b == 0.0) {
        printf("Error: Division by zero!\n");
        return;
    }
    if (strcmp(command->name, "Square root") == 0 && a < 0.0) {
        printf("Error: Negative square root!\n");
        return;
    }
    double result = command->function(a, b);
    if (!isfinite(result)) {
        printf("Error: Invalid mathematical result!\n");
        return;
    }
    printf("Result: %.2f\n", result);
}

// Освобождение загруженных библиотек
static void unloadPlugins(void) {
    for (int i = 0; i < commandCount; i++) FreeLibrary(commands[i].library);
    commandCount = 0;
}

int main(void) {
    printf("Searching for DLL plugins...\n");
    loadPlugins();
    printf("Loaded plugins: %d\n", commandCount);
    for (;;) {
        showMenu();
        printf("Enter your choice: ");
        int choice = inputInt();
        if (choice == 0) break;
        if (choice == commandCount + 1) {
            manageCommands();
        }
        else if (choice >= 1 && choice <= commandCount) {
            if (commands[choice - 1].enabled) executeCommand(&commands[choice - 1]);
            else printf("This command is disabled.\n");
        }
        else {
            printf("Invalid choice. Try again.\n");
        }
    }
    unloadPlugins();
    printf("Exiting calculator. Goodbye!\n");
    return 0;
}
