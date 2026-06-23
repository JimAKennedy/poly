#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/plugin-bindings/vst3editor.h"

#include "poly/scene.h"

namespace poly {

class PolyController : public Steinberg::Vst::EditControllerEx1, public VSTGUI::VST3EditorDelegate {
public:
    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IEditController*>(new PolyController()); // ownership-transfer
    }

    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) override;
    Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) override;

    VSTGUI::CView* createCustomView(VSTGUI::UTF8StringPtr name, const VSTGUI::UIAttributes& attributes,
                                    const VSTGUI::IUIDescription* description, VSTGUI::VST3Editor* editor) override;

    const SceneState& cachedState() const { return cachedState_; }
    SceneState& mutableCachedState() { return cachedState_; }

private:
    SceneState cachedState_{};
};

} // namespace poly
