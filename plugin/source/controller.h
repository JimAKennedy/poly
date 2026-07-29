#pragma once

#include "controller_base.h"

namespace poly {

// Plugin controller. All state, parameter, and messaging logic lives on the
// VSTGUI-free base (PolyControllerBase); this subclass only adds the editor
// factory (createView). On the choc-webview platforms (Apple/Windows) it
// returns the WebUIView editor; on other platforms it returns nullptr (no
// editor) until choc's WebKitGTK backend lands.
class PolyController : public PolyControllerBase {
public:
    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IEditController*>(new PolyController()); // ownership-transfer
    }

    Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) override;
};

} // namespace poly
