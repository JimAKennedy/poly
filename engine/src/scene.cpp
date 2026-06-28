#include "poly/scene.h"

#include <algorithm>
#include <cmath>

namespace poly {

namespace {

float lerpf(float a, float b, float t) {
    return a + t * (b - a);
}

template <typename T> T snap(T a, T b, float t) {
    return t < 0.5f ? a : b;
}

Envelope interpolateEnvelope(const Envelope& a, const Envelope& b, float t) {
    Envelope r{};
    r.target = snap(a.target, b.target, t);
    r.periodBars = lerpf(a.periodBars, b.periodBars, t);
    r.shape = snap(a.shape, b.shape, t);
    r.depth = lerpf(a.depth, b.depth, t);
    r.phaseOffset = lerpf(a.phaseOffset, b.phaseOffset, t);
    r.curvature = lerpf(a.curvature, b.curvature, t);
    r.stepCount = snap(a.stepCount, b.stepCount, t);
    for (int i = 0; i < kMaxStepListEntries; ++i) {
        auto idx = static_cast<size_t>(i);
        r.stepValues[idx] = lerpf(a.stepValues[idx], b.stepValues[idx], t);
    }
    return r;
}

LaneConfig interpolateLane(const LaneConfig& a, const LaneConfig& b, float t) {
    LaneConfig r{};
    r.id = snap(a.id, b.id, t);
    r.role = snap(a.role, b.role, t);
    r.midiNote = snap(a.midiNote, b.midiNote, t);
    r.cycle = snap(a.cycle, b.cycle, t);
    r.hitCount = static_cast<int>(std::round(lerpf(static_cast<float>(a.hitCount), static_cast<float>(b.hitCount), t)));
    r.rotation = static_cast<int>(std::round(lerpf(static_cast<float>(a.rotation), static_cast<float>(b.rotation), t)));
    r.probability = lerpf(a.probability, b.probability, t);
    r.baseVelocity = static_cast<uint8_t>(std::clamp(
        static_cast<int>(std::round(lerpf(static_cast<float>(a.baseVelocity), static_cast<float>(b.baseVelocity), t))),
        0, 127));
    r.accents = snap(a.accents, b.accents, t);
    r.emphasisProb = lerpf(a.emphasisProb, b.emphasisProb, t);
    r.ghostFloor = static_cast<uint8_t>(std::clamp(
        static_cast<int>(std::round(lerpf(static_cast<float>(a.ghostFloor), static_cast<float>(b.ghostFloor), t))), 0,
        127));
    r.velocitySpread = lerpf(a.velocitySpread, b.velocitySpread, t);
    r.humanizeMs = lerpf(a.humanizeMs, b.humanizeMs, t);
    r.swingAmount = lerpf(a.swingAmount, b.swingAmount, t);
    r.syncopationOffset = lerpf(a.syncopationOffset, b.syncopationOffset, t);
    r.tempoMultiplier = lerpf(a.tempoMultiplier, b.tempoMultiplier, t);
    r.noteDuration = lerpf(a.noteDuration, b.noteDuration, t);
    r.active = snap(a.active, b.active, t);
    r.envelopeCount = snap(a.envelopeCount, b.envelopeCount, t);
    for (int i = 0; i < kMaxEnvelopesPerLane; ++i) {
        auto idx = static_cast<size_t>(i);
        r.envelopes[idx].envelope = interpolateEnvelope(a.envelopes[idx].envelope, b.envelopes[idx].envelope, t);
        r.envelopes[idx].active = snap(a.envelopes[idx].active, b.envelopes[idx].active, t);
    }
    r.constraints.anchorSteps = snap(a.constraints.anchorSteps, b.constraints.anchorSteps, t);
    r.constraints.backbeatProtect = snap(a.constraints.backbeatProtect, b.constraints.backbeatProtect, t);
    r.constraints.densityMin = snap(a.constraints.densityMin, b.constraints.densityMin, t);
    r.constraints.densityMax = snap(a.constraints.densityMax, b.constraints.densityMax, t);
    return r;
}

} // namespace

GrooveState interpolateGrooveState(const GrooveState& a, const GrooveState& b, float t) {
    GrooveState r{};
    for (int i = 0; i < kMaxLanes; ++i) {
        auto idx = static_cast<size_t>(i);
        r.lanes[idx] = interpolateLane(a.lanes[idx], b.lanes[idx], t);
    }
    r.activeLaneCount = static_cast<int>(
        std::round(lerpf(static_cast<float>(a.activeLaneCount), static_cast<float>(b.activeLaneCount), t)));
    for (int i = 0; i < kMaxGlobalEnvelopes; ++i) {
        auto idx = static_cast<size_t>(i);
        r.globalEnvelopes[idx] = interpolateEnvelope(a.globalEnvelopes[idx], b.globalEnvelopes[idx], t);
    }
    r.globalEnvelopeCount = snap(a.globalEnvelopeCount, b.globalEnvelopeCount, t);
    r.macros = {
        lerpf(a.macros.complexity, b.macros.complexity, t),   lerpf(a.macros.density, b.macros.density, t),
        lerpf(a.macros.syncopation, b.macros.syncopation, t), lerpf(a.macros.swing, b.macros.swing, t),
        lerpf(a.macros.tension, b.macros.tension, t),         lerpf(a.macros.humanize, b.macros.humanize, t),
    };
    r.seed = snap(a.seed, b.seed, t);
    r.globalDensityCeiling = snap(a.globalDensityCeiling, b.globalDensityCeiling, t);
    return r;
}

} // namespace poly
