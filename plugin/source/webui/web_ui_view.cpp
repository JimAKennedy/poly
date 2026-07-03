// M028 Phase B scaffold — see web_ui_view.h. Compiled only with
// -DPOLY_WEB_UI=ON. The TODOs here are the spike work items; everything
// that can be decided without a host machine (message routing shape,
// asset serving, bridge schema) is already fixed by webui/bridge-schema.md.

#include "web_ui_view.h"

#include <string>

#include "../controller.h"
#include "bridge_params.h"
#include "choc/gui/choc_WebView.h"
#include "poly_webui_assets.h" // generated: jk_embed_assets(webui/*)

namespace poly {

WebUIView::WebUIView(PolyController* controller) : CPluginView(nullptr), controller_(controller) {
    Steinberg::ViewRect rect(0, 0, 1160, 760);
    setRect(rect);
}

WebUIView::~WebUIView() = default;

Steinberg::tresult PLUGIN_API WebUIView::isPlatformTypeSupported(Steinberg::FIDString type) {
#if defined(__APPLE__)
    if (Steinberg::FIDStringsEqual(type, Steinberg::kPlatformTypeNSView))
        return Steinberg::kResultTrue;
#elif defined(_WIN32)
    if (Steinberg::FIDStringsEqual(type, Steinberg::kPlatformTypeHWND))
        return Steinberg::kResultTrue;
#endif
    return Steinberg::kResultFalse;
}

Steinberg::tresult PLUGIN_API WebUIView::attached(void* parent, Steinberg::FIDString type) {
    choc::ui::WebView::Options options;
    options.enableDebugMode = false;
    // Serve the embedded bundle: index.html inlines nothing, so register a
    // fetchResource handler that returns the byte arrays from
    // poly_webui_assets.h keyed by path ("/index.html", "/ui.js", ...).
    options.fetchResource = [](const std::string& path) -> std::optional<choc::ui::WebView::Options::Resource> {
        const auto* asset = webui_assets::lookup(path == "/" ? "/index.html" : path);
        if (!asset)
            return std::nullopt;
        choc::ui::WebView::Options::Resource res;
        res.data.assign(asset->data, asset->data + asset->size);
        res.mimeType = std::string(asset->mime);
        return res;
    };
    webview_ = std::make_unique<choc::ui::WebView>(options);

    // Define the embed marker before any script runs, then load the page.
    webview_->addInitScript("window.__POLY_EMBEDDED__ = true;");
    webview_->bind("polyHostCall", [this](const choc::value::ValueView& args) -> choc::value::Value {
        if (args.isArray() && args.size() > 0)
            handleHostCall(std::string(args[0].getString()));
        return {};
    });
    webview_->navigate("https://poly.assets/index.html");

    // TODO(spike): reparent webview_->getViewHandle() into `parent`
    //   macOS: [(NSView*)parent addSubview:(NSView*)handle]; resize to bounds.
    //   Windows: SetParent((HWND)handle, (HWND)parent); MoveWindow(...).
    (void)parent;
    (void)type;

    // TODO(spike): start the ~30 Hz frame timer (choc::messageloop::Timer)
    // calling pushFrame(); stop it in removed().

    return CPluginView::attached(parent, type);
}

Steinberg::tresult PLUGIN_API WebUIView::removed() {
    webview_.reset();
    return CPluginView::removed();
}

Steinberg::tresult PLUGIN_API WebUIView::onSize(Steinberg::ViewRect* newSize) {
    // TODO(spike): resize the native webview child to newSize.
    return CPluginView::onSize(newSize);
}

void WebUIView::handleHostCall(const std::string& json) {
    // Messages per webui/bridge-schema.md. Parsing TODO(spike): use
    // choc::json::parse (header-only, already vendored via choc).
    //   {type:'ready'}                        -> pushState()
    //   {type:'edit', paramId, value, gesture} ->
    //        auto id = webui::resolveParamId(paramId);
    //        gesture begin  -> controller_->beginEdit(id)
    //        gesture perform-> controller_->setParamNormalized(id, value) +
    //                          controller_->performEdit(id, value)
    //        gesture end    -> controller_->endEdit(id)
    //   {type:'action', name, payload}        -> route via controller
    //        messages (ApplyPreset pattern); each action is followed by
    //        pushState() so the UI re-renders from truth.
    (void)json;
}

void WebUIView::pushState() {
    // TODO(spike): serialize the selected scene's GrooveState (via the
    // controller's cached state for now; engine truth once the sceneState_
    // access races are fixed) into the State JSON shape from
    // webui/host-iface.js, then:
    //   webview_->evaluateJavascript("window.polyHostPush(" + json + ")");
}

void WebUIView::pushFrame() {
    // TODO(spike): read the same values the read-only feedback params carry
    // (lane phase, current step, transport ppq) and push a `frame` message.
}

} // namespace poly
