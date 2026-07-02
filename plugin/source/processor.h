#pragma once

#include <atomic>

#include "public.sdk/source/vst/vstaudioeffect.h"

#include "poly/bridge.h"
#include "poly/engine.h"
#include "poly/macro.h"
#include "poly/midi_capture.h"
#include "poly/scene.h"

namespace poly {

class PolyProcessor : public Steinberg::Vst::AudioEffect {
public:
    PolyProcessor();
    ~PolyProcessor() override = default;

    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IAudioProcessor*>(
            new PolyProcessor()); // ownership-transfer — RT-SAFE-OK: host factory, not audio thread
    }

    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) override;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;
    Steinberg::tresult PLUGIN_API notify(Steinberg::Vst::IMessage* message) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;

private:
    void applyParameter(Steinberg::Vst::ParamID id, double normalized);
    void updateTransportContext(const Steinberg::Vst::ProcessData& data);
    void handleTransportJump(Steinberg::Vst::IEventList* outputEvents);
    void emitMidiOutput(Steinberg::Vst::IEventList* outputEvents, Steinberg::int32 numSamples);
    void outputParameterFeedback(Steinberg::Vst::ProcessData& data, const GrooveState& resolved);
    bool applySceneParameter(Steinberg::Vst::ParamID id, double normalized);
    bool applyLaneParameter(Steinberg::Vst::ParamID id, double normalized, GrooveState& gs);

    Engine engine_;
    SceneState sceneState_{};
    NoteEventBuffer noteBuffer_{};
    TransportContext tc_{};
    PendingNoteOffBuffer pendingNoteOffs_{};
    MidiCaptureBuffer captureBuffer_;
    std::array<NoteEvent, MidiCaptureBuffer::kCapacity> exportEvents_{};
    size_t exportEventCount_ = 0;
    double exportTempo_ = 120.0;
    std::atomic<bool> exportReady_{false};
    bool exportTriggered_ = false;
    bool wasPlaying_ = false;
    int captureLengthBars_ = MidiCaptureBuffer::kDefaultCaptureBars;
    double expectedNextPpq_ = -1.0;
    MacroSmoother macroSmoother_{};
    SceneChainState chainState_{};

    SceneState pendingState_{};
    std::atomic<bool> stateReady_{false};
    NoteMap pendingNoteMap_{};
    std::atomic<bool> noteMapReady_{false};

    struct PendingCellSizes {
        int laneIndex = 0;
        std::array<int, kMaxSteps> sizes{};
    };
    PendingCellSizes pendingCellSizes_{};
    std::atomic<bool> cellSizesReady_{false};

    struct PendingTimelinePattern {
        int laneIndex = 0;
        std::array<bool, kMaxSteps> pattern{};
        int patternLength = 0;
    };
    PendingTimelinePattern pendingTimeline_{};
    std::atomic<bool> timelineReady_{false};

    struct PendingMicroTiming {
        int laneIndex = 0;
        std::array<float, kMaxSteps> timingMs{};
    };
    PendingMicroTiming pendingMicroTiming_{};
    std::atomic<bool> microTimingReady_{false};

    struct PendingEnvelope {
        int laneIndex = 0;
        int envelopeIndex = 0;
        Envelope envelope{};
        bool active = true;
    };
    PendingEnvelope pendingEnvelope_{};
    std::atomic<bool> envelopeReady_{false};

    SceneState stateSnapshot_{};
    std::atomic<bool> snapshotReady_{false};
};

} // namespace poly
