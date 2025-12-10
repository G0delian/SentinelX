#include "analyzer.h"

#include "detectors.h"

namespace sentinel {

Analyzer::Analyzer(AnalyzerConfig config)
    : config_(config) {}

Findings Analyzer::analyze(const std::vector<std::string>& source_paths,
                           const std::vector<std::string>& binary_paths) const {
    Findings all;

    SourceDetector source_detector;
    BinaryDetector binary_detector;

    if (config_.analyze_source) {
        for (const auto& p : source_paths) {
            Findings f = source_detector.analyze_path(p);
            all.insert(all.end(), f.begin(), f.end());
        }
    }

    if (config_.analyze_binary) {
        for (const auto& p : binary_paths) {
            Findings f = binary_detector.analyze_binary(p);
            all.insert(all.end(), f.begin(), f.end());
        }
    }

    return all;
}

} // namespace sentinel
