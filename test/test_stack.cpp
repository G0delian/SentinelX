#include <cstring>
#include <cstdio>

// Test case 1: Large stack buffer (should trigger BIN_LARGE_STACK_FRAME)
void large_buffer_function() {
    char huge_buffer[2048];  // 2KB buffer - should be detected
    huge_buffer[0] = 'A';
    printf("Large buffer allocated\n");
}

// Test case 2: Very large stack buffer
void very_large_buffer() {
    char massive_buffer[4096];  // 4KB buffer
    massive_buffer[0] = 'B';
}

// Test case 3: Multiple large buffers
void multiple_large_buffers() {
    char buf1[1536];  // 1.5KB
    char buf2[1536];  // 1.5KB
    buf1[0] = 'C';
    buf2[0] = 'D';
}

// Test case 4: String operations that might use rep movs
void string_copy_operations() {
    char src[1024];
    char dst[1024];

    memset(src, 'S', sizeof(src));

    // This might compile to rep movs instruction
    memcpy(dst, src, sizeof(src));

    printf("Copied: %c\n", dst[0]);
}

// Test case 5: Normal size buffer (should NOT trigger)
void normal_buffer() {
    char small_buffer[512];  // 512 bytes - below threshold
    small_buffer[0] = 'E';
}

int main() {
    large_buffer_function();
    very_large_buffer();
    multiple_large_buffers();
    string_copy_operations();
    normal_buffer();

    return 0;
}
