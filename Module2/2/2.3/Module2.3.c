
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

    int choice;
    char buffer[100];
    char extra;

    while (1) {
        printf("\n========== CALCULATOR ==========\n");
        for (int i = 0; i < MAX_COMMANDS; i++) {
            if (commands[i].enabled) {
                printf("%d. %s\n", i + 1, commands[i].name);
            }
        }
        printf("7. Manage commands\n");
        printf("0. Exit\n");
        printf("================================\n");

        printf("Enter your choice: ");
        do {
            if (!fgets(buffer, sizeof(buffer), stdin)) goto cleanup;
            if (sscanf(buffer, " %d %c", &choice, &extra) == 1) break;
            printf("Invalid input. Enter an integer: ");
        } while (1);

        if (choice == 0) {
            printf("\nExiting calculator. Goodbye!\n");
            break;
        }

        if (choice == 7) {
            int manageChoice;
            do {
                printf("\n===== MANAGE COMMANDS =====\n");
                for (int i = 0; i < MAX_COMMANDS; i++) {
                    printf("%d. %s [%s]\n", i + 1, commands[i].name,
                           commands[i].enabled ? "ON" : "OFF");
                }
                printf("0. Back to calculator\n");
                printf("===========================\n");
                printf("Select command to toggle: ");
                do {
                    if (!fgets(buffer, sizeof(buffer), stdin)) goto cleanup;
                    if (sscanf(buffer, " %d %c", &manageChoice, &extra) == 1) break;
                    printf("Invalid input. Enter an integer: ");
                } while (1);
                if (manageChoice >= 1 && manageChoice <= MAX_COMMANDS) {
                    int index = manageChoice - 1;
                    commands[index].enabled = !commands[index].enabled;
                    printf("\n%s is now %s.\n", commands[index].name,
                           commands[index].enabled ? "ON" : "OFF");
                } else if (manageChoice != 0) {
                    printf("\nInvalid choice!\n");
                }
            } while (manageChoice != 0);
            continue;
        }

        if (choice >= 1 && choice <= MAX_COMMANDS &&
            commands[choice - 1].enabled) {

            Command* command = &commands[choice - 1];
            double a, b = 0.0, result;
            printf("\n===== %s =====\n", command->name);
            printf("Enter first number: ");
            do {
                if (!fgets(buffer, sizeof(buffer), stdin)) goto cleanup;
                if (sscanf(buffer, " %lf %c", &a, &extra) == 1 && isfinite(a)) break;
                printf("Invalid input. Enter a number: ");
            } while (1);
            if (command->arguments == 2) {
                printf("Enter second number: ");
                do {
                    if (!fgets(buffer, sizeof(buffer), stdin)) goto cleanup;
                    if (sscanf(buffer, " %lf %c", &b, &extra) == 1 && isfinite(b)) break;
                    printf("Invalid input. Enter a number: ");
                } while (1);
            }
            // Check division by zero
            if (command->function == divide && b == 0.0) {
                printf("\nError: Division by zero!\n");
                continue;
            }
            // Check negative square root
            if (command->function == squareRoot && a < 0.0) {
                printf("\nError: Negative square root!\n");
                continue;
            }
            // Call function using function pointer
            result = command->function(a, b);
            if (!isfinite(result)) {
                printf("\nError: Invalid mathematical result!\n");
                continue;
            }
            printf("\nResult: %.2f\n", result);

        }
        else {
            printf("\nInvalid choice. Try again.\n");
        }
    }

cleanup:
    // Release allocated memory
    free(commands);

    return 0;
}
