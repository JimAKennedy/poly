#pragma once

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

namespace Steinberg {
namespace Vst {
class AudioEffect;
class EditController;
} // namespace Vst
} // namespace Steinberg

namespace poly {
class PolyControllerBase;
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
