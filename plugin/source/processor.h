// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
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

// M046 S03 P4: lockless triple-buffer exchange for host→RT handshakes.
//
// Replaces the single-buffer `Pending<T> + std::atomic<bool> ready_` pattern
// that silently lost updates when the host thread wrote twice before the RT
// thread drained (P4 TOCTOU + silent-loss). A fresh publish always lands in a
// slot the reader isn't looking at, and the exchange-based commit surfaces any
// displaced-but-unconsumed publish so the writer can bump an explicit drop
// counter — the "no lost updates" property becomes "either applied or
// accounted."
//
// M031 S01: the prior design used two slots and an alternating writer index,
// relying on the reader detaching slots[cur] into a stack local "before the
// writer laps back." That happens-before edge had no synchronization backing
// it: after the reader's exchange(-1) the writer had NO way to know the reader
// was still copying slots[cur], so the writer's second alternation could write
// slots[cur] concurrently with the reader's detach copy (TSan data race on
// the slots[] detach copy, all five hot slots — pinned in M031 S01 T01). Two slots are
// fundamentally insufficient: while the reader holds one slot and the last
// publish occupies another, the writer's next write has nowhere safe to go and
// must lap back onto a slot in use.
//
// The fix is a classic lockless triple buffer. Three slots and three indices —
// a writer-private `writeIdx_`, a reader-private `readIdx_`, and a shared atomic
// `mailbox` — are maintained as a permutation of {0,1,2} by only ever swapping
// the mailbox with exactly one private index at a time (an atomic exchange
// preserves the multiset). Because writeIdx_ and readIdx_ are therefore always
// DISTINCT, slots[writeIdx_] (the only slot the writer writes) and
// slots[readIdx_] (the only slot the reader reads) are never the same address:
// the conflicting access is eliminated outright, not merely reordered. The
// acq_rel exchanges on `mailbox` carry the two happens-before edges the old
// design lacked:
//   - writer's release publishes its payload write; reader's acquire on the
//     matching exchange sees it before apply() reads the slot.
//   - reader's release publishes "done reading the slot I'm handing back";
//     writer's acquire on a later commit sees it before it reuses that slot.
//
// SPSC invariants (single writer = host thread, single reader = RT thread):
//   - `writeSlot()` returns the writer's private slot index; it is stable until
//     the next commit() swaps it out. The writer alone touches writeIdx_.
//   - `commit(idx)` atomically swaps slots[writeIdx_] into the mailbox (fresh
//     bit set) and takes the mailbox's old buffer back as the next writeIdx_;
//     returns true iff the displaced mailbox value was still fresh (an
//     unconsumed publish the caller MUST count as a drop).
//   - `consume(fn)` swaps the reader's buffer into the mailbox and, if the
//     mailbox held a fresh publish, invokes `fn(slots[readIdx_])` on the
//     reader-owned buffer. Returns true iff a payload was applied.
//
// RT-safety: writeSlot/commit/consume perform only lockless int32 atomics; the
// reader applies in-place on its private slot (no stack copy of the payload).
// No allocation, no locks, no exceptions.
template <typename Payload> struct HostToRTSlot {
    Payload slots[3]{};

    static constexpr int32_t kIndexMask = 0x3; // low bits hold the slot index (0..2)
    static constexpr int32_t kFreshBit = 0x4;  // set while the mailbox holds an unconsumed publish

    // Initial permutation: writer owns slot 0, reader owns slot 1, mailbox holds
    // slot 2 with the fresh bit clear (nothing published yet).
    int32_t writeIdx_{0};            // writer-private; only the host thread touches it
    int32_t readIdx_{1};             // reader-private; only the RT thread touches it
    std::atomic<int32_t> mailbox{2}; // shared: (index & kIndexMask) | (fresh ? kFreshBit : 0)

    // Writer: the slot to fill next. Stable until commit() swaps writeIdx_ out.
    // SPSC-safe: only the writer reads/writes writeIdx_.
    int32_t writeSlot() { return writeIdx_; }

    // Writer: publish slots[writeIdx_] and take back the buffer the mailbox held.
    // Returns true if the displaced mailbox value was still fresh (unconsumed) —
    // the caller must bump its drop counter. `writeIdx` is passed for call-site
    // symmetry with the old API and equals writeIdx_ by construction.
    bool commit(int32_t writeIdx) {
        (void)writeIdx;
        const int32_t prev = mailbox.exchange(writeIdx_ | kFreshBit, std::memory_order_acq_rel);
        writeIdx_ = prev & kIndexMask;
        return (prev & kFreshBit) != 0;
    }

    // Reader (RT thread): if the mailbox holds a fresh publish, swap the reader's
    // buffer in and apply the freshly-taken buffer in place. A relaxed peek skips
    // the exchange (and buffer churn) when nothing is fresh — only the reader ever
    // clears the fresh bit, so a peeked-fresh mailbox stays fresh until this
    // exchange. apply() reads slots[readIdx_] directly: the writer can never target
    // readIdx_ (permutation invariant), so the reader owns it for the whole call —
    // no stack copy of the payload needed.
    template <typename Apply> bool consume(Apply&& apply) {
        if ((mailbox.load(std::memory_order_relaxed) & kFreshBit) == 0)
            return false;
        const int32_t prev = mailbox.exchange(readIdx_, std::memory_order_acq_rel);
        readIdx_ = prev & kIndexMask;
        if ((prev & kFreshBit) == 0)
            return false; // defensive: only the reader clears fresh, so unreachable in SPSC
        std::forward<Apply>(apply)(slots[readIdx_]);
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

    // M051 S08: test-only accessors for the arm->capture->complete state machine.
    // captureStateForTesting reads the audio-thread-owned state directly (single
    // threaded in host tests); exportReady/exportEvents let a test prove the
    // frozen window is populated at `complete` and byte-stable after further play.
    int captureStateForTesting() const { return static_cast<int>(captureState_); }
    bool exportReadyForTesting() const { return exportReady_.load(std::memory_order_acquire); }
    size_t exportEventCountForTesting() const { return exportEventCount_; }
    const NoteEvent* exportEventsForTesting() const { return exportEvents_.data(); }

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
    // M051 S08 capture state machine helpers (all run on the audio thread).
    void applyCaptureCommand();
    void updateCaptureMachine();
    void publishCaptureSnapshot();
    // M073: drain emissionBuffer_ into the per-lane UISnapshot emission rings.
    void publishEmissions();
    bool applySceneParameter(Steinberg::Vst::ParamID id, double normalized);
    bool applyLaneParameter(Steinberg::Vst::ParamID id, double normalized, GrooveState& gs);

    Engine engine_;
    SceneState sceneState_{};
    NoteEventBuffer noteBuffer_{};
    // M073: per-block emission classification, drained into uiSnapshot_ rings
    // after each render so the WebUI desk overlay + played timeline light up in
    // the DAW. Pre-allocated (fixed-cap array inside EmissionEventBuffer) — no
    // RT allocation. Passed to renderRange only when publishing to the UI.
    EmissionEventBuffer emissionBuffer_{};
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

    // M051 S08: WebUI arm->capture->complete state machine. Distinct from the
    // native VSTGUI export path (exportTriggered_/exportReady_, kept until M053):
    // reaching Complete freezes an exact bar window into exportEvents_ and sets
    // exportReady_, reusing the RequestMidiExport/MidiExportData reply path
    // (Decision D005) rather than allocating a message inside process() (MEM003).
    enum class CaptureState : int { Idle = 0, Armed = 1, Capturing = 2, Complete = 3 };
    CaptureState captureState_ = CaptureState::Idle; // audio-thread owned
    double captureStartPpq_ = -1.0;                  // latched bar boundary (absolute PPQ)
    double captureTempo_ = 0.0;                      // tempo at latch; change mid-capture cancels
    // Message-thread -> audio-thread command: 0=none, 1=arm, 2=reset. Set in
    // notify() (ArmCapture/ResetCapture), consumed via exchange() in process().
    std::atomic<uint8_t> captureCommand_{0};
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
