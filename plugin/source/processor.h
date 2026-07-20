#pragma once

#include <atomic>
#include <cstdint>
#include <utility>

#include "pluginterfaces/vst/ivstevents.h"
#include "public.sdk/source/vst/vstaudioeffect.h"

#include "poly/bridge.h"
#include "poly/engine.h"
#include "poly/macro.h"
#include "poly/midi_capture.h"
#include "poly/scene.h"
#include "ui_snapshot.h"

namespace poly {

// M046 S03 P4: SPSC 2-slot double-buffer exchange for host→RT handshakes.
//
// Replaces the single-buffer `Pending<T> + std::atomic<bool> ready_` pattern
// that silently lost updates when the host thread wrote twice before the RT
// thread drained (P4 TOCTOU + silent-loss). The two slots let a fresh publish
// always land in a slot the reader isn't looking at, and the exchange-based
// commit surfaces any displaced-but-unconsumed publish so the writer can bump
// an explicit drop counter — the "no lost updates" property becomes "either
// applied or accounted."
//
// SPSC invariants (single writer = host thread, single reader = RT thread):
//   - `writeSlot()` returns the slot index that is *not* currently published, so
//     the writer never stomps a slot the reader may be reading.
//   - `commit(idx)` atomically swaps the published-index; if the previous
//     published-index was ≥ 0 the writer displaced an unconsumed publish and
//     the caller MUST bump its drop counter.
//   - `consume(fn)` atomically detaches the published slot (index → -1) and
//     invokes `fn(payload)`. Returns true iff a payload was applied.
//
// RT-safety: writeSlot/commit/consume perform only lockless int32 atomics and
// a POD copy inside the caller. No allocation, no locks, no exceptions.
template <typename Payload> struct HostToRTSlot {
    Payload slots[2]{};
    std::atomic<int32_t> published{-1}; // -1 = empty, 0 or 1 = slot holding an unconsumed publish

    // Writer: pick the slot that is safe to write into (the one not currently published).
    int32_t writeSlot() const { return (published.load(std::memory_order_acquire) == 0) ? 1 : 0; }

    // Writer: atomically publish the newly-filled slot. Returns true if a previous
    // unconsumed publish was displaced (writer should bump its drop counter).
    bool commit(int32_t writeIdx) { return published.exchange(writeIdx, std::memory_order_release) >= 0; }

    // Reader (RT thread): apply pending payload via callback if one is published.
    template <typename Apply> bool consume(Apply&& apply) {
        int32_t cur = published.exchange(-1, std::memory_order_acquire);
        if (cur < 0)
            return false;
        std::forward<Apply>(apply)(slots[cur]);
        return true;
    }
};

class PolyProcessor : public Steinberg::Vst::AudioEffect {
public:
    PolyProcessor();
    ~PolyProcessor() override = default;

    // M046 S03 P4: per-handshake drop counter. Incremented by the writer whenever the
    // reader hasn't consumed the previous publish yet. Invariant (after S03 T02):
    // notify-issued == applied + drops for every field. If drops stay zero on HEAD
    // it proves silent loss (writer stomped pending without accounting).
    struct HandshakeDropCounters {
        std::atomic<uint64_t> state{0};
        std::atomic<uint64_t> noteMap{0};
        std::atomic<uint64_t> cellSizes{0};
        std::atomic<uint64_t> timeline{0};
        std::atomic<uint64_t> microTiming{0};
        std::atomic<uint64_t> envelope{0};
        std::atomic<uint64_t> accentMask{0};
    };
    const HandshakeDropCounters& handshakeDrops() const { return handshakeDrops_; }

    // M046 S03 T03: per-handshake applied counter. Incremented by the reader (RT
    // thread) on every successful consume(). Paired with handshakeDrops_ to prove
    // the "no silent loss" invariant under threaded stress: issued == applied + drops.
    struct HandshakeAppliedCounters {
        std::atomic<uint64_t> state{0};
        std::atomic<uint64_t> noteMap{0};
        std::atomic<uint64_t> cellSizes{0};
        std::atomic<uint64_t> timeline{0};
        std::atomic<uint64_t> microTiming{0};
        std::atomic<uint64_t> envelope{0};
        std::atomic<uint64_t> accentMask{0};
    };
    const HandshakeAppliedCounters& handshakeApplied() const { return handshakeApplied_; }

    // M046 S04 P6: note-off drop counter. Incremented by emitMidiOutput when
    // pendingNoteOffs_.push() returns false (buffer at kCapacity) so the drop is
    // accounted rather than silently swallowed. Fix (T03) also emits an immediate
    // best-effort off in the same block so the DAW hears a short note rather than
    // a stuck one.
    uint64_t noteOffDrops() const { return noteOffDrops_.load(std::memory_order_relaxed); }

    // M046 S04 T01: test-only injector so host tests can prefill pendingNoteOffs_
    // (P6 overflow reproduction) and poke synthetic stragglers (P5 flushDue lower-bound
    // reproduction) without threading a synthetic tempo ramp through processBlock.
    // Returns false if the buffer is already at kCapacity — mirrors PendingNoteOffBuffer::push.
    bool pushPendingNoteOffForTesting(const PendingNoteOff& off) { return pendingNoteOffs_.push(off); }

    // M046 S06 T01: test-only accessor + injector for the MIDI capture buffer so host
    // tests can reproduce the P9 loop-wrap defect. On HEAD, when the transport wraps
    // from loopEnd back to loopStart, handleTransportJump() calls captureBuffer_.clear()
    // because expectedNextPpq_ − ppqStart trips the jump detector (fixed threshold of
    // 0.001 PPQ). Tests seed the buffer with synthetic notes then step through a wrap
    // to observe the drop.
    void pushCapturedNoteForTesting(const NoteEvent& note) { captureBuffer_.push(note); }
    size_t captureBufferCount() const { return captureBuffer_.count(); }

    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IAudioProcessor*>(
            new PolyProcessor()); // ownership-transfer — RT-SAFE-OK: host factory, not audio thread
    }

    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) override;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;
    Steinberg::tresult PLUGIN_API connect(Steinberg::Vst::IConnectionPoint* other) override;
    Steinberg::tresult PLUGIN_API notify(Steinberg::Vst::IMessage* message) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;

private:
    void applyParameter(Steinberg::Vst::ParamID id, double normalized);
    void updateTransportContext(const Steinberg::Vst::ProcessData& data);
    void handleTransportJump(Steinberg::Vst::IEventList* outputEvents);
    void emitMidiOutput(Steinberg::Vst::IEventList* outputEvents, Steinberg::int32 numSamples);
    void outputParameterFeedback(Steinberg::Vst::ProcessData& data, const GrooveState& resolved);
    void bounceExportTriggerZero(Steinberg::Vst::IParameterChanges* outParams);
    void sendSnapshotPointer();
    bool applySceneParameter(Steinberg::Vst::ParamID id, double normalized);
    bool applyLaneParameter(Steinberg::Vst::ParamID id, double normalized, GrooveState& gs);

    Engine engine_;
    SceneState sceneState_{};
    NoteEventBuffer noteBuffer_{};
    TransportContext tc_{};
    PendingNoteOffBuffer pendingNoteOffs_{};
    MidiCaptureBuffer captureBuffer_;
    std::array<NoteEvent, MidiCaptureBuffer::kCapacity> exportEvents_{};
    size_t exportEventCount_ = 0;
    double exportTempo_ = 120.0;
    std::atomic<bool> exportReady_{false};
    bool exportTriggered_ = false;
    bool wasPlaying_ = false;
    int captureLengthBars_ = MidiCaptureBuffer::kDefaultCaptureBars;
    double expectedNextPpq_ = -1.0;
    MacroSmoother macroSmoother_{};
    SceneChainState chainState_{};

    // M046 S03 P4: all seven host→RT handshakes migrated to HostToRTSlot 2-slot exchange.
    HostToRTSlot<SceneState> stateSlot_{};
    HostToRTSlot<NoteMap> noteMapSlot_{};

    struct PendingCellSizes {
        int laneIndex = 0;
        std::array<int, kMaxSteps> sizes{};
    };
    HostToRTSlot<PendingCellSizes> cellSizesSlot_{};

    struct PendingTimelinePattern {
        int laneIndex = 0;
        std::array<bool, kMaxSteps> pattern{};
        int patternLength = 0;
    };
    HostToRTSlot<PendingTimelinePattern> timelineSlot_{};

    struct PendingMicroTiming {
        int laneIndex = 0;
        std::array<float, kMaxSteps> timingMs{};
    };
    HostToRTSlot<PendingMicroTiming> microTimingSlot_{};

    struct PendingEnvelope {
        int laneIndex = 0;
        int envelopeIndex = 0;
        Envelope envelope{};
        bool active = true;
    };
    HostToRTSlot<PendingEnvelope> envelopeSlot_{};

    struct PendingAccentMask {
        int laneIndex = 0;
        std::array<float, kMaxSteps> steps{};
    };
    HostToRTSlot<PendingAccentMask> accentMaskSlot_{};

    SceneState stateSnapshot_{};
    std::atomic<bool> snapshotReady_{false};

    UISnapshot uiSnapshot_{};

    HandshakeDropCounters handshakeDrops_{};
    HandshakeAppliedCounters handshakeApplied_{};

    // M046 S04 P6: incremented on pendingNoteOffs_ overflow. Zero until T03 lands the fix.
    std::atomic<uint64_t> noteOffDrops_{0};

    // M046 S07 P12: pre-allocated scratch for emitMidiOutput. Events are staged
    // here, sorted by sampleOffset, then addEvent()'d in order — avoids the
    // non-monotonic sequence that JUCE-based hosts / older Bitwig reject. Sized
    // for the worst case: kMaxEventsPerBlock due-offs + kMaxEventsPerBlock note-ons
    // + kMaxEventsPerBlock immediate offs on push overflow.
    Steinberg::Vst::Event emitScratch_[3 * kMaxEventsPerBlock]{};
};

} // namespace poly
