# Использование бинарного анализа в SentinelX

## Требования

Для полноценного бинарного анализа с выводом ассемблерного кода необходим **LIEF** (Library to Instrument Executable Formats).

### Проверка наличия LIEF

```bash
# Проверить, включен ли LIEF
grep SENTINELX_USE_LIEF build/CMakeCache.txt
```

Если вывод: `SENTINELX_USE_LIEF:BOOL=OFF`, то нужно пересобрать с LIEF.

### Сборка с LIEF

```bash
# Добавить LIEF как submodule (если еще не добавлен)
git submodule add https://github.com/lief-project/LIEF.git LIEF
git submodule update --init --recursive

# Пересобрать проект с LIEF
rm -rf build
mkdir build
cd build
cmake -DSENTINELX_USE_LIEF=ON ..
cmake --build . --config Release

# Или используя build.sh
./build.sh
```

**Примечание**: Первая сборка LIEF занимает 10-15 минут.

## Использование

### Базовый анализ бинарного файла

```bash
./build/SentinelX --binary ./path/to/binary
```

### Пример вывода с LIEF

```
[BINARY][HIGH] BIN_CALLSITE_STRCPY
  arch: x86_64, section: .text, offset: 0x100003f4a
  Call to dangerous function 'strcpy' at 0x100003f4a
  Recommendation: Review call site and ensure bounds checking. Consider using safer alternatives (e.g., strncpy, snprintf).
  Disassembly:
    0x100003f40: push   rbp
    0x100003f41: mov    rbp, rsp
    0x100003f44: sub    rsp, 0x20
    0x100003f48: lea    rdi, [rbp-0x10]
>>> 0x100003f4a: call   strcpy@PLT
    0x100003f4f: lea    rdi, [rip + 0xf50]
    0x100003f56: lea    rsi, [rbp-0x10]
    ...
```

### Категории обнаруживаемых уязвимостей

#### 1. Buffer Overflow (Phase 1)
- **Функции**: strcpy, strcat, sprintf, gets, scanf, fscanf, sscanf
- **Finding ID**: `BIN_CALLSITE_<FUNCTION>`
- **Severity**: Critical (gets), High (strcpy, sprintf), Warning (остальные)

#### 2. Stack Overflow (Phase 2)
- **Паттерн**: `sub rsp, N` где N >= 1024
- **Finding ID**: `BIN_LARGE_STACK_FRAME`
- **Severity**: Warning

- **Паттерн**: `rep movs*`, `rep stos*`
- **Finding ID**: `BIN_DANGEROUS_STRING_OP`
- **Severity**: High

#### 3. Integer Overflow (Phase 3)
- **Операции**: add, sub, mul, imul, shl, sal без overflow checks
- **Finding ID**: `BIN_INTEGER_OVERFLOW_RISK`
- **Severity**: Warning

#### 4. Format String (Phase 3)
- **Функции**: printf, sprintf, fprintf с непостоянными format string
- **Finding ID**: `BIN_FORMAT_STRING_VULN`
- **Severity**: High

### Комбинированный анализ (source + binary)

```bash
./build/SentinelX --source ./src --binary ./build/myapp
```

### JSON вывод

```bash
./build/SentinelX --binary ./myapp --json > report.json
```

## Тестирование

### Тестовые файлы

Проект включает тестовые файлы для каждой фазы:

```bash
# Phase 1: Call-site detection
./build/SentinelX --binary ./test/test_callsites

# Phase 2: Stack analysis
./build/SentinelX --binary ./test/test_stack

# Phase 3: Arithmetic analysis
./build/SentinelX --binary ./test/test_arithmetic
```

### Создание собственного теста

```bash
# Создать уязвимый C файл
cat > vuln.c << 'EOF'
#include <string.h>
#include <stdio.h>

void unsafe(char* input) {
    char buf[16];
    strcpy(buf, input);  // Overflow!
}

int main(int argc, char** argv) {
    if (argc > 1) unsafe(argv[1]);
    return 0;
}
EOF

# Скомпилировать
gcc -o vuln vuln.c -no-pie

# Анализировать
./build/SentinelX --binary ./vuln
```

## Режим без LIEF (fallback)

Если LIEF недоступен, SentinelX использует упрощенный режим:

```
[BINARY][WARNING] BIN_UNSAFE_BYTES_strcpy
  arch: unknown, section: raw, offset: 0x80b7
  Binary contains reference to potentially unsafe function 'strcpy' at file offset 0x80b7.
  Recommendation: This is a heuristic scan without LIEF. Consider enabling LIEF for accurate disassembly and call-site analysis.
```

**Ограничения fallback режима**:
- Нет дизассемблирования
- Нет определения архитектуры
- Только поиск строк в бинарнике
- Нет call-site detection
- Нет stack/arithmetic анализа

## Производительность

### С кэшированием (новая версия)
- Парсинг бинарника: 1 раз
- Дизассемблирование секций: кэшируется
- **Ускорение**: ~8x по сравнению со старой версией

### Рекомендации
- Для больших бинарников (>100MB) анализ может занять несколько секунд
- JSON вывод быстрее для автоматизации
- Используйте `--no-source` если нужен только бинарный анализ

## Troubleshooting

### "This is a heuristic scan without LIEF"
**Причина**: Проект собран без LIEF
**Решение**: Пересобрать с `-DSENTINELX_USE_LIEF=ON`

### Пустой вывод "No potential buffer overflows detected"
**Возможные причины**:
1. Бинарник действительно безопасен
2. Функции инлайн-оптимизированы (компилятор встроил их)
3. Strip удалил символы

**Решение**: Компилировать с `-O0 -g -no-pie` для более четкого анализа

### Slow first build
**Причина**: LIEF - большая библиотека
**Решение**: Это нормально, последующие сборки будут быстрее

## Дополнительные возможности

- Поддержка нескольких архитектур: x86, x86-64, ARM, ARM64, PPC
- Форматы: ELF, PE, Mach-O
- Verbose режим: `--verbose` для INFO-level сообщений
