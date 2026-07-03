#pragma once

// M028 Phase B scaffold — experimental web UI editor (build with
// -DPOLY_WEB_UI=ON). Hosts the webui/ bundle in a choc::ui::WebView and
// bridges it to the controller per webui/bridge-schema.md. The default
// build keeps the VSTGUI editor; this file is not compiled unless the flag
// is set, so the scaffold can evolve without touching the shipping UI.
//
// Spike checklist (v3-to-plugin-plan.md Phase B):
//   [ ] attach/reparent the webview child view (NSView / HWND)
//   [ ] initial `state` push on `ready`
//   [ ] `edit` -> beginEdit/performEdit/endEdit via bridge_params.h
//   [ ] `action` routing (ApplyPreset-style controller messages)
//   [ ] 30 Hz `frame` push from the feedback param values
//   [ ] keyboard focus / HiDPI / resize behavior in Cubase mac+win

#include <memory>

#include "public.sdk/source/common/pluginview.h"

namespace choc::ui {
class WebView;
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
    void handleHostCall(const std::string& json); // JS -> C++
    void pushState();                             // C++ -> JS (full snapshot)
    void pushFrame();                             // C++ -> JS (~30 Hz visuals)

    PolyController* controller_ = nullptr;
    std::unique_ptr<choc::ui::WebView> webview_;
};

} // namespace poly
