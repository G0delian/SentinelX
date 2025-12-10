#include "disassembler.h"

#ifdef SENTINELX_USE_LIEF
#  include <LIEF/LIEF.hpp>
#endif

namespace sentinel {

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
        result.push_back(std::move(i));
    }
#else
    (void)info;
    (void)func_name;
#endif

    return result;
}

} // namespace sentinel
