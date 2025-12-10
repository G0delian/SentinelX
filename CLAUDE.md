# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

SentinelX is a static analysis tool for detecting buffer overflow vulnerabilities in C/C++ code. It performs both source code analysis and binary analysis (via LIEF library) to identify potentially unsafe memory operations.

## Build and Development Commands

### Building the Project

```bash
# Basic build (with LIEF support enabled by default)
./build.sh

# Build without LIEF (fallback binary analysis mode)
SENTINELX_USE_LIEF=OFF ./build.sh

# Manual CMake build
cmake -S . -B build -DSENTINELX_USE_LIEF=ON
cmake --build build --config Release

# Docker build
docker build -t sentinelx .
```

### Running the Tool

```bash
# Analyze source files
./build/SentinelX --source ./test

# Analyze binary files
./build/SentinelX --binary ./a.out

# Combined analysis with verbose output
./build/SentinelX --source ./src --binary ./build/SentinelX --verbose

# JSON output for programmatic consumption
./build/SentinelX --source ./test --json
```

## Architecture

### Analysis Pipeline

The tool follows a modular architecture with distinct phases:

1. **Input Parsing** (`src/main.cpp`): CLI argument handling and output formatting
2. **Analysis Orchestration** (`src/analyzer.cpp`): Coordinates source and binary detectors
3. **Detection Engines**:
   - **SourceDetector** (`src/detectors.cpp`): Scans C/C++ source for dangerous patterns
   - **BinaryDetector** (`src/detectors.cpp`): Analyzes compiled binaries for unsafe imports

### Source Analysis Strategy

The SourceDetector uses multiple detection techniques:

- **Pattern Matching**: Regex-based detection of dangerous function calls (strcpy, gets, scanf, etc.)
- **Stateful Buffer Tracking**: `BufferFSM` finite state machine tracks buffer declarations and writes
- **Comment Stripping**: Parses code while preserving string literals and ignoring comments
- **Annotation Support**: Respects `SENTINELX_IGNORE` markers to suppress false positives

Key components:
- `strip_comments_preserve_strings()`: Context-aware code cleaning that maintains string boundaries
- `contains_function_call()`: Tokenization-aware function call detection
- `BufferFSM` (include/fsm.hpp): State machine tracking buffer lifecycle (Unknown → Allocated → Tainted)

### Binary Analysis Strategy

Binary analysis has two modes controlled by the `SENTINELX_USE_LIEF` CMake option:

**With LIEF (default)** - Three-Phase Analysis:
- **Phase 1: Call-Site Detection** (`src/call_site_analyzer.cpp`): Finds actual invocations of dangerous functions in .text section
  - Scans disassembled instructions for `call` mnemonics targeting unsafe functions
  - Provides 4-10 instruction context window around each finding
  - Caches binary parsing and section disassembly for efficiency (~8x speedup vs legacy approach)

- **Phase 2: Stack Frame Analysis** (`src/stack_analyzer.cpp`): Detects large stack allocations and dangerous string operations
  - Identifies `sub rsp/esp, N` patterns where N >= 1024 bytes (configurable threshold)
  - Detects unbounded `rep movs*` and `rep stos*` instructions
  - Flags potential stack overflow vulnerabilities

- **Phase 3: Integer Overflow & Format String** (`src/arithmetic_analyzer.cpp`): Advanced vulnerability detection
  - **Integer Overflow**: Clusters arithmetic operations (add/sub/mul/shl) in 256-byte windows
    - Flags operations without subsequent overflow checks (jo, jc, jno, jnc instructions)
    - Reduces false positives by requiring 2+ operations per cluster
  - **Format String**: Analyzes printf/sprintf/fprintf calls
    - Tracks format string argument loading (RDI/RCX registers on x86-64)
    - Detects non-constant format strings (potential user-controlled input)
    - Identifies RIP-relative loads from .rodata as safe constants

- **Caching Architecture** (`src/disassembler.cpp`):
  - Binary parsed once via `load_binary()` and cached in `std::unique_ptr<LIEF::Binary>`
  - Section disassembly cached in `std::unordered_map<std::string, std::vector<Instruction>>`
  - `Instruction` struct extended with `mnemonic` and `operands` fields for efficient pattern matching
  - `get_context()` method extracts instruction windows without re-parsing

**Without LIEF (fallback)**:
- Performs naive byte-level search for function name strings
- No architecture detection or disassembly
- Intended as lightweight fallback mode

### Type System

Core data structures (include/types.h):

- **Finding**: Represents a single detection with severity, location, and recommendations
  - `FindingKind`: SOURCE or BINARY
  - `Severity`: Info, Warning, High, Critical
  - Supports dual location tracking (source file:line or binary section:offset)

- **AnalyzerConfig**: Analysis flags (analyze_source, analyze_binary, verbose)

### Output Formats

- **Human-readable**: Categorized findings with severity, location, and actionable recommendations
- **JSON** (via `json_output.cpp`): Structured report with files_analyzed count and finding arrays

## Key Implementation Details

### Binary Vulnerability Detection Categories

**Buffer Overflow**:
- Call-site detection for strcpy, strcat, sprintf, gets, scanf family
- Detects actual function invocations (not just imports) with assembly context
- Finding ID format: `BIN_CALLSITE_<FUNCTION>` (e.g., `BIN_CALLSITE_STRCPY`)

**Stack Overflow**:
- Large stack allocations >= 1024 bytes via `sub rsp, N` patterns
- Finding ID: `BIN_LARGE_STACK_FRAME`
- Dangerous string operations: `rep movs*`, `rep stos*`
- Finding ID: `BIN_DANGEROUS_STRING_OP`

**Integer Overflow**:
- Arithmetic without overflow checks (add, sub, mul, imul, shl, sal)
- Clustering algorithm reduces false positives (2+ ops in 256-byte window)
- Finding ID: `BIN_INTEGER_OVERFLOW_RISK`

**Format String Vulnerabilities**:
- printf/sprintf/fprintf with non-constant format strings
- Analyzes register loading patterns (System V: RDI, Windows: RCX)
- Finding ID: `BIN_FORMAT_STRING_VULN`

### Source Code Detection

**Unsafe Function Detection**:
- Unbounded copy: strcpy, strcat, sprintf, gets
- Format string issues: scanf family with unbounded %s
- Buffer writes: memcpy, read() with size exceeding declaration

**Buffer Overflow Detection via FSM**:
1. `on_declare()`: Registers buffer with size (state → Allocated)
2. `on_write()`: Checks if write size exceeds buffer size (state → Tainted if overflow)
3. Regex patterns extract buffer names and sizes from declarations like `char buf[16]`

### Suppression Mechanism

Lines containing `SENTINELX_IGNORE` or `SENTINELX:IGNORE` are skipped during analysis.

### Testing

Test files demonstrate each detection category:
- `test/test_callsites.cpp`: Phase 1 - Dangerous function calls
- `test/test_stack.cpp`: Phase 2 - Large stack buffers and string operations
- `test/test_arithmetic.cpp`: Phase 3 - Integer overflow and format string vulnerabilities

## LIEF Integration

The project includes LIEF as a git submodule. When LIEF support is enabled:
- `SENTINELX_USE_LIEF` preprocessor macro is defined
- Binary parser uses LIEF API for cross-platform binary format parsing
- Disassembler accesses LIEF's instruction decoding capabilities

The CMake configuration explicitly disables LIEF's examples, tests, logging, and Python API to minimize build overhead.
