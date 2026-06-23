#pragma once

#include <cmath>

#include "poly/types.h"

namespace poly {

GrooveState resolveMacros(const GrooveState& input);

struct MacroSmoother {
    MacroValues current{};
    MacroValues target{};
    bool initialized = false;

    static constexpr float kTransitionTimeSec = 0.4f;

    void setTarget(const MacroValues& t) { target = t; }

    void advance(double sampleRate, int blockSize) {
        if (!initialized) {
            current = target;
            initialized = true;
            return;
        }
        float blockDur = static_cast<float>(blockSize) / static_cast<float>(sampleRate);
        float coeff = 1.0f - std::exp(-blockDur / kTransitionTimeSec);
        smoothField(current.complexity, target.complexity, coeff);
        smoothField(current.density, target.density, coeff);
        smoothField(current.syncopation, target.syncopation, coeff);
        smoothField(current.swing, target.swing, coeff);
        smoothField(current.tension, target.tension, coeff);
        smoothField(current.humanize, target.humanize, coeff);
    }

    void snapToTarget() {
        current = target;
        initialized = true;
    }

private:
    static void smoothField(float& val, float tgt, float coeff) { val += (tgt - val) * coeff; }
};

} // namespace poly
