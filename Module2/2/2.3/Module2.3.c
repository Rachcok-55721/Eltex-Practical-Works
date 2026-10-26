
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_COMMANDS 6

// Function pointer type
typedef double (*Operation)(double, double);

// Command structure
typedef struct {
    const char* name;
    Operation function;
    int arguments;
    int enabled;
} Command;

// Mathematical operations
double add(double a, double b) {
    return a + b;
}

double subtract(double a, double b) {
    return a - b;
}

double multiply(double a, double b) {
    return a * b;
}

double divide(double a, double b) {
    return a / b;
}

double power(double a, double b) {
    return pow(a, b);
}

double squareRoot(double a, double b) {
    (void)b;
    return sqrt(a);
}

// Read an integer
int inputInt() {
    int value;
    char buffer[100];
    char extra;

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            exit(0);
        }

        if (sscanf(buffer, " %d %c", &value, &extra) == 1) {
            return value;
        }

        printf("Invalid input. Enter an integer: ");
    }
}

// Read a number
double inputDouble() {
    double value;
    char buffer[100];
    char extra;

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            exit(0);
        }

        if (sscanf(buffer, " %lf %c", &value, &extra) == 1 &&
            isfinite(value)) {
            return value;
        }

        printf("Invalid input. Enter a number: ");
    }
}

// Display available commands
int showMenu(Command commands[], int count,
    int available[]) {
    int total = 0;

    printf("\n========== CALCULATOR ==========\n");

    for (int i = 0; i < count; i++) {
        if (commands[i].enabled) {
            available[total] = i;
            total++;

            printf("%d. %s\n",
                i + 1, commands[i].name);
        }
    }

    printf("7. Manage commands\n");
    printf("0. Exit\n");
    printf("================================\n");

    return total;
}


// Enable or disable commands
void manageCommands(Command commands[], int count) {
    int choice;

    do {
        printf("\n===== MANAGE COMMANDS =====\n");

        for (int i = 0; i < count; i++) {
            printf("%d. %s [%s]\n",
                i + 1,
                commands[i].name,
                commands[i].enabled ? "ON" : "OFF");
        }

        printf("0. Back to calculator\n");
        printf("===========================\n");
        printf("Select command to toggle: ");

        choice = inputInt();

        if (choice >= 1 && choice <= count) {
            int index = choice - 1;

            commands[index].enabled =
                !commands[index].enabled;

            printf("\n%s is now %s.\n",
                commands[index].name,
                commands[index].enabled ? "ON" : "OFF");
        }
        else if (choice != 0) {
            printf("\nInvalid choice!\n");
        }

    } while (choice != 0);
}

// Execute selected command
void executeCommand(Command* command) {
    double a, b = 0.0, result;

    printf("\n===== %s =====\n", command->name);

    printf("Enter first number: ");
    a = inputDouble();

    if (command->arguments == 2) {
        printf("Enter second number: ");
        b = inputDouble();
    }

    // Check division by zero
    if (command->function == divide && b == 0.0) {
        printf("\nError: Division by zero!\n");
        return;
    }

    // Check negative square root
    if (command->function == squareRoot && a < 0.0) {
        printf("\nError: Negative square root!\n");
        return;
    }

    // Call function using function pointer
    result = command->function(a, b);

    if (!isfinite(result)) {
        printf("\nError: Invalid mathematical result!\n");
        return;
    }

    printf("\nResult: %.2f\n", result);
}

// Main program
int main() {
    // Dynamic array of commands
    Command* commands = malloc(
        MAX_COMMANDS * sizeof(Command)
    );

    if (commands == NULL) {
        printf("Memory allocation error!\n");
        return 1;
    }

    // Initialize commands
    commands[0] = (Command){
        "Addition", add, 2, 1
    };

    commands[1] = (Command){
        "Subtraction", subtract, 2, 1
    };

    commands[2] = (Command){
        "Multiplication", multiply, 2, 1
    };

    commands[3] = (Command){
        "Division", divide, 2, 1
    };

    commands[4] = (Command){
        "Power", power, 2, 1
    };

    commands[5] = (Command){
        "Square root", squareRoot, 1, 1
    };

    int available[MAX_COMMANDS];
    int choice;

    while (1) {
        showMenu(commands, MAX_COMMANDS, available);

        printf("Enter your choice: ");
        choice = inputInt();

        if (choice == 0) {
            printf("\nExiting calculator. Goodbye!\n");
            break;
        }

        if (choice == 7) {
            manageCommands(commands, MAX_COMMANDS);
            continue;
        }

        if (choice >= 1 && choice <= MAX_COMMANDS &&
            commands[choice - 1].enabled) {

            executeCommand(&commands[choice - 1]);

        }
        else {
            printf("\nInvalid choice. Try again.\n");
        }
    }

    // Release allocated memory
    free(commands);

    return 0;
}
