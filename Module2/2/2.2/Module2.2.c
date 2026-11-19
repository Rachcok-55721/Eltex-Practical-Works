
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Addition
double add(double a, double b) {
    return a + b;
}

// Subtraction
double subtract(double a, double b) {
    return a - b;
}

// Multiplication
double multiply(double a, double b) {
    return a * b;
}

// Division
double divide(double a, double b) {
    return a / b;
}

// Power
double power(double a, double b) {
    return pow(a, b);
}

// Square root
double squareRoot(double a) {
    return sqrt(a);
}

// Main program
int main() {
    int choice;
    double a, b, result;
    char buffer[100];
    char extra;

    do {
        printf("\n========== CALCULATOR ==========\n");
        printf("1. Addition\n");
        printf("2. Subtraction\n");
        printf("3. Multiplication\n");
        printf("4. Division\n");
        printf("5. Power\n");
        printf("6. Square root\n");
        printf("0. Exit\n");
        printf("================================\n");
        printf("Enter your choice: ");

        do {
            if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
            if (sscanf(buffer, " %d %c", &choice, &extra) == 1) break;
            printf("Invalid input. Enter an integer: ");
        } while (1);

        if (choice == 0) {
            printf("\nExiting calculator. Goodbye!\n");
            break;
        }

        if (choice < 1 || choice > 6) {
            printf("\nInvalid choice. Try again.\n");
            continue;
        }

        printf("\nEnter first number: ");
        do {
            if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
            if (sscanf(buffer, " %lf %c", &a, &extra) == 1) break;
            printf("Invalid input. Enter a number: ");
        } while (1);

        if (choice != 6) {
            printf("Enter second number: ");
            do {
                if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
                if (sscanf(buffer, " %lf %c", &b, &extra) == 1) break;
                printf("Invalid input. Enter a number: ");
            } while (1);
        }

        switch (choice) {
        case 1:
            result = add(a, b);
            printf("\nResult: %.2f\n", result);
            break;

        case 2:
            result = subtract(a, b);
            printf("\nResult: %.2f\n", result);
            break;

        case 3:
            result = multiply(a, b);
            printf("\nResult: %.2f\n", result);
            break;

        case 4:
            if (b == 0) {
                printf("\nError: Division by zero!\n");
            }
            else {
                result = divide(a, b);
                printf("\nResult: %.2f\n", result);
            }
            break;

        case 5:
            result = power(a, b);
            printf("\nResult: %.2f\n", result);
            break;

        case 6:
            if (a < 0) {
                printf("\nError: Cannot calculate "
                    "square root of a negative number!\n");
            }
            else {
                result = squareRoot(a);
                printf("\nResult: %.2f\n", result);
            }
            break;
        }

    } while (1);

    return 0;
}
