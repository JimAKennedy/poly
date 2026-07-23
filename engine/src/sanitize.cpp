#include "poly/sanitize.h"

#include <algorithm>
#include <cmath>

namespace poly {

namespace {

// NaN-safe float clamp: NaN compares false everywhere, so test for the valid
// range and replace anything else (including NaN/inf) with the fallback.
float clampf(float v, float lo, float hi, float fallback) {
    if (v >= lo && v <= hi)
        return v;
    if (v > hi)
        return hi;
    if (v < lo)
        return lo;
    return fallback; // NaN
}

int clampi(int v, int lo, int hi) {
    return std::clamp(v, lo, hi);
}

void sanitizeEnvelope(Envelope& env) {
    if (static_cast<uint8_t>(env.target) > static_cast<uint8_t>(EnvTarget::FillLikelihood))
        env.target = EnvTarget::Velocity;
    if (static_cast<uint8_t>(env.shape) > static_cast<uint8_t>(Shape::StepList))
        env.shape = Shape::Sine;
    // periodBars must be strictly positive: computeEnvelopePhase divides by it.
    env.periodBars = clampf(env.periodBars, 0.0625f, 1024.0f, 4.0f);
    env.depth = clampf(env.depth, -4.0f, 4.0f, 1.0f);
    env.phaseOffset = clampf(env.phaseOffset, -64.0f, 64.0f, 0.0f);
    env.curvature = clampf(env.curvature, -1.0f, 1.0f, 0.0f);
    env.stepCount = clampi(env.stepCount, 0, kMaxStepListEntries);
    for (auto& v : env.stepValues)
        v = clampf(v, -1.0f, 1.0f, 0.0f);
}

void sanitizeAccentMask(AccentMask& mask) {
    for (auto& v : mask.steps)
        v = clampf(v, 0.0f, 1.0f, 0.0f);
}

void sanitizeLane(LaneConfig& lane, int laneIndex) {
    // Repair the lane.id == laneIndex invariant. `deterministicRand` in
    // engine.cpp keys 8 per-step decision channels on `cfg.id`; two lanes
    // sharing an id would produce byte-identical rolls, silently correlating
    // what should be independent lanes. Both live init sites already assign
    // .id = laneIndex (processor.cpp setActive, web_ui_view.cpp createInitial);
    // this makes the invariant universal so hostile / corrupted presets can't
    // break it either. (E4, M049 S04.)
    lane.id = laneIndex;

    if (static_cast<uint8_t>(lane.role) > static_cast<uint8_t>(Role::Custom))
        lane.role = Role::Custom;
    lane.midiNote = static_cast<int16_t>(clampi(lane.midiNote, 0, 127));
    lane.midiChannel = static_cast<int16_t>(clampi(lane.midiChannel, -1, 15));

    lane.cycle.steps = clampi(lane.cycle.steps, 1, kMaxSteps);
    lane.cycle.subdivision = clampi(lane.cycle.subdivision, 1, 16);
    lane.hitCount = clampi(lane.hitCount, 0, lane.cycle.steps);
    lane.rotation = clampi(lane.rotation, -kMaxSteps, kMaxSteps);

    lane.probability = clampf(lane.probability, 0.0f, 1.0f, 1.0f);
    sanitizeAccentMask(lane.accents);
    lane.emphasisProb = clampf(lane.emphasisProb, 0.0f, 1.0f, 0.5f);
    lane.velocitySpread = clampf(lane.velocitySpread, 0.0f, 1.0f, 0.0f);
    lane.humanizeMs = clampf(lane.humanizeMs, 0.0f, 100.0f, 0.0f);
    lane.swingAmount = clampf(lane.swingAmount, 0.0f, 1.0f, 0.0f);
    lane.noteDuration = clampf(lane.noteDuration, 0.0f, 16.0f, 0.0f);
    lane.phraseLength = clampf(lane.phraseLength, 0.0f, 4096.0f, 0.0f);
    lane.phraseGap = clampf(lane.phraseGap, 0.0f, 4096.0f, 0.0f);
    lane.phraseOffset = clampf(lane.phraseOffset, 0.0f, 4096.0f, 0.0f);
    lane.mutationRate = clampf(lane.mutationRate, 0.0f, 1.0f, 0.0f);
    lane.driftRate = clampf(lane.driftRate, -64.0f, 64.0f, 0.0f);
    lane.timingOffsetMs = clampf(lane.timingOffsetMs, -20.0f, 20.0f, 0.0f);
    lane.syncopationOffset = clampf(lane.syncopationOffset, 0.0f, 1.0f, 0.0f);
    // tempoMultiplier scales cycle math — zero/negative would zero the cycle.
    lane.tempoMultiplier = clampf(lane.tempoMultiplier, 0.25f, 4.0f, 1.0f);

    lane.kotekanSourceLane = clampi(lane.kotekanSourceLane, -1, kMaxLanes - 1);
    if (lane.kotekanSourceLane == laneIndex)
        lane.kotekanSourceLane = -1;

    lane.cellCount = clampi(lane.cellCount, 0, kMaxSteps);
    // Each active cell must be >= 1 subdivision unit so the additive cycle
    // length can never be zero (renderRange divides by it).
    for (int i = 0; i < lane.cellCount; ++i)
        lane.cellSizes[static_cast<size_t>(i)] = clampi(lane.cellSizes[static_cast<size_t>(i)], 1, kMaxSteps);

    lane.fixedPatternLength = clampi(lane.fixedPatternLength, 0, kMaxSteps);
    for (auto& v : lane.microTimingMs)
        v = clampf(v, -20.0f, 20.0f, 0.0f);

    lane.envelopeCount = clampi(lane.envelopeCount, 0, kMaxEnvelopesPerLane);
    for (auto& assign : lane.envelopes)
        sanitizeEnvelope(assign.envelope);

    sanitizeAccentMask(lane.constraints.anchorSteps);
    lane.constraints.densityMin = clampi(lane.constraints.densityMin, 0, kMaxSteps);
    lane.constraints.densityMax = clampi(lane.constraints.densityMax, lane.constraints.densityMin, kMaxSteps);
}

} // namespace

void sanitizeGrooveState(GrooveState& state) {
    state.activeLaneCount = clampi(state.activeLaneCount, 0, kMaxLanes);
    for (int i = 0; i < kMaxLanes; ++i)
        sanitizeLane(state.lanes[static_cast<size_t>(i)], i);

    state.globalEnvelopeCount = clampi(state.globalEnvelopeCount, 0, kMaxGlobalEnvelopes);
    for (auto& env : state.globalEnvelopes)
        sanitizeEnvelope(env);

    state.macros.complexity = clampf(state.macros.complexity, 0.0f, 1.0f, 0.5f);
    state.macros.density = clampf(state.macros.density, 0.0f, 1.0f, 0.5f);
    state.macros.syncopation = clampf(state.macros.syncopation, 0.0f, 1.0f, 0.0f);
    state.macros.swing = clampf(state.macros.swing, 0.0f, 1.0f, 0.0f);
    state.macros.tension = clampf(state.macros.tension, 0.0f, 1.0f, 0.0f);
    state.macros.humanize = clampf(state.macros.humanize, 0.0f, 1.0f, 0.0f);

    state.globalDensityCeiling = clampi(state.globalDensityCeiling, 0, kMaxLanes * kMaxSteps);
}

void sanitizeSceneState(SceneState& scene) {
    sanitizeGrooveState(scene.sceneA);
    sanitizeGrooveState(scene.sceneB);

    if (static_cast<uint8_t>(scene.select) > static_cast<uint8_t>(SceneSelect::Morph))
        scene.select = SceneSelect::A;
    scene.morphAmount = clampf(scene.morphAmount, 0.0f, 1.0f, 0.0f);

    for (auto& v : scene.noteMap.map)
        v = static_cast<int16_t>(std::clamp<int>(v, 0, 127));

    scene.chain.entryCount = std::clamp(scene.chain.entryCount, 0, kMaxChainEntries);
    if (static_cast<uint8_t>(scene.chain.mode) > static_cast<uint8_t>(ChainMode::PingPong))
        scene.chain.mode = ChainMode::Loop;
    for (auto& entry : scene.chain.entries) {
        if (static_cast<uint8_t>(entry.scene) > static_cast<uint8_t>(SceneSelect::Morph))
            entry.scene = SceneSelect::A;
        entry.bars = std::clamp(entry.bars, 1, 1024);
    }
}

} // namespace poly
