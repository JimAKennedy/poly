// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#include "controller.h"

// The shipping macOS/Windows plugin uses the choc-webview editor. On other
// platforms (Linux) createView returns nullptr — the plugin loads with no
// editor until choc's WebKitGTK backend lands in M054.
#if defined(__APPLE__) || defined(_WIN32)
#include "web_ui_view.h"
#endif

namespace poly {

Steinberg::IPlugView* PLUGIN_API PolyController::createView(Steinberg::FIDString name) {
    if (Steinberg::FIDStringsEqual(name, Steinberg::Vst::ViewType::kEditor)) {
#if defined(__APPLE__) || defined(_WIN32)
        return new WebUIView(this); // ownership-transfer
#else
        return nullptr;
#endif
    }
    return nullptr;
}

} // namespace poly
