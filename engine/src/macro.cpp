#include "poly/macro.h"

#include <algorithm>
#include <cmath>

namespace poly {

static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// region:apply
GrooveState resolveMacros(const GrooveState& input) {
    GrooveState out = input;
    const auto& m = input.macros;

    for (int i = 0; i < out.activeLaneCount; ++i) {
        auto& lane = out.lanes[i];
        const auto& base = input.lanes[i];
        if (base.timeline)
            continue;
        int maxSteps = base.cycle.steps;
        if (maxSteps <= 0)
            continue;

        // --- Complexity: scales hitCount and rotation, deepens envelopes ---
        // At 0: hitCount toward 1, rotation 0, envelope depth halved
        // At 0.5: no change (passthrough)
        // At 1: hitCount toward maxSteps, rotation toward half-cycle, envelope depth doubled
        int minHits = 1;
        int maxHits = maxSteps;
        if (m.complexity < 0.5f) {
            float t = m.complexity * 2.0f;
            lane.hitCount =
                static_cast<int>(std::round(lerp(static_cast<float>(minHits), static_cast<float>(base.hitCount), t)));
        } else {
            float t = (m.complexity - 0.5f) * 2.0f;
            lane.hitCount =
                static_cast<int>(std::round(lerp(static_cast<float>(base.hitCount), static_cast<float>(maxHits), t)));
        }
        lane.hitCount = std::clamp(lane.hitCount, 0, maxSteps);

        float rotRange = static_cast<float>(maxSteps / 2);
        if (m.complexity < 0.5f) {
            float t = m.complexity * 2.0f;
            lane.rotation = static_cast<int>(std::round(lerp(0.0f, static_cast<float>(base.rotation), t)));
        } else {
            float t = (m.complexity - 0.5f) * 2.0f;
            lane.rotation = static_cast<int>(std::round(lerp(static_cast<float>(base.rotation), rotRange, t)));
        }

        float envDepthScale = lerp(0.5f, 2.0f, m.complexity);
        for (int e = 0; e < lane.envelopeCount; ++e) {
            lane.envelopes[e].envelope.depth = std::clamp(base.envelopes[e].envelope.depth * envDepthScale, 0.0f, 1.0f);
        }

        // --- Density: scales probability and hitCount ---
        // At 0: probability halved, hitCount toward 1
        // At 0.5: no change
        // At 1: probability toward 1.0, hitCount toward max
        if (m.density < 0.5f) {
            float t = m.density * 2.0f;
            lane.probability = lerp(base.probability * 0.5f, base.probability, t);
            int densityHits =
                static_cast<int>(std::round(lerp(static_cast<float>(minHits), static_cast<float>(lane.hitCount), t)));
            lane.hitCount = std::clamp(densityHits, 0, maxSteps);
        } else {
            float t = (m.density - 0.5f) * 2.0f;
            lane.probability = lerp(base.probability, 1.0f, t);
            int densityHits =
                static_cast<int>(std::round(lerp(static_cast<float>(lane.hitCount), static_cast<float>(maxHits), t)));
            lane.hitCount = std::clamp(densityHits, 0, maxSteps);
        }
        lane.probability = std::clamp(lane.probability, 0.0f, 1.0f);

        // --- Syncopation: timing displacement, rotation, and accent shift ---
        // At 0: no change
        // At 1: even steps pushed late (1/3 step), rotation += half cycle, emphasisProb inverted
        int syncopRotation = static_cast<int>(std::round(m.syncopation * rotRange));
        lane.rotation = (lane.rotation + syncopRotation) % maxSteps;

        lane.emphasisProb = lerp(base.emphasisProb, 1.0f - base.emphasisProb, m.syncopation);
        lane.emphasisProb = std::clamp(lane.emphasisProb, 0.0f, 1.0f);

        lane.syncopationOffset = m.syncopation;

        // --- Swing: sets swingAmount across all lanes ---
        // Additive: macro swing adds to per-lane swing
        lane.swingAmount = std::clamp(base.swingAmount + m.swing, 0.0f, 1.0f);

        // --- Tension: widens velocity spread, pushes emphasisProb, deepens envelopes ---
        // At 0: spread halved, emphasisProb toward 0
        // At 0.5: no change
        // At 1: spread doubled, emphasisProb toward 1
        if (m.tension < 0.5f) {
            float t = m.tension * 2.0f;
            lane.velocitySpread = lerp(base.velocitySpread * 0.5f, base.velocitySpread, t);
        } else {
            float t = (m.tension - 0.5f) * 2.0f;
            lane.velocitySpread = lerp(base.velocitySpread, std::min(base.velocitySpread * 2.0f, 0.5f), t);
        }

        float tensionEmphasis = lerp(0.0f, 1.0f, m.tension);
        lane.emphasisProb = lerp(lane.emphasisProb, tensionEmphasis, 0.5f);
        lane.emphasisProb = std::clamp(lane.emphasisProb, 0.0f, 1.0f);

        float tensionEnvScale = lerp(0.5f, 2.0f, m.tension);
        for (int e = 0; e < lane.envelopeCount; ++e) {
            lane.envelopes[e].envelope.depth =
                std::clamp(lane.envelopes[e].envelope.depth * tensionEnvScale, 0.0f, 1.0f);
        }

        // --- Humanize: sets humanizeMs and widens velocity spread ---
        // At 0: no humanize
        // At 1: humanizeMs up to 25ms, spread increased
        lane.humanizeMs = std::clamp(base.humanizeMs + m.humanize * 25.0f, 0.0f, 50.0f);
        lane.velocitySpread = std::clamp(lane.velocitySpread + m.humanize * 0.05f, 0.0f, 0.5f);
    }

    return out;
}
// endregion:apply

} // namespace poly
