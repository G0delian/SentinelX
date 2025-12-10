#include "arithmetic_analyzer.h"
#include "utils.h"

#include <sstream>
#include <iomanip>
#include <unordered_map>

namespace sentinel {

ArithmeticAnalyzer::ArithmeticAnalyzer(Disassembler& disasm)
    : disasm_(disasm) {}

Findings ArithmeticAnalyzer::analyze(const BinaryInfo& info) {
    Findings findings;

    auto text_insts = disasm_.disassemble_section(".text");

    if (text_insts.empty()) {
        return findings;
    }

    // Integer overflow detection: cluster arithmetic operations
    std::unordered_map<std::uint64_t, std::vector<std::size_t>> clusters;

    for (std::size_t i = 0; i < text_insts.size(); ++i) {
        const auto& inst = text_insts[i];

        // Arithmetic operations that can overflow
        if (inst.mnemonic == "add" || inst.mnemonic == "sub" ||
            inst.mnemonic == "mul" || inst.mnemonic == "imul" ||
            inst.mnemonic == "shl" || inst.mnemonic == "sal") {

            std::uint64_t cluster_key = inst.address / 256;  // 256-byte windows
            clusters[cluster_key].push_back(i);
        }
    }

    // Analyze clusters with 2+ arithmetic operations
    for (const auto& [key, indices] : clusters) {
        if (indices.size() < 2) {
            continue;
        }

        for (std::size_t idx : indices) {
            const auto& inst = text_insts[idx];

            // Check next ~5 instructions for overflow check
            bool has_overflow_check = false;
            for (std::size_t j = idx + 1; j < idx + 6 && j < text_insts.size(); ++j) {
                const auto& next = text_insts[j];

                // Overflow check instructions
                if (next.mnemonic == "jo" || next.mnemonic == "jc" ||
                    next.mnemonic == "jno" || next.mnemonic == "jnc" ||
                    next.mnemonic == "jb" || next.mnemonic == "jnb" ||
                    next.mnemonic == "ja" || next.mnemonic == "jbe") {
                    has_overflow_check = true;
                    break;
                }
            }

            if (!has_overflow_check) {
                auto context = disasm_.get_context(inst.address, text_insts, 4, 10);

                Finding f;
                f.kind = FindingKind::Binary;
                f.severity = Severity::Warning;
                f.id = "BIN_INTEGER_OVERFLOW_RISK";
                f.message = "Arithmetic operation '" + inst.mnemonic +
                           "' without overflow check at " + to_hex(inst.address);
                f.recommendation = "Add overflow checks (jo, jc, etc.) after arithmetic operations. "
                                  "Integer overflows can lead to unexpected behavior and vulnerabilities.";
                f.binary_location.arch = info.arch;
                f.binary_location.segment_or_section = ".text";
                f.binary_location.offset = inst.address;
                f.binary_location.disasm = format_context(context, inst.address);

                findings.push_back(f);
                break;  // One finding per cluster to reduce noise
            }
        }
    }

    // Format string vulnerability detection
    for (std::size_t i = 0; i < text_insts.size(); ++i) {
        const auto& inst = text_insts[i];

        if (inst.mnemonic == "call" &&
            (inst.operands.find("printf") != std::string::npos ||
             inst.operands.find("sprintf") != std::string::npos ||
             inst.operands.find("fprintf") != std::string::npos ||
             inst.operands.find("snprintf") != std::string::npos)) {

            // Check previous instructions for format string loading
            bool format_is_constant = false;

            // Look back up to 10 instructions
            for (int j = static_cast<int>(i) - 1;
                 j >= 0 && j > static_cast<int>(i) - 10;
                 --j) {
                const auto& prev = text_insts[static_cast<std::size_t>(j)];

                // x86_64 System V ABI: format string in RDI (1st arg)
                // Windows x64: format string in RCX
                // x86 (32-bit): format string pushed on stack
                if (prev.mnemonic == "lea" &&
                    (prev.operands.find("rdi") != std::string::npos ||
                     prev.operands.find("rcx") != std::string::npos ||
                     prev.operands.find("edi") != std::string::npos ||
                     prev.operands.find("ecx") != std::string::npos)) {

                    // If lea loads from rip-relative address (rodata) - constant
                    if (prev.operands.find("rip") != std::string::npos) {
                        format_is_constant = true;
                        break;
                    }
                }

                // If loading immediate address - constant
                if ((prev.mnemonic == "mov" || prev.mnemonic == "movabs") &&
                    (prev.operands.find("rdi") != std::string::npos ||
                     prev.operands.find("rcx") != std::string::npos)) {
                    // Check if moving constant (contains $ or immediate value)
                    if (prev.operands.find("$") != std::string::npos ||
                        prev.operands.find("0x") != std::string::npos) {
                        format_is_constant = true;
                        break;
                    }
                }
            }

            if (!format_is_constant) {
                auto context = disasm_.get_context(inst.address, text_insts, 6, 8);

                Finding f;
                f.kind = FindingKind::Binary;
                f.severity = Severity::High;
                f.id = "BIN_FORMAT_STRING_VULN";
                f.message = "Potential format string vulnerability at " +
                           to_hex(inst.address);
                f.recommendation = "Ensure format string is constant, not user-controlled. "
                                  "Format string attacks can lead to information disclosure and arbitrary code execution.";
                f.binary_location.arch = info.arch;
                f.binary_location.segment_or_section = ".text";
                f.binary_location.offset = inst.address;
                f.binary_location.disasm = format_context(context, inst.address);

                findings.push_back(f);
            }
        }
    }

    return findings;
}

std::string ArithmeticAnalyzer::format_context(
    const std::vector<Instruction>& context,
    std::uint64_t highlight_addr) const {

    std::ostringstream oss;
    for (const auto& inst : context) {
        if (inst.address == highlight_addr) {
            oss << ">>> ";  // Marker for problem instruction
        } else {
            oss << "    ";
        }
        oss << to_hex(inst.address) << ": " << inst.text << "\n";
    }
    return oss.str();
}

std::string ArithmeticAnalyzer::to_hex(std::uint64_t addr) const {
    std::ostringstream oss;
    oss << "0x" << std::hex << addr;
    return oss.str();
}

} // namespace sentinel
