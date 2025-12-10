#include <cstring>
#include <cstdio>

// Test case 1: Unsafe strcpy call
void unsafe_copy() {
    char buf[16];
    char* input = "This is a very long string that will overflow";
    strcpy(buf, input);  // SHOULD detect call-site
    printf("Copied: %s\n", buf);
}

// Test case 2: Unsafe scanf call
void unsafe_read() {
    char buf[32];
    printf("Enter input: ");
    scanf("%s", buf);  // SHOULD detect call-site
    printf("Read: %s\n", buf);
}

// Test case 3: Unsafe sprintf call
void unsafe_format() {
    char buf[64];
    char* user_data = "Some user controlled data that could be long";
    sprintf(buf, "%s", user_data);  // SHOULD detect call-site
    printf("Formatted: %s\n", buf);
}

// Test case 4: Multiple unsafe calls in one function
void multiple_unsafe() {
    char buf1[16];
    char buf2[16];

    strcpy(buf1, "test1");  // SHOULD detect
    strcat(buf2, "test2");  // SHOULD detect
}

int main(int argc, char** argv) {
    if (argc > 1) {
        unsafe_copy();
    } else {
        unsafe_read();
    }

    unsafe_format();
    multiple_unsafe();

    return 0;
}
