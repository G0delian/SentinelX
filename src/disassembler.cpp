#include "disassembler.h"
#include "utils.h"

#ifdef SENTINELX_USE_LIEF
#  include <LIEF/LIEF.hpp>
#endif

#include <sstream>
#include <algorithm>

namespace sentinel {

Disassembler::Disassembler() = default;

Disassembler::~Disassembler() = default;

void Disassembler::load_binary(const std::string& path) {
#ifdef SENTINELX_USE_LIEF
    if (cached_path_ == path && cached_binary_) {
        return; // Already loaded
    }

    cached_binary_ = LIEF::Parser::parse(path);
    if (cached_binary_) {
        cached_path_ = path;
        section_cache_.clear(); // Clear old cache
    }
#else
    (void)path;
#endif
}

std::vector<Instruction> Disassembler::disassemble_section(const std::string& section_name) {
    // Check cache first
    auto it = section_cache_.find(section_name);
    if (it != section_cache_.end()) {
        return it->second;
    }

    std::vector<Instruction> result;

#ifdef SENTINELX_USE_LIEF
    if (!cached_binary_) {
        return result;
    }

    // Find the section
    LIEF::Section* section = nullptr;
    for (auto& sec : cached_binary_->sections()) {
        std::string sec_name = sec.name();
        if (sec_name.find(section_name) != std::string::npos) {
            section = &sec;
            break;
        }
    }

    if (!section) {
        return result;
    }

    // Disassemble the entire section
    uint64_t start_addr = section->virtual_address();
    uint64_t size = section->size();

    auto lief_insts = cached_binary_->disassemble(start_addr, size);

    for (auto inst_ptr : lief_insts) {
        if (!inst_ptr) continue;

        Instruction inst;
        inst.address = inst_ptr->address();
        inst.text = inst_ptr->to_string(true);

        // Parse mnemonic and operands
        parse_instruction(inst);

        result.push_back(std::move(inst));
    }

    // Cache the result
    section_cache_[section_name] = result;
#else
    (void)section_name;
#endif

    return result;
}

std::vector<Instruction> Disassembler::get_context(std::uint64_t address,
                                                   const std::vector<Instruction>& all_insts,
                                                   std::size_t before,
                                                   std::size_t after) const {
    std::vector<Instruction> context;

    // Find the instruction at the given address
    auto it = std::find_if(all_insts.begin(), all_insts.end(),
                          [address](const Instruction& i) {
                              return i.address == address;
                          });

    if (it == all_insts.end()) {
        return context;
    }

    std::size_t idx = static_cast<std::size_t>(std::distance(all_insts.begin(), it));

    // Calculate range
    std::size_t start_idx = (idx >= before) ? (idx - before) : 0;
    std::size_t end_idx = std::min(idx + after + 1, all_insts.size());

    // Extract context
    for (std::size_t i = start_idx; i < end_idx; ++i) {
        context.push_back(all_insts[i]);
    }

    return context;
}

void Disassembler::parse_instruction(Instruction& inst) const {
    // Parse "mnemonic operands" from text
    // Example: "call   0x401020 <strcpy@plt>" -> mnemonic="call", operands="0x401020 <strcpy@plt>"

    std::string trimmed = trim(inst.text);

    std::size_t space_pos = trimmed.find_first_of(" \t");
    if (space_pos == std::string::npos) {
        // No operands
        inst.mnemonic = trimmed;
        inst.operands = "";
    } else {
        inst.mnemonic = trimmed.substr(0, space_pos);
        inst.operands = trim(trimmed.substr(space_pos + 1));
    }
}

// Legacy method for backward compatibility
std::vector<Instruction> Disassembler::disassemble_function(const BinaryInfo& info,
                                                            const std::string& func_name) const {
    std::vector<Instruction> result;

#ifdef SENTINELX_USE_LIEF
    std::unique_ptr<LIEF::Binary> binary = LIEF::Parser::parse(info.path);
    if (!binary) {
        return result;
    }

    auto insts = binary->disassemble(func_name);
    for (auto inst : insts) {
        if (!inst) continue;
        Instruction i;
        i.address = inst->address();
        i.text = inst->to_string(true);

        // Parse mnemonic and operands for legacy calls too
        std::string trimmed = trim(i.text);
        std::size_t space_pos = trimmed.find_first_of(" \t");
        if (space_pos == std::string::npos) {
            i.mnemonic = trimmed;
            i.operands = "";
        } else {
            i.mnemonic = trimmed.substr(0, space_pos);
            i.operands = trim(trimmed.substr(space_pos + 1));
        }

        result.push_back(std::move(i));
    }
#else
    (void)info;
    (void)func_name;
#endif

    return result;
}

} // namespace sentinel
