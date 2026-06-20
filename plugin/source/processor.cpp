#include "processor.h"
#include "plugids.h"

#include "pluginterfaces/base/ibstream.h"

#include <cstring>

namespace poly {

PolyProcessor::PolyProcessor() {
    setControllerClass(kPolyControllerUID);
}

Steinberg::tresult PLUGIN_API PolyProcessor::initialize(
    Steinberg::FUnknown* context) {
    auto result = AudioEffect::initialize(context);
    if (result != Steinberg::kResultOk)
        return result;

    addAudioOutput(STR16("Stereo Out"),
                   Steinberg::Vst::SpeakerArr::kStereo);
    addEventOutput(STR16("MIDI Out"), 1);

    return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API PolyProcessor::setActive(
    Steinberg::TBool state) {
    return AudioEffect::setActive(state);
}

Steinberg::tresult PLUGIN_API PolyProcessor::process(
    Steinberg::Vst::ProcessData& data) {
    if (data.numOutputs > 0) {
        for (Steinberg::int32 ch = 0; ch < data.outputs[0].numChannels; ++ch) {
            if (data.outputs[0].channelBuffers32[ch]) {
                std::memset(data.outputs[0].channelBuffers32[ch], 0,
                            static_cast<size_t>(data.numSamples) * sizeof(float));
            }
        }
        data.outputs[0].silenceFlags = 0x3;
    }

    return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API PolyProcessor::getState(
    Steinberg::IBStream* state) {
    if (!state) return Steinberg::kInvalidArgument;

    Steinberg::int32 kStateVersion = 1;
    state->write(&kStateVersion, sizeof(kStateVersion), nullptr);

    return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API PolyProcessor::setState(
    Steinberg::IBStream* state) {
    if (!state) return Steinberg::kInvalidArgument;

    Steinberg::int32 version = 0;
    state->read(&version, sizeof(version), nullptr);

    return Steinberg::kResultOk;
}

} // namespace poly
