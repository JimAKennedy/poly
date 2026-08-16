#pragma once

// Web UI editor — hosts the webui/ bundle in a choc::ui::WebView and
// bridges it to the controller per webui/bridge-schema.md. Compiled on the
// choc-webview platforms (Apple/Windows); Linux keeps the VSTGUI editor.

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/common/pluginview.h"

#include "choc/gui/choc_MessageLoop.h"

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
};

} // namespace poly
