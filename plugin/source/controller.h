#pragma once

#include "vstgui/plugin-bindings/vst3editor.h"

#include "controller_base.h"

namespace poly {

// VSTGUI editor delegate wrapper around PolyControllerBase.
// Adds the VST3Editor factory and the custom-view registration; all
// state, parameter, and messaging logic lives on the VSTGUI-free base.
class PolyController : public PolyControllerBase, public VSTGUI::VST3EditorDelegate {
public:
    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IEditController*>(new PolyController()); // ownership-transfer
    }

    Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) override;

    VSTGUI::CView* createCustomView(VSTGUI::UTF8StringPtr name, const VSTGUI::UIAttributes& attributes,
                                    const VSTGUI::IUIDescription* description, VSTGUI::VST3Editor* editor) override;
};

} // namespace poly
