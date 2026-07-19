#pragma once

#include <array>
#include <string>
#include <vector>

#include "public.sdk/source/vst/vsteditcontroller.h"

#include "poly/scene.h"
#include "ui_snapshot.h"

namespace poly {

// VSTGUI-free portion of the plugin controller.
//
// Split rationale: `PolyController` (controller.h) additionally inherits
// VSTGUI::VST3EditorDelegate and overrides `createView`/`createCustomView`,
// which pulls VSTGUI into every translation unit that includes it. The
// state-serialization, notify(), and message-send code is UI-independent,
// so we host it here so `poly_host_tests` can link a real controller
// without the VSTGUI dependency.
class PolyControllerBase : public Steinberg::Vst::EditControllerEx1 {
public:
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API notify(Steinberg::Vst::IMessage* message) override;
    Steinberg::tresult PLUGIN_API disconnect(Steinberg::Vst::IConnectionPoint* other) override;

    const SceneState& cachedState() const { return cachedState_; }
    SceneState& mutableCachedState() { return cachedState_; }
    const GrooveState& activeScene() const {
        return (cachedState_.select == SceneSelect::B) ? cachedState_.sceneB : cachedState_.sceneA;
    }
    GrooveState& mutableActiveScene() {
        return (cachedState_.select == SceneSelect::B) ? cachedState_.sceneB : cachedState_.sceneA;
    }
    void sendNoteMap();
    void sendCellSizes(int laneIndex);
    void sendTimelinePattern(int laneIndex);
    void sendMicroTiming(int laneIndex);
    void sendEnvelopeUpdate(int laneIndex, int envelopeIndex);
    void sendAccentMask(int laneIndex);

    bool hasPendingSmf() const { return !pendingSmfData_.empty(); }
    std::vector<uint8_t> consumeSmfData() { return std::move(pendingSmfData_); }

    bool hasDragSmf() const { return !dragSmfCache_.empty(); }
    const std::vector<uint8_t>& dragSmfData() const { return dragSmfCache_; }

    const std::string& laneName(int lane) const { return laneNames_[lane]; }
    void setLaneName(int lane, const std::string& name);
    void resetLaneNames();

    uint32_t stateGeneration() const { return stateGeneration_; }
    // Invariant: the returned pointer is populated by the processor's UISnapshotPtr
    // message on connect() and cleared by disconnect(). Consumers MUST null-check
    // before dereferencing — the pointer is nullptr both before the first connect
    // and while disconnected between reconnects. See P3 (M046 S02).
    UISnapshot* uiSnapshot() const { return uiSnapshot_; }

protected:
    void registerOutputParameters();
    // v1: version + lane names.
    // v2: v1 + writable-param snapshot (pluginval save/restore round-trip). Bumped in M046 S01a.
    static constexpr int kControllerStateVersion = 2;
    SceneState cachedState_{};
    std::array<std::string, kMaxLanes> laneNames_;
    uint32_t stateGeneration_ = 0;
    std::vector<uint8_t> pendingSmfData_;
    std::vector<uint8_t> dragSmfCache_;
    UISnapshot* uiSnapshot_ = nullptr;
};

} // namespace poly
