#pragma once

#include <atomic>

#include "poly/types.h"

namespace poly {

// region:transport-frame-exchange
// Lock-free transport frame shared between the processor (audio thread writer)
// and the web UI view (UI thread reader). Bypasses the host's output parameter
// relay which some hosts only activate when the editor has VSTGUI bindings.
//
// Each field is independently atomic — the reader may see a slightly stale mix
// across fields (e.g., ppqNorm from one process call, a lane phase from the
// previous), but at 30fps visualization this is imperceptible.
struct TransportFrameExchange {
    std::atomic<double> ppqNorm{0.0};
    std::atomic<bool> playing{false};
    std::atomic<double> lanePhases[kMaxLanes]{};
};
// endregion:transport-frame-exchange

inline TransportFrameExchange& sharedTransportFrame() {
    static TransportFrameExchange instance;
    return instance;
}

} // namespace poly
