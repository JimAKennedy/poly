#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "poly/engine.h"

namespace poly {

class PolyProcessor : public Steinberg::Vst::AudioEffect {
public:
    PolyProcessor();
    ~PolyProcessor() override = default;

    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IAudioProcessor*>(
            new PolyProcessor()); // RT-SAFE-OK: host factory, not audio thread
    }

    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) override;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;

private:
    Engine engine_;
    GrooveState grooveState_{};
    NoteEventBuffer noteBuffer_{};
};

} // namespace poly
