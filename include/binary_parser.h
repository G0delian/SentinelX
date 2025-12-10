#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "types.h"

namespace sentinel {

struct BinarySegment {
    std::string name;
    std::uint64_t virtual_address = 0;
    std::vector<std::uint8_t> content;
};

struct BinaryInfo {
    std::string path;
    std::string arch;
    std::vector<BinarySegment> executable_segments;
    std::vector<std::string> imported_functions;
};

class BinaryParser {
public:
    BinaryInfo parse(const std::string& path) const;
};

} // namespace sentinel
