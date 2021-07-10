//
// Created by coder2k on 04.07.2021.
//

#include "ScopedTimer.hpp"
#include <spdlog/spdlog.h>
#include <gsl/gsl>
#include <vector>
#include <utility>
#include <algorithm>
#include <ratio>
#include <filesystem>
#include <cmath>

ScopedTimer::ScopedTimer(const char* name, std::source_location sourceLocation) noexcept
    : mName(name),
      mSourceLocation{ sourceLocation },
      startTime{ std::chrono::high_resolution_clock::now() } {
    ++sCurrentDepth;
}

ScopedTimer::~ScopedTimer() {
    auto endTime = std::chrono::high_resolution_clock::now();
    --sCurrentDepth;
    auto duration = std::chrono::duration<double>(endTime - startTime).count();
    auto locationString = sourceLocationToString(mName, mSourceLocation);
    auto findIt = sMeasurements.find(locationString);
    if (findIt == sMeasurements.end()) {// not found in map
        sMeasurements[locationString] = Measurement{ .count = 1,
                                                     .depth = sCurrentDepth,
                                                     .minDuration = duration,
                                                     .maxDuration = duration,
                                                     .totalDuration = duration };
    } else {
        Measurement& measurement = findIt->second;
        ++measurement.count;
        measurement.minDuration = std::min(measurement.minDuration, duration);
        measurement.maxDuration = std::max(measurement.maxDuration, duration);
        measurement.totalDuration += duration;
    }
}

std::string ScopedTimer::sourceLocationToString(const std::string& name,
                                                const std::source_location& sourceLocation) noexcept {
    auto path = std::filesystem::path(sourceLocation.file_name());
    if (name.empty()) {
        return fmt::format("[{}, {}]{}", path.filename().string(), sourceLocation.line(),
                           sourceLocation.function_name());
    }
    return fmt::format("[{}, {}]{}", path.filename().string(), sourceLocation.line(), name);
}

void ScopedTimer::logResults() noexcept {
#if ENABLE_PROFILING
    std::vector<std::pair<std::string, Measurement>> measurements;
    for (const auto& dataPoint : sMeasurements) {
        measurements.emplace_back(dataPoint);
    }
    std::sort(measurements.begin(), measurements.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.second.totalDuration > rhs.second.totalDuration; });
    spdlog::info("=== Measurement Summary (all times in ms) ===");
    for (const auto& dataPoint : measurements) {
        spdlog::info("{:->{}}{}|{:.3f} ({}x)|AVG:{:.3f}|MIN: {:.3f}|MAX: {:.3f}", "",
                     dataPoint.second.depth, dataPoint.first, dataPoint.second.totalDuration * 1000.0, dataPoint.second.count,
                     dataPoint.second.totalDuration * 1000.0 / gsl::narrow_cast<double>(dataPoint.second.count),
                     dataPoint.second.minDuration * 1000.0, dataPoint.second.maxDuration * 1000.0);
    }
#else
    spdlog::info("Unable to log time measurements. #define ENABLE_PROFILING 1 to activate measuring.");
#endif
}
