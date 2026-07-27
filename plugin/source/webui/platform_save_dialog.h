#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace poly {

// Open a native "Save As…" dialog and, if the user picks a path, write the
// given bytes to it. Runs modal to the parent window; must be called from the
// UI thread. Never blocks the audio thread — WebUI actions arrive on the UI
// thread via choc::ui::WebView's bind() dispatcher.
//
// Arguments:
//   parentView     platform-native parent handle (NSView* on macOS, HWND on
//                  Windows). May be null; the dialog opens without an owner.
//   suggestedName  default filename shown in the dialog (e.g. "poly.mid").
//   bytes          payload to write once the user confirms. Copy-in — caller
//                  keeps ownership.
//   onResult       invoked on the UI thread with the final path if the user
//                  saved successfully, or empty string if cancelled/failed.
//                  Optional — pass an empty function to ignore.
void openMidiSaveDialog(void* parentView, const std::string& suggestedName, const std::vector<uint8_t>& bytes,
                        std::function<void(const std::string& savedPath)> onResult);

} // namespace poly
