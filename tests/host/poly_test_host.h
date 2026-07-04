#pragma once

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
