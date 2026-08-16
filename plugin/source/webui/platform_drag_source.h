// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace poly {

// Open a small native drag-source window that holds a draggable MIDI clip
// proxy. The user drags the proxy straight into the DAW; the platform layer
// serves a temporary .mid file through the OS drag pasteboard (NSPasteboard /
// file URL on macOS, OLE DoDragDrop/CF_HDROP on Windows).
//
// This is additive to openMidiSaveDialog (the Save-As path) and reuses the
// same NSView*/HWND parent seam declared in platform_save_dialog.h — it does
// NOT use VSTGUI. Must be called from the UI thread; never touches the audio
// thread. WebUI actions arrive on the UI thread via choc::ui::WebView's bind()
// dispatcher.
//
// Arguments:
//   parentView     platform-native parent handle (NSView* on macOS, HWND on
//                  Windows). May be null; the window opens without an owner.
//   suggestedName  filename for the dragged clip (e.g. "poly.mid").
//   bytes          MIDI payload written to a temp file that backs the drag.
//                  Copy-in — caller keeps ownership.
void beginMidiDragExport(void* parentView, const std::string& suggestedName, const std::vector<uint8_t>& bytes);

} // namespace poly
