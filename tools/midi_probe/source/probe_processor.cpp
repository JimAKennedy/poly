#include "probe_processor.h"

#include <cstdlib>
#include <fstream>

#include "pluginterfaces/vst/ivstevents.h"

#include "probe_ids.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace probe {

ProbeProcessor::ProbeProcessor() = default;

tresult PLUGIN_API ProbeProcessor::initialize(FUnknown* context) {
    tresult result = AudioEffect::initialize(context);
    if (result != kResultOk)
        return result;

    addEventInput(STR16("MIDI In"));

    return kResultOk;
}

tresult PLUGIN_API ProbeProcessor::terminate() {
    return AudioEffect::terminate();
}

tresult PLUGIN_API ProbeProcessor::setActive(TBool state) {
    if (!state) {
        const char* path = std::getenv("POLY_PROBE_OUTPUT");
        if (path && path[0] != '\0')
            writeJsonl(path);
    }
    return kResultOk;
}

tresult PLUGIN_API ProbeProcessor::setupProcessing(ProcessSetup& setup) {
    return AudioEffect::setupProcessing(setup);
}

tresult PLUGIN_API ProbeProcessor::process(ProcessData& data) {
    if (!data.inputEvents)
        return kResultOk;

    int32 eventCount = data.inputEvents->getEventCount();
    for (int32 i = 0; i < eventCount; ++i) {
        Event ev{};
        if (data.inputEvents->getEvent(i, ev) != kResultOk)
            continue;
        if (ev.type == Event::kNoteOnEvent) {
            events_.push_back({ProbeEvent::NoteOn, ev.ppqPosition, ev.noteOn.pitch, ev.noteOn.velocity,
                               ev.noteOn.channel, ev.sampleOffset});
        } else if (ev.type == Event::kNoteOffEvent) {
            events_.push_back({ProbeEvent::NoteOff, ev.ppqPosition, ev.noteOff.pitch, ev.noteOff.velocity,
                               ev.noteOff.channel, ev.sampleOffset});
        }
    }
    return kResultOk;
}

tresult PLUGIN_API ProbeProcessor::getState(IBStream* /*state*/) {
    return kResultOk;
}

tresult PLUGIN_API ProbeProcessor::setState(IBStream* /*state*/) {
    return kResultOk;
}

bool ProbeProcessor::writeJsonl(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open())
        return false;

    for (const auto& e : events_) {
        char buf[256];
        std::snprintf(buf, sizeof(buf), R"({"type":"%s","ppq":%.6f,"pitch":%d,"velocity":%.6f,"channel":%d})",
                      e.type == ProbeEvent::NoteOn ? "noteOn" : "noteOff", e.ppqPosition, static_cast<int>(e.pitch),
                      e.velocity, static_cast<int>(e.channel));
        out << buf << '\n';
    }
    return true;
}

} // namespace probe
