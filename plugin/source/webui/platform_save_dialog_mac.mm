// macOS native Save As… dialog for MIDI export. Uses NSSavePanel with a
// sheet attached to the parent NSView's window when available, otherwise
// modal. Called from the UI thread by WebUIView's exportSaveAs action.

#import <AppKit/AppKit.h>

#include "platform_save_dialog.h"

#include <fstream>

namespace poly {

static void writeBytesToPath(const std::string& path, const std::vector<uint8_t>& bytes) {
    std::ofstream out(path, std::ios::binary);
    if (!out)
        return;
    out.write(reinterpret_cast<const char*>(bytes.data()),
              static_cast<std::streamsize>(bytes.size()));
}

void openMidiSaveDialog(void* parentView,
                        const std::string& suggestedName,
                        const std::vector<uint8_t>& bytes,
                        std::function<void(const std::string&)> onResult) {
    NSSavePanel* panel = [NSSavePanel savePanel];
    panel.title = @"Export MIDI";
    panel.allowedFileTypes = @[@"mid", @"midi"];
    panel.canCreateDirectories = YES;
    panel.nameFieldStringValue = [NSString stringWithUTF8String:suggestedName.c_str()];

    // Copy bytes into the completion block — the caller's vector may be gone
    // by the time the user confirms.
    auto bytesCopy = bytes;

    void (^handler)(NSInteger) = ^(NSInteger result) {
        std::string savedPath;
        if (result == NSModalResponseOK) {
            NSURL* url = [panel URL];
            if (url) {
                const char* utf8 = [[url path] UTF8String];
                if (utf8) {
                    savedPath = utf8;
                    writeBytesToPath(savedPath, bytesCopy);
                }
            }
        }
        if (onResult)
            onResult(savedPath);
    };

    NSWindow* parentWindow = nil;
    if (parentView) {
        NSView* view = (__bridge NSView*)parentView;
        parentWindow = [view window];
    }

    if (parentWindow) {
        [panel beginSheetModalForWindow:parentWindow completionHandler:handler];
    } else {
        NSInteger result = [panel runModal];
        handler(result);
    }
}

} // namespace poly
