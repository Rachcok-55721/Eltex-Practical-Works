#include <dlfcn.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "plugin_api.h"

#define PLUGIN_DIR "plugins"
#define MAX_PLUGINS 100
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct {
    char filename[PATH_MAX];
    char name[100];
    int arguments;
    int enabled;
    Operation function;
    void* library;
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

// Поиск и загрузка .so из каталога plugins
static void loadPlugins(void) {
    DIR* directory = opendir(PLUGIN_DIR);
    if (!directory) {
        printf("Cannot open plugin directory '%s'.\n", PLUGIN_DIR);
        return;
    }
    struct dirent* entry;
    while ((entry = readdir(directory)) != NULL) {
        const char* filename = entry->d_name;
        size_t len = strlen(filename);
        if (len < 3 || strcmp(filename + len - 3, ".so") != 0) continue;
        if (commandCount >= MAX_PLUGINS) {
            printf("Plugin limit reached.\n");
            break;
        }
        char path[PATH_MAX];
        int length = snprintf(path, sizeof(path), "./%s/%s", PLUGIN_DIR, filename);
        if (length < 0 || (size_t)length >= sizeof(path)) continue;

        void* library = dlopen(path, RTLD_NOW);
        if (!library) {
            printf("Cannot load %s: %s\n", filename, dlerror());
            continue;
        }
        dlerror(); // Clear any previous dynamic-loader error.
        void* symbol = dlsym(library, "getPluginInfo");
        GetPluginInfo getInfo = NULL;
        if (symbol) memcpy(&getInfo, &symbol, sizeof(getInfo));
        const char* error = dlerror();
        if (error || !getInfo) {
            printf("Skipped %s: missing getPluginInfo.\n", filename);
            dlclose(library);
            continue;
        }
        const PluginInfo* info = getInfo();
        if (!info || !info->name || !info->function ||
            (info->arguments != 1 && info->arguments != 2) ||
            strlen(info->name) >= sizeof(commands[0].name)) {
            printf("Skipped %s: invalid plugin.\n", filename);
            dlclose(library);
            continue;
        }
        Command* command = &commands[commandCount++];
        snprintf(command->filename, sizeof(command->filename), "%s", filename);
        snprintf(command->name, sizeof(command->name), "%s", info->name);
        command->arguments = info->arguments;
        command->enabled = 1;
        command->function = info->function;
        command->library = library;
        printf("Loaded: %s (%s)\n", command->name, command->filename);
    }
    closedir(directory);
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
    for (int i = 0; i < commandCount; i++) dlclose(commands[i].library);
    commandCount = 0;
}

int main(void) {
    printf("Searching for .so plugins...\n");
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
