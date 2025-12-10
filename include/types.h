#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sentinel {

enum class Severity {
    Info,
    Warning,
    High,
    Critical
};

enum class FindingKind {
    Source,
    Binary
};

struct SourceLocation {
    std::string file;
    std::size_t line = 0;
};

struct BinaryLocation {
    std::string  segment_or_section;  // .text, .plt, сегмент и т.п.
    std::uint64_t offset = 0;         // виртуальный адрес или смещение
    std::string  arch;                // x86, x86_64, arm, arm64, ...
    std::string  disasm;              // сниппет ассемблера вокруг проблемы
};

struct Finding {
    FindingKind     kind        = FindingKind::Source;
    Severity        severity    = Severity::Info;
    std::string     id;
    std::string     message;
    std::string     recommendation;
    SourceLocation  source_location;
    BinaryLocation  binary_location;
};

struct AnalyzerConfig {
    bool analyze_source = true;
    bool analyze_binary = true;
    bool verbose        = false;
};

using Findings = std::vector<Finding>;

inline std::string severity_to_string(Severity s) {
    switch (s) {
        case Severity::Info:     return "INFO";
        case Severity::Warning:  return "WARNING";
        case Severity::High:     return "HIGH";
        case Severity::Critical: return "CRITICAL";
    }
    return "UNKNOWN";
}

inline std::string kind_to_string(FindingKind k) {
    switch (k) {
        case FindingKind::Source: return "SOURCE";
        case FindingKind::Binary: return "BINARY";
    }
    return "UNKNOWN";
}

} // namespace sentinel
