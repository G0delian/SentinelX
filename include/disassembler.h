#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "binary_parser.h"

#ifdef SENTINELX_USE_LIEF
namespace LIEF {
    class Binary;
}
#endif

namespace sentinel {

struct Instruction {
    std::uint64_t address = 0;
    std::string   text;
    std::string   mnemonic;    // "call", "sub", "add", etc.
    std::string   operands;    // "strcpy@plt", "rsp, 0x100", etc.
};

class Disassembler {
public:
    Disassembler();
    ~Disassembler();

    // Load and cache binary (call once per binary)
    void load_binary(const std::string& path);

    // Disassemble entire section (cached)
    std::vector<Instruction> disassemble_section(const std::string& section_name);

    // Get context window around address (N instructions before and after)
    std::vector<Instruction> get_context(std::uint64_t address,
                                         const std::vector<Instruction>& all_insts,
                                         std::size_t before = 4,
                                         std::size_t after = 10) const;

    // Legacy method for backward compatibility
    std::vector<Instruction> disassemble_function(const BinaryInfo& binary,
                                                  const std::string& func_name) const;

private:
    void parse_instruction(Instruction& inst) const;

#ifdef SENTINELX_USE_LIEF
    std::unique_ptr<LIEF::Binary> cached_binary_;
#endif
    std::string cached_path_;
    std::unordered_map<std::string, std::vector<Instruction>> section_cache_;
};

} // namespace sentinel
