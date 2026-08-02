#pragma once

#include <atomic>

#include "poly/scene.h"
#include "poly/types.h"

namespace poly {

// region:ui-snapshot
// Per-instance UI snapshot shared between the processor (audio thread writer)
// and the controller/web view (UI thread reader). Each plugin instance gets
// its own UISnapshot — no globals, no multi-instance crosstalk.
//
// Transport fields are individually atomic (relaxed reads are fine at 30fps).
// The full SceneState uses a flag-guarded single-copy exchange: the processor
// writes only when the reader has consumed the previous snapshot.
struct UISnapshot {
    // Transport — written by processor every process() call
    std::atomic<double> ppqNorm{0.0};
    std::atomic<bool> playing{false};
    std::atomic<double> lanePhases[kMaxLanes]{};
    // M051 S02: host time signature, populated from ProcessContext every block.
    // Defaults to 4/4 when the host doesn't publish kTimeSigValid.
    std::atomic<int16_t> timeSigNumerator{4};
    std::atomic<int16_t> timeSigDenominator{4};

    // M051 S08: capture state machine surfaced to the WebUI Cloth timeline so
    // the arm->capture->complete progression is directly observable (the visual
    // is the receipt). All relaxed — single writer (audio thread), 30fps reader.
    //   captureState: 0=idle, 1=armed, 2=capturing, 3=complete.
    std::atomic<int> captureState{0};
    //   captureBars: target window length in bars (mirrors captureLengthBars_ /
    //   kCaptureLength). Default 8 = MidiCaptureBuffer::kDefaultCaptureBars.
    std::atomic<int> captureBars{8};
    //   captureProgressBars: bars elapsed within the window, 0..captureBars.
    //   Drives the Cloth playhead (playhead = captureProgressBars / captureBars).
    std::atomic<double> captureProgressBars{0.0};

    // M073: per-lane emission ring surfaced to the WebUI desk overlay + played
    // timeline. The engine emits a per-block EmissionEventBuffer classifying
    // every step Base/Ghost/Add/Drop with its grid ppq and post-timing-shift
    // onset (shiftedPpqPosition). The audio thread appends each block's
    // emissions into these fixed-cap per-lane rings; the UI thread (30fps)
    // drains them in publish order for getLaneEmissions(li).
    //
    // RT-safety: the writer (audio thread) only stores POD fields into a
    // pre-allocated array and bumps a relaxed atomic head counter — no alloc,
    // lock, throw, or IO. The reader tolerates a slightly torn view (a slot
    // being overwritten as it reads); at 30fps a one-frame-stale emission is
    // imperceptible and the ring self-heals on the next drain. This mirrors the
    // WASM host's per-lane emission ring (webui/wasm-host.js) so both surfaces
    // present the same shape.
    static constexpr int kEmissionRingCap = 64;
    struct EmissionSlot {
        std::atomic<double> ppq{0.0};
        std::atomic<double> shiftedPpq{0.0};
        std::atomic<int> step{0};
        std::atomic<int> kind{0}; // EmissionKind
    };
    // head[lane] is the monotonically-increasing total emission count for the
    // lane; (head-1) mod cap is the newest slot. The reader reads head, then
    // walks back min(head, cap) slots oldest→newest to reconstruct the ring.
    std::atomic<uint64_t> emissionHead[kMaxLanes]{};
    EmissionSlot emissionRing[kMaxLanes][kEmissionRingCap]{};

    // Full state — flag-guarded exchange
    SceneState state{};
    std::atomic<bool> stateReady{false};
};
// endregion:ui-snapshot

} // namespace poly
