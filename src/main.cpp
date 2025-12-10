#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include "analyzer.h"
#include "types.h"
#include "report.hpp"
#include "json_output.hpp"

using sentinel::Analyzer;
using sentinel::AnalyzerConfig;
using sentinel::Finding;
using sentinel::Findings;
using sentinel::FindingKind;
using sentinel::Severity;

namespace {

void print_usage(const char* prog) {
    std::cout
        << "Usage:\n"
        << "  " << prog << " [options]\n\n"
        << "Options:\n"
        << "  --source <PATH>    Add source file or directory for analysis\n"
        << "  --binary <PATH>    Add binary file for analysis\n"
        << "  --no-source        Disable source analysis\n"
        << "  --no-binary        Disable binary analysis\n"
        << "  --verbose          Show INFO-level messages\n"
        << "  --json             Output findings as JSON report\n"
        << "  -h, --help         Show this help\n\n"
        << "Example:\n"
        << "  " << prog << " --source ./test --binary ./a.out\n";
}

sx::AnalysisReport build_report(const Findings& findings) {
    sx::AnalysisReport report;

    std::unordered_set<std::string> files;

    for (const auto& f : findings) {
        sx::Finding rf;

        if (!f.source_location.file.empty()) {
            rf.file = f.source_location.file;
            rf.line = static_cast<int>(f.source_location.line);
            files.insert(f.source_location.file);
        } else if (!f.binary_location.segment_or_section.empty()) {
            rf.file = f.binary_location.segment_or_section;
            rf.line = 0;
        } else {
            rf.file = "";
            rf.line = 0;
        }

        rf.function = "";   // можно расширить позже до реального имени функции
        rf.buffer   = "";   // сюда можно выводить имя буфера при углублённом анализе

        rf.kind     = f.id;
        rf.severity = sentinel::severity_to_string(f.severity);
        rf.message  = f.message;

        report.findings.push_back(std::move(rf));
    }

    report.files_analyzed = files.size();
    return report;
}

} // namespace

int main(int argc, char** argv) {
    AnalyzerConfig config;
    std::vector<std::string> source_paths;
    std::vector<std::string> binary_paths;
    bool json_output = false;

    if (argc == 1) {
        print_usage(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--source") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --source\n";
                return 1;
            }
            source_paths.emplace_back(argv[++i]);
        } else if (arg == "--binary") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --binary\n";
                return 1;
            }
            binary_paths.emplace_back(argv[++i]);
        } else if (arg == "--no-source") {
            config.analyze_source = false;
        } else if (arg == "--no-binary") {
            config.analyze_binary = false;
        } else if (arg == "--verbose") {
            config.verbose = true;
        } else if (arg == "--json") {
            json_output = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!config.analyze_source && !config.analyze_binary) {
        std::cerr << "Both source and binary analysis are disabled.\n";
        return 1;
    }

    if (source_paths.empty() && binary_paths.empty()) {
        std::cerr << "No input paths specified (neither --source nor --binary).\n";
        print_usage(argv[0]);
        return 1;
    }

    Analyzer analyzer(config);
    Findings findings = analyzer.analyze(source_paths, binary_paths);

    if (json_output) {
        sx::AnalysisReport report = build_report(findings);
        sx::print_json(report, std::cout);
        return 0;
    }

    if (findings.empty()) {
        std::cout << "No potential buffer overflows detected.\n";
        return 0;
    }

    std::size_t printed = 0;
    for (const Finding& f : findings) {
        if (!config.verbose && f.severity == Severity::Info) {
            continue; // INFO показываем только в verbose
        }

        ++printed;
        std::cout << "[" << sentinel::kind_to_string(f.kind) << "]"
                  << "[" << sentinel::severity_to_string(f.severity) << "] "
                  << f.id << "\n";

        if (!f.source_location.file.empty()) {
            std::cout << "  at " << f.source_location.file
                      << ":" << f.source_location.line << "\n";
        } else if (!f.binary_location.arch.empty()) {
            std::cout << "  arch: " << f.binary_location.arch;
            if (!f.binary_location.segment_or_section.empty()) {
                std::cout << ", section: " << f.binary_location.segment_or_section;
            }
            if (f.binary_location.offset != 0) {
                std::cout << ", offset: 0x"
                          << std::hex << f.binary_location.offset << std::dec;
            }
            std::cout << "\n";

            if (!f.binary_location.disasm.empty()) {
                std::cout << "  Disassembly:\n";
                std::cout << f.binary_location.disasm << "\n";
            }
        }

        if (!f.message.empty()) {
            std::cout << "  " << f.message << "\n";
        }
        if (!f.recommendation.empty()) {
            std::cout << "  Recommendation: " << f.recommendation << "\n";
        }
        std::cout << "\n";
    }

    if (printed == 0) {
        std::cout << "No findings (after filtering INFO-level messages).\n";
    }

    return 0;
}
