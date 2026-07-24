#pragma once

#include <cstddef>
#include <cstdint>

#include "poly/types.h"

namespace poly {

// region:state-version
static constexpr int32_t kCurrentStateVersion = 15;
// endregion:state-version

// --- Envelope serialization helpers ---
//
// Split out from state_io.h (M049 S08, E9) so state_io_read_lane.h /
// state_io_write_lane.h can include it directly and become self-contained.
// Before the split, the lane headers called writeEnvelope/readEnvelope
// without including any header that declared them — they only compiled
// because state_io.h defined the helpers, closed the namespace, and
// included the lane headers mid-file. That inclusion-order dependency
// made the lane headers fragile and unusable in isolation.

template <typename WriteFn>
[[nodiscard]] bool writeEnvelope(WriteFn&& write, const Envelope& env, int32_t version = kCurrentStateVersion) {
    auto target = static_cast<uint8_t>(env.target);
    if (!write(&target, sizeof(target)))
        return false;
    if (!write(&env.periodBars, sizeof(env.periodBars)))
        return false;
    auto shape = static_cast<uint8_t>(env.shape);
    if (!write(&shape, sizeof(shape)))
        return false;
    if (!write(&env.depth, sizeof(env.depth)))
        return false;
    if (!write(&env.phaseOffset, sizeof(env.phaseOffset)))
        return false;
    // v2 added curvature + step list. Mirror readEnvelope's version gate so
    // writeGrooveStateBody(bodyVersion=1) produces bytes readGrooveStateBody(v=1)
    // can align. (E5, M049 S05.)
    if (version >= 2) {
        if (!write(&env.curvature, sizeof(env.curvature)))
            return false;
        if (!write(&env.stepCount, sizeof(env.stepCount)))
            return false;
        for (int s = 0; s < kMaxStepListEntries; ++s) {
            if (!write(&env.stepValues[static_cast<size_t>(s)], sizeof(float)))
                return false;
        }
    }
    return true;
}

template <typename ReadFn> [[nodiscard]] bool readEnvelope(ReadFn&& read, Envelope& env, int32_t version) {
    uint8_t target = 0;
    if (!read(&target, sizeof(target)))
        return false;
    env.target = static_cast<EnvTarget>(target);
    if (!read(&env.periodBars, sizeof(env.periodBars)))
        return false;
    uint8_t shape = 0;
    if (!read(&shape, sizeof(shape)))
        return false;
    env.shape = static_cast<Shape>(shape);
    if (!read(&env.depth, sizeof(env.depth)))
        return false;
    if (!read(&env.phaseOffset, sizeof(env.phaseOffset)))
        return false;
    if (version >= 2) {
        if (!read(&env.curvature, sizeof(env.curvature)))
            return false;
        if (!read(&env.stepCount, sizeof(env.stepCount)))
            return false;
        for (int s = 0; s < kMaxStepListEntries; ++s) {
            if (!read(&env.stepValues[static_cast<size_t>(s)], sizeof(float)))
                return false;
        }
    }
    return true;
}

} // namespace poly
