#pragma once

// Web UI editor — hosts the webui/ bundle in a choc::ui::WebView and
// bridges it to the controller per webui/bridge-schema.md. Built with
// -DPOLY_WEB_UI=ON; the default build keeps the VSTGUI editor.

#include <memory>
#include <optional>
#include <string>

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
    void requestMidiExport();
    void sendCaptureCommand(const char* messageId);
    void openSaveDialogFromCache();
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
    // If the user clicks Export while dragSmfCache_ is empty, we fire a
    // MidiExport request and open the dialog on the frame tick that sees
    // the cache filled. saveDialogOpen_ prevents re-entrancy.
    bool savePending_ = false;
    bool saveDialogOpen_ = false;
    // M051 S08: capture state machine mirror (values match UISnapshot::captureState:
    // 0=idle, 1=armed, 2=capturing, 3=complete). Tracked so pushFrame can detect the
    // capturing->complete edge and invalidate any stale drag cache: each fresh
    // `complete` freezes a NEW window, so the next Export must pull the fresh frozen
    // bytes rather than a previous capture's leftover cache.
    int lastCaptureState_ = 0;
    bool freshExportPending_ = false;
};

} // namespace poly
