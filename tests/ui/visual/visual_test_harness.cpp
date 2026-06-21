//------------------------------------------------------------------------
// Copyright(c) 2025-2026 Jim Kennedy / jk.digital
// Shared VSTGUI visual test harness for all jk.digital VST3 plugins.
// Canonical source: audio-meta/test_harness/
//------------------------------------------------------------------------

#include "visual_test_harness.h"

#include <cstdio>
#include <mutex>

#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/coffscreencontext.h"
#include "vstgui/lib/platform/iplatformbitmap.h"
#include "vstgui/lib/platform/platformfactory.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <sys/stat.h>
#endif

using namespace VSTGUI;

namespace JKDigital {
namespace VisualTest {

//------------------------------------------------------------------------
static std::once_flag sPlatformInitFlag;

void initPlatformOnce() {
    std::call_once(sPlatformInitFlag, []() {
#ifdef __APPLE__
        VSTGUI::initPlatform(CFBundleGetMainBundle());
#else
        VSTGUI::initPlatform(nullptr);
#endif
    });
}

//------------------------------------------------------------------------
SharedPointer<CBitmap> renderViewToBitmap(CView* view, double scaleFactor) {
    if (!view)
        return nullptr;

    const CRect& viewSize = view->getViewSize();
    CPoint size(viewSize.getWidth(), viewSize.getHeight());

    return renderBitmapOffscreen(size, scaleFactor, [&](CDrawContext& ctx) { view->draw(&ctx); });
}

//------------------------------------------------------------------------
bool saveBitmapToPNG(CBitmap* bitmap, const std::string& path) {
    if (!bitmap || !bitmap->getPlatformBitmap())
        return false;

    auto pngData = getPlatformFactory().createBitmapMemoryPNGRepresentation(bitmap->getPlatformBitmap());

    if (pngData.empty())
        return false;

    FILE* f = fopen(path.c_str(), "wb");
    if (!f)
        return false;

    size_t written = fwrite(pngData.data(), 1, pngData.size(), f);
    fclose(f);

    return written == pngData.size();
}

//------------------------------------------------------------------------
bool loadPNG(const std::string& path, uint32_t& width, uint32_t& height, std::vector<uint8_t>& pixels) {
    auto platformBitmap = getPlatformFactory().createBitmapFromPath(path.c_str());
    if (!platformBitmap)
        return false;

    auto accessor = platformBitmap->lockPixels(true);
    if (!accessor)
        return false;

    width = static_cast<uint32_t>(platformBitmap->getSize().x);
    height = static_cast<uint32_t>(platformBitmap->getSize().y);
    uint32_t bytesPerRow = accessor->getBytesPerRow();

    pixels.resize(width * height * 4);

    auto pixelFormat = accessor->getPixelFormat();

    for (uint32_t y = 0; y < height; ++y) {
        const uint8_t* srcRow = accessor->getAddress() + y * bytesPerRow;
        uint8_t* dstRow = pixels.data() + y * width * 4;

        for (uint32_t x = 0; x < width; ++x) {
            const uint8_t* src = srcRow + x * 4;
            uint8_t r, g, b, a;

            switch (pixelFormat) {
            case IPlatformBitmapPixelAccess::kBGRA:
                b = src[0];
                g = src[1];
                r = src[2];
                a = src[3];
                break;
            case IPlatformBitmapPixelAccess::kARGB:
                a = src[0];
                r = src[1];
                g = src[2];
                b = src[3];
                break;
            case IPlatformBitmapPixelAccess::kABGR:
                a = src[0];
                b = src[1];
                g = src[2];
                r = src[3];
                break;
            case IPlatformBitmapPixelAccess::kRGBA:
            default:
                r = src[0];
                g = src[1];
                b = src[2];
                a = src[3];
                break;
            }

            dstRow[x * 4 + 0] = r;
            dstRow[x * 4 + 1] = g;
            dstRow[x * 4 + 2] = b;
            dstRow[x * 4 + 3] = a;
        }
    }

    return true;
}

//------------------------------------------------------------------------
static void ensureDir(const std::string& path) {
#ifdef __APPLE__
    mkdir(path.c_str(), 0755);
#endif
}

//------------------------------------------------------------------------
std::string getOutputDir() {
    std::string dir = std::string(VISUAL_TEST_OUTPUT_DIR);
    ensureDir(dir);
    return dir;
}

//------------------------------------------------------------------------
std::string getReferenceDir() {
    std::string dir = std::string(VISUAL_TEST_REFERENCE_DIR);
    return dir;
}

//------------------------------------------------------------------------
} // namespace VisualTest
} // namespace JKDigital
