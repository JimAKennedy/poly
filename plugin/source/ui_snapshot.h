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

    // Full state — flag-guarded exchange
    SceneState state{};
    std::atomic<bool> stateReady{false};
};
// endregion:ui-snapshot

} // namespace poly
