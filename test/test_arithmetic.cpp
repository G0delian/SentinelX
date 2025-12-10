#include <cstdio>
#include <cstdint>

// Test case 1: Integer overflow risk (addition without check)
int overflow_addition(int a, int b) {
    return a + b;  // No overflow check - should be detected
}

// Test case 2: Integer overflow risk (multiplication)
int overflow_multiplication(int x, int y) {
    int result = x * y;  // No overflow check
    return result;
}

// Test case 3: Safe arithmetic with overflow check
int safe_addition(int a, int b) {
    int result = a + b;
    // Check for overflow (this would be jo/jno in assembly)
    if ((b > 0 && a > INT32_MAX - b) || (b < 0 && a < INT32_MIN - b)) {
        return 0;  // Overflow detected
    }
    return result;
}

// Test case 4: Format string vulnerability
void format_string_vuln(char* user_input) {
    printf(user_input);  // DANGEROUS - user input as format string
}

// Test case 5: Safe format string usage
void safe_format_string(char* user_input) {
    printf("%s", user_input);  // SAFE - constant format string
}

// Test case 6: Format string with fprintf
void fprintf_vuln(char* input) {
    fprintf(stderr, input);  // DANGEROUS
}

// Test case 7: Multiple arithmetic operations
uint64_t complex_calculation(uint64_t a, uint64_t b, uint64_t c) {
    uint64_t sum = a + b;       // No overflow check
    uint64_t product = sum * c; // No overflow check
    return product >> 2;        // Shift operation
}

// Test case 8: Shift operation (potential overflow)
int shift_overflow(int value, int shift_amount) {
    return value << shift_amount;  // Can overflow
}

int main(int argc, char** argv) {
    // Test overflow scenarios
    int x = overflow_addition(2000000000, 2000000000);
    int y = overflow_multiplication(100000, 100000);

    // Test safe version
    int z = safe_addition(100, 200);

    // Test format string vulnerabilities
    if (argc > 1) {
        format_string_vuln(argv[1]);  // VULN
        fprintf_vuln(argv[1]);         // VULN
        safe_format_string(argv[1]);   // SAFE
    }

    // Test complex arithmetic
    uint64_t result = complex_calculation(1000, 2000, 3000);
    int shifted = shift_overflow(42, 10);

    printf("Results: %d %d %d %llu %d\n", x, y, z, result, shifted);

    return 0;
}
