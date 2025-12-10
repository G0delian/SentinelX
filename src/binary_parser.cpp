#include "binary_parser.h"
#include "utils.h"

#include <stdexcept>

#ifdef SENTINELX_USE_LIEF
#  include <LIEF/LIEF.hpp>
#endif

namespace sentinel {

BinaryInfo BinaryParser::parse(const std::string& path) const {
    BinaryInfo info;
    info.path = path;
    info.arch = "unknown";

#ifdef SENTINELX_USE_LIEF
    std::unique_ptr<LIEF::Binary> binary = LIEF::Parser::parse(path);
    if (!binary) {
        throw std::runtime_error("LIEF failed to parse binary: " + path);
    }

    // Архитектура
    const auto header = binary->header();
    using Arch = LIEF::Header::ARCHITECTURES;
    switch (header.architecture()) {
        case Arch::X86:     info.arch = "x86";     break;
        case Arch::X86_64:  info.arch = "x86_64";  break;
        case Arch::ARM:     info.arch = "arm";     break;
        case Arch::ARM64:   info.arch = "arm64";   break;
        case Arch::PPC:     info.arch = "ppc";     break;
        case Arch::PPC64:   info.arch = "ppc64";   break;
        default:            info.arch = "unknown"; break;
    }

    auto sections = binary->sections();
    for (const LIEF::Section& sec : sections) {
        if (sec.size() == 0) {
            continue;
        }
        std::string name = sec.name();
        std::string lower = to_lower(name);
        bool looks_like_code =
            (lower.find("text") != std::string::npos) ||
            (lower.find("code") != std::string::npos);

        if (!looks_like_code) {
            continue;
        }

        BinarySegment seg;
        seg.name = name;
        seg.virtual_address = sec.virtual_address();

        auto span = sec.content();
        seg.content.assign(span.begin(), span.end());

        info.executable_segments.push_back(std::move(seg));
    }

    auto imported = binary->imported_functions();
    for (const LIEF::Function& f : imported) {
        info.imported_functions.push_back(f.name());
    }

#else
    (void)path;
#endif

    return info;
}

} // namespace sentinel
