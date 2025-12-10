#include "detectors.h"

#include "binary_parser.h"
#include "disassembler.h"
#include "fsm.hpp"
#include "utils.h"

#include <filesystem>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <sstream>

namespace fs = std::filesystem;

namespace sentinel {

namespace {

bool is_source_file(const fs::path& p) {
    const std::string ext = to_lower(p.extension().string());
    return ext == ".c" || ext == ".cc" || ext == ".cpp" || ext == ".cxx" ||
           ext == ".h" || ext == ".hpp" || ext == ".hh";
}

std::string strip_comments_preserve_strings(const std::string& line,
                                            bool& in_block_comment) {
    enum class State {
        Normal,
        InString,
        InChar,
        InLineComment,
        InBlockComment
    };

    State state = in_block_comment ? State::InBlockComment : State::Normal;
    std::string out;

    for (std::size_t i = 0; i < line.size(); ++i) {
        char c    = line[i];
        char next = (i + 1 < line.size()) ? line[i + 1] : '\0';

        switch (state) {
            case State::Normal:
                if (c == '/' && next == '/') {
                    state = State::InLineComment;
                    ++i;
                } else if (c == '/' && next == '*') {
                    state = State::InBlockComment;
                    in_block_comment = true;
                    ++i;
                } else if (c == '"') {
                    state = State::InString;
                    out.push_back(c);
                } else if (c == '\'') {
                    state = State::InChar;
                    out.push_back(c);
                } else {
                    out.push_back(c);
                }
                break;

            case State::InString:
                out.push_back(c);
                if (c == '\\' && next != '\0') {
                    out.push_back(next);
                    ++i;
                } else if (c == '"') {
                    state = State::Normal;
                }
                break;

            case State::InChar:
                out.push_back(c);
                if (c == '\\' && next != '\0') {
                    out.push_back(next);
                    ++i;
                } else if (c == '\'') {
                    state = State::Normal;
                }
                break;

            case State::InLineComment:
                break;

            case State::InBlockComment:
                if (c == '*' && next == '/') {
                    state = State::Normal;
                    in_block_comment = false;
                    ++i;
                }
                break;
        }
    }

    return out;
}

bool contains_function_call(const std::string& code,
                            const std::string& func_name,
                            std::size_t* first_pos = nullptr) {
    std::size_t pos = 0;
    while ((pos = code.find(func_name, pos)) != std::string::npos) {
        bool left_ok = (pos == 0) ||
            (!std::isalnum(static_cast<unsigned char>(code[pos - 1])) &&
             code[pos - 1] != '_');
        std::size_t end = pos + func_name.size();
        while (end < code.size() &&
               std::isspace(static_cast<unsigned char>(code[end]))) {
            ++end;
        }
        bool right_ok = (end < code.size() && code[end] == '(');
        if (left_ok && right_ok) {
            if (first_pos) {
                *first_pos = pos;
            }
            return true;
        }
        pos += func_name.size();
    }
    return false;
}

bool scanf_format_has_unbounded_s(const std::string& code,
                                  std::size_t func_pos) {
    std::size_t paren = code.find('(', func_pos);
    if (paren == std::string::npos) return false;
    std::size_t first_quote = code.find('"', paren);
    if (first_quote == std::string::npos) return false;

    bool escape = false;
    for (std::size_t i = first_quote + 1; i < code.size(); ++i) {
        char c = code[i];
        if (!escape && c == '\\') {
            escape = true;
            continue;
        }
        if (!escape && c == '"') {
            break;
        }
        escape = false;

        if (c == 's' && i > first_quote && code[i - 1] == '%') {
            bool has_digit = false;
            std::size_t j = i - 1;
            while (j > first_quote && code[j] != '%') {
                if (std::isdigit(static_cast<unsigned char>(code[j]))) {
                    has_digit = true;
                    break;
                }
                --j;
            }
            if (!has_digit) {
                return true;
            }
        }
    }
    return false;
}

void add_source_finding(Findings& out,
                        const std::string& id,
                        Severity severity,
                        const std::string& msg,
                        const std::string& file,
                        std::size_t line,
                        const std::string& recommendation) {
    Finding f;
    f.kind  = FindingKind::Source;
    f.severity = severity;
    f.id    = id;
    f.message = msg;
    f.recommendation = recommendation;
    f.source_location.file = file;
    f.source_location.line = line;
    out.push_back(std::move(f));
}

std::string to_hex64(std::uint64_t v) {
    std::ostringstream oss;
    oss << "0x" << std::hex << v;
    return oss.str();
}

void add_binary_finding(Findings& out,
                        const std::string& id,
                        Severity severity,
                        const std::string& msg,
                        const std::string& arch,
                        const std::string& segment = {},
                        std::uint64_t offset = 0,
                        const std::string& recommendation = {},
                        const std::string& disasm = {}) {
    Finding f;
    f.kind = FindingKind::Binary;
    f.severity = severity;
    f.id = id;
    f.message = msg;
    f.recommendation = recommendation;
    f.binary_location.arch = arch;
    f.binary_location.segment_or_section = segment;
    f.binary_location.offset = offset;
    f.binary_location.disasm = disasm;
    out.push_back(std::move(f));
}

} // namespace

Findings SourceDetector::analyze_path(const std::string& path_str) const {
    Findings out;
    fs::path root(path_str);

    if (!fs::exists(root)) {
        add_source_finding(out,
                           "SRC_PATH_NOT_FOUND",
                           Severity::Info,
                           "Source path does not exist: " + path_str,
                           path_str,
                           0,
                           "Check the path or adjust SentinelX arguments.");
        return out;
    }

    auto analyze_file = [&out](const fs::path& file) {
        const std::string file_str = file.string();
        bool in_block_comment = false;
        int  brace_depth = 0;

        std::vector<std::string> lines;
        try {
            lines = read_lines(file_str);
        } catch (const std::exception& ex) {
            add_source_finding(out,
                               "SRC_READ_ERROR",
                               Severity::Info,
                               std::string("Failed to read file: ") + ex.what(),
                               file_str,
                               0,
                               "Check file permissions.");
            return;
        }

        static const std::vector<std::string> dangerous_calls = {
            "strcpy", "wcscpy", "strcat", "wcscat",
            "gets", "sprintf", "vsprintf"
        };
        static const std::vector<std::string> scanf_like = {
            "scanf", "fscanf", "sscanf"
        };

        sx::BufferFSM buffer_fsm;
        const std::string current_function = "<unknown>";

        static const std::regex char_array_decl_re(
            R"(\bchar\s+([A-Za-z_]\w*)\s*$$\s*(\d+)\s*$$)");

        static const std::regex memcpy_re(
            R"(\bmemcpy\s*$\s*([A-Za-z_]\w*)\s*,\s*[^,]+,\s*(\d+)\s*$)");
        static const std::regex read_re(
            R"(\bread\s*$\s*[^,]+,\s*([A-Za-z_]\w*)\s*,\s*(\d+)\s*$)");

        const std::size_t LARGE_STACK_THRESHOLD = 1024; // bytes

        for (std::size_t i = 0; i < lines.size(); ++i) {
            const std::size_t line_no = i + 1;
            const std::string& raw_line = lines[i];

            if (raw_line.find("SENTINELX_IGNORE") != std::string::npos ||
                raw_line.find("SENTINELX:IGNORE") != std::string::npos) {
                continue;
            }

            std::string code = strip_comments_preserve_strings(raw_line,
                                                               in_block_comment);
            code = trim(code);
            if (code.empty()) {
                // обновляем глубину скобок даже для пустых строк после trim
                for (char c : raw_line) {
                    if (c == '{') ++brace_depth;
                    else if (c == '}' && brace_depth > 0) --brace_depth;
                }
                continue;
            }

            bool inside_block = (brace_depth > 0) ||
                                (code.find('{') != std::string::npos);

            for (const auto& func : dangerous_calls) {
                if (contains_function_call(code, func)) {
                    Severity sev = Severity::High;
                    if (func == "gets") {
                        sev = Severity::Critical;
                    }
                    std::string msg =
                        "Call to potentially unsafe function '" + func + "' "
                        "without explicit bounds.";
                    std::string rec =
                        "Prefer bounded alternatives (e.g. strncpy, strlcpy, "
                        "snprintf) and ensure buffer size checks.";

                    add_source_finding(out,
                                       "SRC_UNSAFE_CALL_" + func,
                                       sev,
                                       msg,
                                       file_str,
                                       line_no,
                                       rec);
                }
            }

            for (const auto& func : scanf_like) {
                std::size_t pos = 0;
                if (contains_function_call(code, func, &pos) &&
                    scanf_format_has_unbounded_s(code, pos)) {

                    std::string msg =
                        "Use of '" + func +
                        "' with '%s' format specifier without width "
                        "may lead to buffer overflow.";
                    std::string rec =
                        "Specify maximum field width in the format string "
                        "or replace with safer input handling.";

                    add_source_finding(out,
                                       "SRC_SCANF_UNBOUNDED",
                                       Severity::High,
                                       msg,
                                       file_str,
                                       line_no,
                                       rec);
                }
            }

            if (inside_block) {
                std::smatch m;
                if (std::regex_search(code, m, char_array_decl_re)) {
                    std::string buf_name = m[1].str();
                    std::size_t size = 0;
                    try {
                        size = static_cast<std::size_t>(std::stoull(m[2].str()));
                    } catch (...) {
                        size = 0;
                    }

                    sx::BufferKey key{file_str, current_function, buf_name};
                    sx::SourceLocation loc{file_str, static_cast<int>(line_no)};
                    buffer_fsm.on_declare(key, size, loc);

                    if (size >= LARGE_STACK_THRESHOLD) {
                        std::string msg =
                            "Large stack buffer '" + buf_name + "[" +
                            m[2].str() + "]' may contribute to stack overflow.";
                        std::string rec =
                            "Consider using dynamic allocation (heap) or "
                            "reducing buffer size.";

                        add_source_finding(out,
                                           "SRC_LARGE_STACK_BUFFER",
                                           Severity::Warning,
                                           msg,
                                           file_str,
                                           line_no,
                                           rec);
                    }
                }

                // memcpy(dest, src, N)
                {
                    std::smatch m;
                    if (std::regex_search(code, m, memcpy_re)) {
                        std::string dest = m[1].str();
                        std::size_t bytes = 0;
                        try {
                            bytes = static_cast<std::size_t>(std::stoull(m[2].str()));
                        } catch (...) {
                            bytes = 0;
                        }

                        sx::BufferKey key{file_str, current_function, dest};
                        sx::SourceLocation loc{file_str, static_cast<int>(line_no)};
                        buffer_fsm.on_write(key, bytes, loc);

                        if (buffer_fsm.state_of(key) == sx::BufferState::Tainted) {
                            std::string msg =
                                "Write of " + std::to_string(bytes) +
                                " bytes into buffer '" + dest +
                                "' may exceed its declared size.";
                            std::string rec =
                                "Ensure memcpy size does not exceed "
                                "destination buffer size.";

                            add_source_finding(out,
                                               "SRC_BUFFER_OVERFLOW_MEMCPY",
                                               Severity::High,
                                               msg,
                                               file_str,
                                               line_no,
                                               rec);
                        }
                    }
                }

                // read(fd, buf, N)
                {
                    std::smatch m;
                    if (std::regex_search(code, m, read_re)) {
                        std::string dest = m[1].str();
                        std::size_t bytes = 0;
                        try {
                            bytes = static_cast<std::size_t>(std::stoull(m[2].str()));
                        } catch (...) {
                            bytes = 0;
                        }

                        sx::BufferKey key{file_str, current_function, dest};
                        sx::SourceLocation loc{file_str, static_cast<int>(line_no)};
                        buffer_fsm.on_write(key, bytes, loc);

                        if (buffer_fsm.state_of(key) == sx::BufferState::Tainted) {
                            std::string msg =
                                "read() of " + std::to_string(bytes) +
                                " bytes into buffer '" + dest +
                                "' may exceed its declared size.";
                            std::string rec =
                                "Ensure read() third argument does not exceed "
                                "destination buffer size.";

                            add_source_finding(out,
                                               "SRC_BUFFER_OVERFLOW_READ",
                                               Severity::High,
                                               msg,
                                               file_str,
                                               line_no,
                                               rec);
                        }
                    }
                }
            }

            // Обновляем глубину фигурных скобок
            for (char c : code) {
                if (c == '{') ++brace_depth;
                else if (c == '}' && brace_depth > 0) --brace_depth;
            }
        }

        // В конце файла можно было бы пробежаться по snapshot() и
        // сгенерировать дополнительные находки по состоянию FSM,
        // но сейчас все overflow'ы уже зафиксированы в момент on_write().
        (void)buffer_fsm;
        (void)current_function;
    };

    if (fs::is_regular_file(root)) {
        if (is_source_file(root)) {
            analyze_file(root);
        }
    } else if (fs::is_directory(root)) {
        for (const auto& entry : fs::recursive_directory_iterator(root)) {
            if (entry.is_regular_file() && is_source_file(entry.path())) {
                analyze_file(entry.path());
            }
        }
    }

    return out;
}

Findings BinaryDetector::analyze_binary(const std::string& path) const {
    Findings out;

#ifndef SENTINELX_USE_LIEF
    fs::path p(path);
    if (!fs::exists(p)) {
        add_binary_finding(out,
                           "BIN_PATH_NOT_FOUND",
                           Severity::Info,
                           "Binary path does not exist: " + path,
                           "unknown",
                           {},
                           0,
                           "Check the path or adjust SentinelX arguments.");
        return out;
    }

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        add_binary_finding(out,
                           "BIN_READ_ERROR",
                           Severity::Info,
                           "Failed to open binary file: " + path,
                           "unknown",
                           {},
                           0,
                           "Check file permissions.");
        return out;
    }

    std::string content((std::istreambuf_iterator<char>(ifs)),
                        std::istreambuf_iterator<char>());

    static const std::vector<std::string> dangerous_imports = {
        "strcpy", "wcscpy", "strcat", "wcscat", "gets",
        "sprintf", "vsprintf"
    };

    for (const auto& d : dangerous_imports) {
        std::size_t pos = content.find(d);
        while (pos != std::string::npos) {
            std::string msg =
                "Binary contains reference to potentially unsafe function '" +
                d + "' at file offset " + to_hex64(static_cast<std::uint64_t>(pos)) + ".";

            std::string rec =
                "This is a heuristic scan without LIEF. "
                "Consider enabling LIEF for accurate disassembly and call-site analysis.";

            add_binary_finding(out,
                               "BIN_UNSAFE_BYTES_" + d,
                               Severity::Warning,
                               msg,
                               "unknown",
                               "raw",
                               static_cast<std::uint64_t>(pos),
                               rec);

            pos = content.find(d, pos + 1);
        }
    }

    if (out.empty()) {
        add_binary_finding(out,
                           "BIN_ANALYSIS_LIMITED",
                           Severity::Info,
                           "Binary analysis performed without LIEF; no obvious unsafe function names found.",
                           "unknown");
    }

    return out;
#else
    fs::path p(path);
    if (!fs::exists(p)) {
        add_binary_finding(out,
                           "BIN_PATH_NOT_FOUND",
                           Severity::Info,
                           "Binary path does not exist: " + path,
                           "unknown",
                           {},
                           0,
                           "Check the path or adjust SentinelX arguments.");
        return out;
    }

    BinaryParser parser;
    BinaryInfo info;
    try {
        info = parser.parse(path);
    } catch (const std::exception& ex) {
        add_binary_finding(out,
                           "BIN_PARSE_ERROR",
                           Severity::Info,
                           std::string("Failed to parse binary with LIEF: ") +
                               ex.what(),
                           "unknown",
                           {},
                           0,
                           "Ensure the file is a supported executable format.");
        return out;
    }

    static const std::vector<std::string> dangerous_imports = {
        "strcpy", "wcscpy", "strcat", "wcscat", "gets",
        "sprintf", "vsprintf"
    };

    Disassembler disasm;

    for (const auto& imp : info.imported_functions) {
        std::string lower = to_lower(imp);
        for (const auto& d : dangerous_imports) {
            if (lower == d) {
                // Дизассемблируем PLT/обёртку импортируемой функции
                auto insts = disasm.disassemble_function(info, imp);
                std::string snippet;
                std::uint64_t first_addr = 0;

                const std::size_t max_lines = 8;
                std::size_t count = std::min(max_lines, insts.size());
                for (std::size_t i = 0; i < count; ++i) {
                    if (i == 0 && !insts.empty()) {
                        first_addr = insts[i].address;
                    }
                    snippet += to_hex64(insts[i].address);
                    snippet += ": ";
                    snippet += insts[i].text;
                    snippet += "\n";
                }

                std::string msg =
                    "Binary imports potentially unsafe function '" + imp + "'.";

                std::string rec =
                    "Review all call sites of '" + imp +
                    "' and consider replacing with bounded alternatives.";

                add_binary_finding(out,
                                   "BIN_UNSAFE_IMPORT_" + d,
                                   Severity::Warning,
                                   msg,
                                   info.arch,
                                   "",           
                                   first_addr,
                                   rec,
                                   snippet);
                break;
            }
        }
    }

    return out;
#endif
}

} // namespace sentinel
