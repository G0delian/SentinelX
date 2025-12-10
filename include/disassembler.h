#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "binary_parser.h"

namespace sentinel {

struct Instruction {
    std::uint64_t address = 0;
    std::string   text;
};

class Disassembler {
public:
    // used LIEF::Binary::disassemble
    std::vector<Instruction> disassemble_function(const BinaryInfo& binary,
                                                  const std::string& func_name) const;
};

} // namespace sentinel
