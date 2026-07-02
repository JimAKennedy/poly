#pragma once

#include <array>
#include <string>
#include <vector>

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
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API notify(Steinberg::Vst::IMessage* message) override;
    Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) override;

    VSTGUI::CView* createCustomView(VSTGUI::UTF8StringPtr name, const VSTGUI::UIAttributes& attributes,
                                    const VSTGUI::IUIDescription* description, VSTGUI::VST3Editor* editor) override;

    const SceneState& cachedState() const { return cachedState_; }
    SceneState& mutableCachedState() { return cachedState_; }
    void sendNoteMap();
    void sendCellSizes(int laneIndex);

    bool hasPendingSmf() const { return !pendingSmfData_.empty(); }
    std::vector<uint8_t> consumeSmfData() { return std::move(pendingSmfData_); }

    bool hasDragSmf() const { return !dragSmfCache_.empty(); }
    const std::vector<uint8_t>& dragSmfData() const { return dragSmfCache_; }

    const std::string& laneName(int lane) const { return laneNames_[lane]; }
    void setLaneName(int lane, const std::string& name);
    void resetLaneNames();

private:
    void registerOutputParameters();
    static constexpr int kControllerStateVersion = 1;
    SceneState cachedState_{};
    std::array<std::string, kMaxLanes> laneNames_;
    std::vector<uint8_t> pendingSmfData_;
    std::vector<uint8_t> dragSmfCache_;
};

} // namespace poly
