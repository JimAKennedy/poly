// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

// Web UI editor — hosts the webui/ bundle in a choc::ui::WebView and
// bridges it to the controller per webui/bridge-schema.md. Compiled on the
// choc-webview platforms (Apple/Windows); Linux keeps the VSTGUI editor.

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/common/pluginview.h"

#include "choc/gui/choc_MessageLoop.h"
#include "poly/midi_reader.h" // M035 S03 T03: LaneSnapshot for the revertImport store
#include "poly/types.h"       // M035 S03 T03: kMaxLanes for the per-lane snapshot array

namespace choc::ui {
class WebView;
}
namespace choc::value {
class ValueView;
}

namespace poly {

class PolyController;

class WebUIView : public Steinberg::CPluginView {
public:
    explicit WebUIView(PolyController* controller);
    ~WebUIView() override;

    // IPlugView
    Steinberg::tresult PLUGIN_API isPlatformTypeSupported(Steinberg::FIDString type) override;
    Steinberg::tresult PLUGIN_API attached(void* parent, Steinberg::FIDString type) override;
    Steinberg::tresult PLUGIN_API removed() override;
    Steinberg::tresult PLUGIN_API onSize(Steinberg::ViewRect* newSize) override;

private:
    void handleHostCall(const std::string& json);
    void handleAction(const std::string& name, const choc::value::ValueView& payload);
    void applyEditToCache(Steinberg::Vst::ParamID id, double normalized);
    void pushState();
    void pushFrame();
    void startFrameTimer();
    void stopFrameTimer();
    void resizeWebviewToRect(const Steinberg::ViewRect& r);
    void sendCaptureCommand(const char* messageId);
    // M035 S03 T03: drive the mutated lane's core params + timeline/micro-timing
    // sends so the audio thread and UI both see an imported/reverted rhythm.
    // Shared by the fitMidi (apply) and revertImport (restore) handleAction cases
    // so both surfaces reconcile exactly the fields importMidiToLane can rewrite.
    void driveLaneImportSends(int lane);
    // M053 S11: offline MIDI export helpers. renderCurrentPatternSmf renders the
    // controller's live cachedState to an SMF blob with no DAW transport; the two
    // sinks feed those bytes to the Save-As panel / native drag source.
    // M032 S02 (T03): laneFilter forwards to renderPatternToSMF — -1 (default)
    // exports every active lane; N in [0, kMaxLanes) exports only lane N as a
    // single named MTrk. The WebUI Export chip passes the payload's optional
    // {lane} through so a per-lane drag/save emits just that lane.
    std::vector<uint8_t> renderCurrentPatternSmf(int laneFilter = -1) const;
    void openMidiExportDialog(const std::vector<uint8_t>& bytes);
    void beginDragExport(const std::vector<uint8_t>& bytes);
    std::string suggestedExportName() const;
    // Push an exportResult message to the WebUI (toast / clear pending). Empty
    // path = cancelled. Shared by the Save-As completion callback and the
    // POLY_EXPORT_SINK test path.
    void pushExportResult(const std::string& savedPath);

    PolyController* controller_ = nullptr;
    std::unique_ptr<choc::ui::WebView> webview_;
    std::optional<choc::messageloop::Timer> frameTimer_;
    uint32_t lastStateGen_ = 0;
    int editCooldown_ = 0;
    bool webviewReady_ = false;
    std::string lastPushedJson_;
    std::string currentPresetName_;
    // Host-provided platform parent for modal dialogs (NSView* on macOS,
    // HWND on Windows). Captured in attached() so exportSaveAs can anchor
    // its NSSavePanel / IFileSaveDialog to the correct window.
    void* parentView_ = nullptr;
    // Re-entrancy guard for the modal Save-As panel: exportSaveAs ignores clicks
    // while a panel is already open (openMidiExportDialog clears it in the
    // completion callback).
    bool saveDialogOpen_ = false;

    // M035 S03 T03: per-lane pre-import snapshots backing the revertImport bridge
    // action (D039). fitMidi captures the target lane here immediately before the
    // import overwrites it and retains the snapshot only on success — a rejected
    // drop leaves no stale snapshot to revert into (S03 must-have 6). Keyed by
    // lane index so the bridge payload stays {lane}-only; no LaneConfig ever
    // crosses the JS boundary. Symmetric with the wasm Context's importSnapshots.
    std::array<poly::LaneSnapshot, kMaxLanes> importSnapshots_{};
};

} // namespace poly
