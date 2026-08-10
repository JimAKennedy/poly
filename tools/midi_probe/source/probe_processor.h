#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "public.sdk/source/vst/vstaudioeffect.h"

namespace probe {

struct ProbeEvent {
    enum Type : uint8_t { NoteOn, NoteOff };
    Type type;
    double ppqPosition;
    int16_t pitch;
    float velocity;
    int16_t channel;
    int32_t sampleOffset;
};

class ProbeProcessor : public Steinberg::Vst::AudioEffect {
public:
    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IAudioProcessor*>(new ProbeProcessor()); // ownership-transfer
    }

    ProbeProcessor();

    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API terminate() override;
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) override;
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& setup) override;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;

    const std::vector<ProbeEvent>& events() const { return events_; }
    void clearEvents() { events_.clear(); }
    bool writeJsonl(const std::string& path) const;

private:
    // Flush events_ to POLY_PROBE_OUTPUT if that env var is set. No-op otherwise.
    // Called from both the transport-stop edge in process() and setActive(false).
    void flushToOutputPath();

    std::vector<ProbeEvent> events_;
    // Tracks the transport play state across process() blocks so we can flush on
    // the playing->stopped edge. This runner hard-kills Cubase (the Hub blocks a
    // clean exit), so setActive(false) never fires — flushing on transport-stop
    // is the only trigger that lands probe.jsonl on disk before the kill.
    bool wasPlaying_ = false;
};

} // namespace probe
