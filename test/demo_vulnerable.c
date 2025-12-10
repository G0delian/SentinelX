#include <stdio.h>
#include <string.h>

// Vulnerable function with strcpy
void vulnerable_copy(char* user_input) {
    char buffer[16];
    strcpy(buffer, user_input);  // Buffer overflow vulnerability!
    printf("Buffer: %s\n", buffer);
}

// Vulnerable function with gets
void vulnerable_gets() {
    char buffer[32];
    printf("Enter your name: ");
    gets(buffer);  // Very dangerous! No bounds checking
    printf("Hello, %s!\n", buffer);
}

// Vulnerable function with scanf
void vulnerable_scanf() {
    char name[20];
    printf("Name: ");
    scanf("%s", name);  // No width specifier - buffer overflow risk
    printf("Hi %s\n", name);
}

int main(int argc, char** argv) {
    if (argc > 1) {
        vulnerable_copy(argv[1]);
    }

    vulnerable_gets();
    vulnerable_scanf();

    return 0;
}
