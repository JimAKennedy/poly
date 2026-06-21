//------------------------------------------------------------------------
// Copyright(c) 2025-2026 Jim Kennedy / jk.digital
// Shared pixel-comparison utility for visual regression tests.
// Canonical source: audio-meta/test_harness/
//------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace JKDigital {
namespace VisualTest {

//------------------------------------------------------------------------
struct CompareResult {
    bool matched = false;
    uint32_t totalPixels = 0;
    uint32_t diffPixels = 0;
    double diffPercentage = 0.0;
    std::string diffImagePath;
};

//------------------------------------------------------------------------
CompareResult compareImages(const std::vector<uint8_t>& actual, const std::vector<uint8_t>& reference, uint32_t width,
                            uint32_t height, uint8_t tolerance = 2, double maxDiffPercent = 0.0);

//------------------------------------------------------------------------
bool generateDiffImage(const std::vector<uint8_t>& actual, const std::vector<uint8_t>& reference, uint32_t width,
                       uint32_t height, const std::string& outputPath, uint8_t tolerance = 2);

//------------------------------------------------------------------------
bool writeRGBAToPNG(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, const std::string& path);

//------------------------------------------------------------------------
} // namespace VisualTest
} // namespace JKDigital
