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
    std::vector<ProbeEvent> events_;
};

} // namespace probe
