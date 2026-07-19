#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "poly/types.h"

namespace Steinberg {
namespace Vst {
class AudioEffect;
class EditController;
} // namespace Vst
} // namespace Steinberg

namespace poly {
class PolyControllerBase;
struct UISnapshot;
} // namespace poly

namespace poly {
namespace test {

struct MidiEvent {
    enum Type : uint8_t { NoteOn, NoteOff };
    Type type;
    double ppqPosition;
    int16_t pitch;
    float velocity;
    int16_t channel;
    int32_t sampleOffset;
};

class PolyTestHost {
public:
    PolyTestHost();
    ~PolyTestHost();

    PolyTestHost(const PolyTestHost&) = delete;
    PolyTestHost& operator=(const PolyTestHost&) = delete;

    bool setup(double sampleRate = 44100.0, int blockSize = 512);
    void teardown();

    // --- IConnectionPoint lifecycle (M046 S02 P3) ---
    // Wires processor <-> controller via IConnectionPoint::connect on both sides — matches
    // what real DAWs (Cubase, JUCE hosts) do at plugin load. Called automatically by setup()
    // so most tests see the full lifecycle; exposed here for tests that need to reconnect.
    // Returns false if either side lacks IConnectionPoint or the connect calls fail.
    bool connectComponents();
    // Symmetric IConnectionPoint::disconnect on both sides. Called automatically by teardown()
    // before terminate/release. After a clean disconnect, the controller's cached uiSnapshot_
    // pointer MUST be nulled (that's the P3 invariant — see PolyControllerBase::disconnect).
    void disconnectComponents();
    // Returns the controller's cached processor-side UISnapshot pointer.
    // Null before connect() and after disconnect(); non-null while connected once the
    // UISnapshotPtr message has been delivered via notify().
    poly::UISnapshot* controllerUiSnapshot() const;

    void playBars(double bars, double tempo);
    void processBlock(double ppqStart, double tempo, bool playing, bool looping = false, double loopStart = 0.0,
                      double loopEnd = 0.0);
    void stopAndFlush(double ppqPos, double tempo);

    // --- Host-side state IO (real IBStream via Steinberg::MemoryStream) ---
    // saveState() calls processor->getState() end-to-end and returns the serialized bytes.
    // loadState() calls processor->setState(); if a controller is attached, it also invokes
    // controller->setComponentState(bytes) so the controller-side param cache mirrors the
    // restored processor state — matches the host contract pluginval exercises.
    // Returns an empty vector on failure.
    std::vector<uint8_t> saveState();
    bool loadState(const std::vector<uint8_t>& bytes);

    // --- Full-DAW pluginval-mimic state IO ---
    // saveFullPluginState() serializes BOTH processor and controller state in one blob,
    // the way JUCE-based hosts (including pluginval) do via getStateInformation. Layout:
    //   [uint32 procLen][procBytes][uint32 ctrlLen][ctrlBytes]
    // loadFullPluginState() unpacks the blob and calls, in order:
    //   1. processor->setState(procBytes)
    //   2. controller->setComponentState(procBytes) — DAW sync contract
    //   3. controller->setState(ctrlBytes)          — controller-owned state has the last word
    // This is the loop pluginval exercises when it reports "Param not restored on
    // setStateInformation" — use these helpers (not saveState/loadState) for tests that
    // reproduce controller-side param round-trip.
    std::vector<uint8_t> saveFullPluginState();
    bool loadFullPluginState(const std::vector<uint8_t>& bytes);

    // Dispatches a NoteMapUpdate message through processor->notify(). The edit is parked in
    // pendingNoteMap_ and drained into sceneState_ on the next process() block. Used to model
    // a controller-driven scene-state edit in save-after-stop regression tests.
    void injectNoteMap(const std::array<int16_t, 128>& map);

    // --- Controller-side parameter access (pluginval-mimic path) ---
    // setParamOnController: pushes a normalized value into the controller's param cache
    // via IEditController::setParamNormalized. Does NOT touch the processor. Mirrors
    // pluginval's setParamNormalized calls between save/restore.
    bool setParamOnController(uint32_t paramId, double normalizedValue);
    // getParamOnController: reads back the controller-cached normalized value. Returns
    // -1.0 if no controller is attached.
    double getParamOnController(uint32_t paramId) const;
    // injectParamChangeThroughProcess: queues a normalized value onto a fresh
    // IParameterChanges/IParamValueQueue that will be handed to the next processBlock()
    // call. Models the standard VST3 automation path — this IS the working path.
    void injectParamChangeThroughProcess(uint32_t paramId, double normalizedValue);

    // --- M046 S03 P4: UI-to-audio handshake burst injectors ---
    // Each helper dispatches ONE notify() call to the processor with a distinct payload.
    // Tests call these back-to-back without processBlock() between to reproduce the P4
    // TOCTOU race: on current HEAD the second call silently overwrites the first, and
    // the drop counter stays at zero (proof of silent loss). After S03 T02's 2-slot
    // exchange, the drop counter increments to 1 on the second call.
    void injectCellSizes(int laneIndex, const std::array<int, poly::kMaxSteps>& sizes);
    void injectTimelinePattern(int laneIndex, const std::array<bool, poly::kMaxSteps>& pattern, int patternLength);
    void injectMicroTiming(int laneIndex, const std::array<float, poly::kMaxSteps>& timingMs);
    void injectEnvelope(int laneIndex, int envelopeIndex, bool active, const poly::Envelope& envelope);
    void injectAccentMask(int laneIndex, const std::array<float, poly::kMaxSteps>& steps);

    // Read-only handshake drop counter snapshot. Reflects the number of writer-side
    // overwrites of unconsumed publishes for each handshake site. Zero on HEAD; expected
    // to increment after S03 T02 lands the 2-slot exchange.
    struct HandshakeDropSnapshot {
        uint64_t state = 0;
        uint64_t noteMap = 0;
        uint64_t cellSizes = 0;
        uint64_t timeline = 0;
        uint64_t microTiming = 0;
        uint64_t envelope = 0;
        uint64_t accentMask = 0;
    };
    HandshakeDropSnapshot handshakeDrops() const;

    const std::vector<MidiEvent>& events() const { return events_; }
    void clearEvents() { events_.clear(); }

    std::vector<MidiEvent> noteOnEvents() const;
    std::vector<MidiEvent> noteOffEvents() const;

    double sampleRate() const { return sampleRate_; }
    int blockSize() const { return blockSize_; }
    double ppqPerBlock(double tempo) const;

private:
    Steinberg::Vst::AudioEffect* processor_ = nullptr;
    poly::PolyControllerBase* controller_ = nullptr;
    double sampleRate_ = 44100.0;
    int blockSize_ = 512;
    bool active_ = false;
    std::vector<MidiEvent> events_;
    std::vector<float> leftBuf_;
    std::vector<float> rightBuf_;
    // Pending param changes to be flushed on the next processBlock() call via inputParameterChanges.
    std::vector<std::pair<uint32_t, double>> pendingParamChanges_;
};

} // namespace test
} // namespace poly
