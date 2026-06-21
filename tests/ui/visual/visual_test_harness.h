//------------------------------------------------------------------------
// Copyright(c) 2025-2026 Jim Kennedy / jk.digital
// Shared VSTGUI visual test harness for all jk.digital VST3 plugins.
// Canonical source: audio-meta/test_harness/
//------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/cpoint.h"
#include "vstgui/lib/cview.h"

namespace JKDigital {
namespace VisualTest {

//------------------------------------------------------------------------
/**
 * Initializes VSTGUI platform for headless offscreen rendering.
 * Must be called once before any rendering. Safe to call multiple times.
 */
void initPlatformOnce();

//------------------------------------------------------------------------
/**
 * Render a CView to an offscreen bitmap.
 *
 * @param view The view to render (must have valid size via getViewSize)
 * @param scaleFactor The bitmap scale factor (1.0 = 1x, 2.0 = retina)
 * @return Shared pointer to the rendered bitmap, or nullptr on failure
 */
VSTGUI::SharedPointer<VSTGUI::CBitmap> renderViewToBitmap(
    VSTGUI::CView* view, double scaleFactor = 1.0);

//------------------------------------------------------------------------
/**
 * Save a VSTGUI bitmap to a PNG file on disk.
 *
 * @param bitmap The bitmap to save
 * @param path Absolute file path for the output PNG
 * @return true on success
 */
bool saveBitmapToPNG(VSTGUI::CBitmap* bitmap, const std::string& path);

//------------------------------------------------------------------------
/**
 * Read raw RGBA pixel data from a PNG file.
 *
 * @param path Absolute path to a PNG file
 * @param width Output: image width
 * @param height Output: image height
 * @param pixels Output: RGBA pixel data (4 bytes per pixel)
 * @return true on success
 */
bool loadPNG(const std::string& path, uint32_t& width, uint32_t& height,
             std::vector<uint8_t>& pixels);

//------------------------------------------------------------------------
/**
 * Get the output directory for visual test artifacts.
 * Creates the directory if it doesn't exist.
 *
 * @return Absolute path to the visual test output directory
 */
std::string getOutputDir();

//------------------------------------------------------------------------
/**
 * Get the reference images directory.
 *
 * @return Absolute path to the reference images directory
 */
std::string getReferenceDir();

//------------------------------------------------------------------------
}  // namespace VisualTest
}  // namespace JKDigital
