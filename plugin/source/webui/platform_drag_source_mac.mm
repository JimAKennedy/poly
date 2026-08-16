// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
// macOS native drag-source window for MIDI export. Opens a small floating
// panel containing a draggable clip proxy; the user drags it into the DAW,
// which pulls a temporary .mid file through the drag pasteboard. Called from
// the UI thread by WebUIView's beginMidiDrag action. Additive to
// openMidiSaveDialog (see platform_save_dialog_mac.mm); does NOT use VSTGUI.

#include <fstream>

#import <AppKit/AppKit.h>

#include "platform_drag_source.h"

namespace {

// Write the export bytes to a temp .mid file and return its path, or nil on
// failure. The DAW reads this file when the drag is dropped, so it must
// outlive the drag session — a temp-dir file is the simplest durable backing.
NSString* writeTempMidi(const std::string& suggestedName, const std::vector<uint8_t>& bytes) {
    NSString* name = [NSString stringWithUTF8String:suggestedName.c_str()];
    if (name.length == 0)
        name = @"poly.mid";
    NSString* path = [NSTemporaryDirectory() stringByAppendingPathComponent:name];
    std::ofstream out(path.UTF8String, std::ios::binary);
    if (!out)
        return nil;
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return out ? path : nil;
}

} // namespace

// The proxy the user drags into the DAW. On drag it begins a real
// NSDraggingSession carrying the temp .mid file URL (NSURL conforms to
// NSPasteboardWriting, so the DAW receives a file drop / CF_HDROP equivalent).
@interface PolyDragSourceView : NSView <NSDraggingSource>
@property(nonatomic, strong) NSURL* fileURL;
@property(nonatomic, copy) NSString* label;
@end

@implementation PolyDragSourceView

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect {
    [[NSColor windowBackgroundColor] setFill];
    NSRectFill(dirtyRect);

    NSRect border = NSInsetRect(self.bounds, 4.0, 4.0);
    NSBezierPath* box = [NSBezierPath bezierPathWithRoundedRect:border xRadius:6.0 yRadius:6.0];
    [[NSColor secondaryLabelColor] setStroke];
    box.lineWidth = 1.0;
    [box stroke];

    if (self.label.length > 0) {
        NSDictionary* attrs = @{
            NSFontAttributeName : [NSFont systemFontOfSize:11.0],
            NSForegroundColorAttributeName : [NSColor labelColor]
        };
        NSSize sz = [self.label sizeWithAttributes:attrs];
        NSPoint origin =
            NSMakePoint((self.bounds.size.width - sz.width) * 0.5, (self.bounds.size.height - sz.height) * 0.5);
        [self.label drawAtPoint:origin withAttributes:attrs];
    }
}

- (NSImage*)dragImage {
    NSImage* image = [[NSImage alloc] initWithSize:self.bounds.size];
    [image lockFocus];
    [self drawRect:self.bounds];
    [image unlockFocus];
    return image;
}

- (void)mouseDragged:(NSEvent*)event {
    if (!self.fileURL)
        return;
    NSDraggingItem* item = [[NSDraggingItem alloc] initWithPasteboardWriter:self.fileURL];
    NSImage* image = [self dragImage];
    [item setDraggingFrame:NSMakeRect(0, 0, image.size.width, image.size.height) contents:image];
    [self beginDraggingSessionWithItems:@[ item ] event:event source:self];
}

- (NSDragOperation)draggingSession:(NSDraggingSession*)session
    sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
    return NSDragOperationCopy;
}

@end

// Retains the open drag windows (and their controllers) so ARC does not
// deallocate them while they are still visible. The controller removes itself
// on windowWillClose:.
static NSMutableArray* polyDragWindows(void) {
    static NSMutableArray* windows = nil;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
      windows = [[NSMutableArray alloc] init];
    });
    return windows;
}

@interface PolyDragWindowController : NSObject <NSWindowDelegate>
@property(nonatomic, strong) NSWindow* window;
@end

@implementation PolyDragWindowController
- (void)windowWillClose:(NSNotification*)notification {
    [polyDragWindows() removeObject:self];
}
@end

namespace poly {

void beginMidiDragExport(void* parentView, const std::string& suggestedName, const std::vector<uint8_t>& bytes) {
    NSString* path = writeTempMidi(suggestedName, bytes);
    if (!path)
        return;
    NSURL* fileURL = [NSURL fileURLWithPath:path];

    const CGFloat width = 180.0;
    const CGFloat height = 64.0;
    NSRect contentRect = NSMakeRect(0, 0, width, height);
    NSPanel* panel = [[NSPanel alloc]
        initWithContentRect:contentRect
                  styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskUtilityWindow)
                    backing:NSBackingStoreBuffered
                      defer:NO];
    panel.title = @"Drag to DAW";
    panel.floatingPanel = YES;
    panel.hidesOnDeactivate = NO;
    [panel setReleasedWhenClosed:NO];

    PolyDragSourceView* view = [[PolyDragSourceView alloc] initWithFrame:contentRect];
    view.fileURL = fileURL;
    view.label = [NSString stringWithUTF8String:suggestedName.c_str()];
    panel.contentView = view;

    // Position over the parent window when we have one, otherwise center on
    // the active screen.
    NSWindow* parentWindow = nil;
    if (parentView) {
        NSView* parent = (__bridge NSView*)parentView;
        parentWindow = [parent window];
    }
    if (parentWindow) {
        NSRect pf = parentWindow.frame;
        [panel setFrameOrigin:NSMakePoint(NSMidX(pf) - width * 0.5, NSMidY(pf) - height * 0.5)];
    } else {
        [panel center];
    }

    PolyDragWindowController* controller = [[PolyDragWindowController alloc] init];
    controller.window = panel;
    panel.delegate = controller;
    [polyDragWindows() addObject:controller];

    [panel makeKeyAndOrderFront:nil];
}

} // namespace poly
