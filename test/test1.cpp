#include <cstdio>
#include <cstring>

// Пример уязвимого кода для теста SentinelX
void vuln_strcpy(const char* input) {
    char buf[16];
    std::strcpy(buf, input); // здесь должен сработать детектор
    std::printf("%s\n", buf);
}

void vuln_scanf() {
    char buf[32];
    // Без ограничения ширины у %s — потенциальное переполнение
    std::scanf("%s", buf);
}

void safe_snprintf(const char* input) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%s", input); // не должно срабатывать
}

// Пример подавления ложных срабатываний:
// SENTINELX_IGNORE
void weird_but_intended(const char* src) {
    char buf[64];
    // SENTINELX_IGNORE
    std::strcpy(buf, src); // будет проигнорировано из-за комментария выше
}

int main(int argc, char** argv) {
    if (argc > 1) {
        vuln_strcpy(argv[1]);
    } else {
        vuln_scanf();
    }
    return 0;
}
