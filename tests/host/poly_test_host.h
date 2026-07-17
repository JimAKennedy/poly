#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace Steinberg {
namespace Vst {
class AudioEffect;
}
} // namespace Steinberg

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
    // loadState() calls processor->setState(); the state is applied on the next process() block.
    // Returns an empty vector on failure.
    std::vector<uint8_t> saveState();
    bool loadState(const std::vector<uint8_t>& bytes);

    // Dispatches a NoteMapUpdate message through processor->notify(). The edit is parked in
    // pendingNoteMap_ and drained into sceneState_ on the next process() block. Used to model
    // a controller-driven scene-state edit in save-after-stop regression tests.
    void injectNoteMap(const std::array<int16_t, 128>& map);

    const std::vector<MidiEvent>& events() const { return events_; }
    void clearEvents() { events_.clear(); }

    std::vector<MidiEvent> noteOnEvents() const;
    std::vector<MidiEvent> noteOffEvents() const;

    double sampleRate() const { return sampleRate_; }
    int blockSize() const { return blockSize_; }
    double ppqPerBlock(double tempo) const;

private:
    Steinberg::Vst::AudioEffect* processor_ = nullptr;
    double sampleRate_ = 44100.0;
    int blockSize_ = 512;
    bool active_ = false;
    std::vector<MidiEvent> events_;
    std::vector<float> leftBuf_;
    std::vector<float> rightBuf_;
};

} // namespace test
} // namespace poly
