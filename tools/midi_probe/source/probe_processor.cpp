#include "probe_processor.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"

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
    // Belt-and-suspenders flush for the graceful-deactivation path. On this
    // runner setActive(false) never fires (Cubase is hard-killed), so the real
    // flush trigger is the transport-stop edge in process() — but keep this so a
    // clean host still writes the file.
    if (!state)
        flushToOutputPath();
    return kResultOk;
}

tresult PLUGIN_API ProbeProcessor::setupProcessing(ProcessSetup& setup) {
    return AudioEffect::setupProcessing(setup);
}

tresult PLUGIN_API ProbeProcessor::process(ProcessData& data) {
    ++processCalls_;

    if (data.inputEvents) {
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
    }

    // Primary durable trigger: flush-during-playback. Whenever events_ has grown
    // since the last flush, re-write probe.jsonl from inside process() so the
    // complete-so-far stream is on disk BEFORE the transport stops. The runner
    // hard-kills Cubase (the Hub blocks a clean exit), so a flush that waits for
    // the transport-stop edge or setActive(false) can be beaten by the kill and
    // silently leave no file — this run's exact failure mode. By writing while
    // the transport still plays, the last complete write survives the kill. The
    // file is fully rewritten each time (writeJsonl truncates), so the last write
    // is always the full note stream, and the compare harness reads a complete
    // file regardless of when the kill lands. This tool is a diagnostic harness
    // (it already allocates in process() via push_back), so a throttled file
    // write is consistent with its posture — not shipping-plugin RT code.
    if (events_.size() > lastFlushedEventCount_)
        flushToOutputPath();

    // Flush on the playing->stopped transport edge too, so a clean host that DOES
    // call process() after stopping writes the final, complete stream. Redundant
    // with flush-during-playback on the hard-kill path but harmless (idempotent
    // rewrite), and it records stopEdgeFired_ for the diagnostic sidecar.
    if (data.processContext) {
        const bool playing = (data.processContext->state & ProcessContext::kPlaying) != 0;
        if (wasPlaying_ && !playing) {
            stopEdgeFired_ = true;
            flushToOutputPath();
        }
        wasPlaying_ = playing;
    }
    return kResultOk;
}

tresult PLUGIN_API ProbeProcessor::getState(IBStream* /*state*/) {
    return kResultOk;
}

tresult PLUGIN_API ProbeProcessor::setState(IBStream* /*state*/) {
    return kResultOk;
}

void ProbeProcessor::flushToOutputPath() {
    const char* path = std::getenv("POLY_PROBE_OUTPUT");
    envSeen_ = (path != nullptr && path[0] != '\0');
    if (envSeen_) {
        lastFlushOk_ = writeJsonl(path);
        lastFlushedEventCount_ = events_.size();
        ++flushCount_;
    }
    // Always refresh the diagnostic sidecar so the next run is diagnosable even
    // when the JSONL write is a no-op (env var absent) or fails to open.
    writeStatusSidecar();
}

void ProbeProcessor::writeStatusSidecar() const {
    // Derive the sidecar path from POLY_PROBE_OUTPUT (probe.jsonl ->
    // probe-status.txt in the same dir). If the env var is absent we can't know
    // where the artifact dir is, so there is nowhere durable to write — skip.
    const char* path = std::getenv("POLY_PROBE_OUTPUT");
    std::string statusPath;
    if (path != nullptr && path[0] != '\0') {
        std::string p(path);
        const auto dot = p.find_last_of('.');
        // Strip the extension if present, then append the sidecar suffix.
        statusPath = (dot == std::string::npos ? p : p.substr(0, dot)) + "-status.txt";
    } else {
        return;
    }

    std::ofstream out(statusPath);
    if (!out.is_open())
        return;

    out << "env_seen=" << (envSeen_ ? "yes" : "no") << '\n'
        << "env_path=" << (path ? path : "") << '\n'
        << "process_calls=" << processCalls_ << '\n'
        << "event_count=" << events_.size() << '\n'
        << "flush_count=" << flushCount_ << '\n'
        << "last_flush_ok=" << (lastFlushOk_ ? "yes" : "no") << '\n'
        << "stop_edge_fired=" << (stopEdgeFired_ ? "yes" : "no") << '\n';
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
