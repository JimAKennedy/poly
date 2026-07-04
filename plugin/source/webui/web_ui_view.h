#pragma once

// Web UI editor — hosts the webui/ bundle in a choc::ui::WebView and
// bridges it to the controller per webui/bridge-schema.md. Built with
// -DPOLY_WEB_UI=ON; the default build keeps the VSTGUI editor.

#include <memory>
#include <optional>
#include <string>

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
    void pushState();
    void pushFrame();
    void startFrameTimer();
    void stopFrameTimer();
    void resizeWebviewToRect(const Steinberg::ViewRect& r);

    PolyController* controller_ = nullptr;
    std::unique_ptr<choc::ui::WebView> webview_;
    std::optional<choc::messageloop::Timer> frameTimer_;
    uint32_t lastStateGen_ = 0;
    int editCooldown_ = 0;
    std::string lastPushedJson_;
};

} // namespace poly
