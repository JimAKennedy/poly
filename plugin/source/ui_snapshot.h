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

    // Full state — flag-guarded exchange
    SceneState state{};
    std::atomic<bool> stateReady{false};
};
// endregion:ui-snapshot

} // namespace poly
