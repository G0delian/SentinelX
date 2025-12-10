#pragma once

#include <string>

#include "types.h"

namespace sentinel {

class SourceDetector {
public:
    Findings analyze_path(const std::string& path) const;
};

class BinaryDetector {
public:
    Findings analyze_binary(const std::string& path) const;
};

} // namespace sentinel
