//------------------------------------------------------------------------
// Copyright(c) 2025-2026 Jim Kennedy / jk.digital
// Shared pixel-comparison utility for visual regression tests.
// Canonical source: audio-meta/test_harness/
//------------------------------------------------------------------------

#include "image_compare.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#endif

namespace JKDigital {
namespace VisualTest {

//------------------------------------------------------------------------
CompareResult compareImages(const std::vector<uint8_t>& actual, const std::vector<uint8_t>& reference, uint32_t width,
                            uint32_t height, uint8_t tolerance, double maxDiffPercent) {
    CompareResult result;
    result.totalPixels = width * height;

    if (actual.size() != reference.size() || actual.size() != static_cast<size_t>(width) * height * 4) {
        result.matched = false;
        result.diffPixels = result.totalPixels;
        result.diffPercentage = 100.0;
        return result;
    }

    for (uint32_t i = 0; i < result.totalPixels; ++i) {
        size_t offset = static_cast<size_t>(i) * 4;
        bool pixelMatch = true;

        for (int c = 0; c < 4; ++c) {
            int diff = std::abs(static_cast<int>(actual[offset + c]) - static_cast<int>(reference[offset + c]));
            if (diff > tolerance) {
                pixelMatch = false;
                break;
            }
        }

        if (!pixelMatch) {
            result.diffPixels++;
        }
    }

    result.diffPercentage =
        result.totalPixels > 0 ? (static_cast<double>(result.diffPixels) / result.totalPixels) * 100.0 : 0.0;
    result.matched = (result.diffPercentage <= maxDiffPercent);

    return result;
}

//------------------------------------------------------------------------
bool generateDiffImage(const std::vector<uint8_t>& actual, const std::vector<uint8_t>& reference, uint32_t width,
                       uint32_t height, const std::string& outputPath, uint8_t tolerance) {
    if (actual.size() != reference.size() || actual.size() != static_cast<size_t>(width) * height * 4) {
        return false;
    }

    std::vector<uint8_t> diffPixels(width * height * 4);
    uint32_t totalPixels = width * height;

    for (uint32_t i = 0; i < totalPixels; ++i) {
        size_t offset = static_cast<size_t>(i) * 4;
        bool pixelMatch = true;

        for (int c = 0; c < 4; ++c) {
            int diff = std::abs(static_cast<int>(actual[offset + c]) - static_cast<int>(reference[offset + c]));
            if (diff > tolerance) {
                pixelMatch = false;
                break;
            }
        }

        if (pixelMatch) {
            diffPixels[offset + 0] = actual[offset + 0] / 4;
            diffPixels[offset + 1] = actual[offset + 1] / 4;
            diffPixels[offset + 2] = actual[offset + 2] / 4;
            diffPixels[offset + 3] = 255;
        } else {
            diffPixels[offset + 0] = 255;
            diffPixels[offset + 1] = 0;
            diffPixels[offset + 2] = 255;
            diffPixels[offset + 3] = 255;
        }
    }

    return writeRGBAToPNG(diffPixels, width, height, outputPath);
}

//------------------------------------------------------------------------
bool writeRGBAToPNG(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, const std::string& path) {
#ifdef __APPLE__
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    if (!colorSpace)
        return false;

    CGContextRef ctx = CGBitmapContextCreate(const_cast<uint8_t*>(pixels.data()), width, height, 8, width * 4,
                                             colorSpace, kCGImageAlphaPremultipliedLast);

    CGColorSpaceRelease(colorSpace);
    if (!ctx)
        return false;

    CGImageRef image = CGBitmapContextCreateImage(ctx);
    CGContextRelease(ctx);
    if (!image)
        return false;

    CFStringRef cfPath = CFStringCreateWithCString(nullptr, path.c_str(), kCFStringEncodingUTF8);
    CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, cfPath, kCFURLPOSIXPathStyle, false);
    CFRelease(cfPath);

    if (!url) {
        CGImageRelease(image);
        return false;
    }

    CGImageDestinationRef dest = CGImageDestinationCreateWithURL(url, CFSTR("public.png"), 1, nullptr);
    CFRelease(url);

    if (!dest) {
        CGImageRelease(image);
        return false;
    }

    CGImageDestinationAddImage(dest, image, nullptr);
    bool ok = CGImageDestinationFinalize(dest);

    CFRelease(dest);
    CGImageRelease(image);

    return ok;
#else
    (void)pixels;
    (void)width;
    (void)height;
    (void)path;
    return false;
#endif
}

//------------------------------------------------------------------------
} // namespace VisualTest
} // namespace JKDigital
