// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "public.sdk/source/vst/vstaudioeffect.h"

namespace probe {

// M004 S05: where an instance writes its capture. Slot 0 -- the only slot in
// a single-probe project -- writes `base` itself, so every existing fixture and
// workflow step is unchanged. Slot n > 0 inserts "-<n+1>" before the extension
// (probe.jsonl -> probe-2.jsonl), so two probes in one Cubase no longer
// overwrite each other's file. An empty base stays empty: nothing is written.
std::string outputPathForSlot(const std::string& base, int slot);

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
    // Called from the flush-during-playback path, the transport-stop edge in
    // process(), and setActive(false). Records the outcome in the diagnostic
    // sidecar so the next run is diagnosable without shelling in.
    void flushToOutputPath();

    // Always write a probe-status.txt next to POLY_PROBE_OUTPUT recording what
    // the probe actually saw (env var, process-call count, event count, whether
    // the stop edge fired, last flush result). This turns a silent no-flush into
    // a definitive diagnosis: the archive step collects the whole artifact dir, so
    // this lands in cubase-nightly-artifacts even when probe.jsonl does not.
    void writeStatusSidecar() const;

    // POLY_PROBE_OUTPUT adjusted for this instance's slot (outputPathForSlot),
    // or empty when the env var is unset.
    std::string outputPath() const;

    std::vector<ProbeEvent> events_;
    // The lowest slot no live instance holds, claimed in initialize() and
    // released in terminate(). Lowest-free rather than a running count so a
    // host that creates and destroys a probe while scanning or loading cannot
    // push the project's first probe off slot 0 and its file name.
    int slot_ = -1;
    // Tracks the transport play state across process() blocks so we can flush on
    // the playing->stopped edge. This runner hard-kills Cubase (the Hub blocks a
    // clean exit), so setActive(false) never fires — flushing on transport-stop
    // is the only trigger that lands probe.jsonl on disk before the kill.
    bool wasPlaying_ = false;
    // Count of process() calls and the event count at the last flush. The primary
    // durable trigger is flush-during-playback: whenever events_ has grown since
    // the last flush, re-write probe.jsonl from inside process() so the file is on
    // disk BEFORE the transport stops and the kill can beat any stop-edge flush.
    uint64_t processCalls_ = 0;
    size_t lastFlushedEventCount_ = 0;
    // Diagnostic state for the sidecar: did the stop edge fire, was the env var
    // seen, and did the last flush succeed. Mutable so writeJsonl()/flush can be
    // const-friendly while still recording outcomes.
    bool stopEdgeFired_ = false;
    bool envSeen_ = false;
    bool lastFlushOk_ = false;
    uint64_t flushCount_ = 0;
};

} // namespace probe
