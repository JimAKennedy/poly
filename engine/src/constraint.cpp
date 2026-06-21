#include "poly/constraint.h"

#include <algorithm>
#include <cmath>

namespace poly {

GrooveState resolveConstraints(const GrooveState& original, const GrooveState& macroResolved) {
    GrooveState out = macroResolved;

    for (int i = 0; i < out.activeLaneCount; ++i) {
        auto& lane = out.lanes[static_cast<size_t>(i)];
        const auto& orig = original.lanes[static_cast<size_t>(i)];
        const auto& cc = lane.constraints;
        int maxSteps = lane.cycle.steps;
        if (maxSteps <= 0)
            continue;

        if (cc.densityMin > 0)
            lane.hitCount = std::max(lane.hitCount, cc.densityMin);
        if (cc.densityMax < maxSteps)
            lane.hitCount = std::min(lane.hitCount, cc.densityMax);
        lane.hitCount = std::clamp(lane.hitCount, 0, maxSteps);

        if (cc.backbeatProtect) {
            lane.emphasisProb = orig.emphasisProb;
            lane.baseVelocity = orig.baseVelocity;
        }
    }

    if (out.globalDensityCeiling > 0) {
        int totalHits = 0;
        for (int i = 0; i < out.activeLaneCount; ++i)
            totalHits += out.lanes[static_cast<size_t>(i)].hitCount;

        if (totalHits > out.globalDensityCeiling) {
            float scale = static_cast<float>(out.globalDensityCeiling) / static_cast<float>(totalHits);
            for (int i = 0; i < out.activeLaneCount; ++i) {
                auto& lane = out.lanes[static_cast<size_t>(i)];
                int scaled = static_cast<int>(std::round(static_cast<float>(lane.hitCount) * scale));
                int floor = lane.constraints.densityMin > 0 ? lane.constraints.densityMin : 1;
                lane.hitCount = std::max(scaled, floor);
            }
        }
    }

    return out;
}

} // namespace poly
