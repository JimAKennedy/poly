#include "poly/engine.h"

#include <algorithm>
#include <cmath>

#include "poly/envelope.h"
#include "poly/euclidean.h"
#include "poly/rng.h"

namespace poly {

static constexpr double kPpqPerBar = 4.0;
static constexpr double kMsPerMinute = 60000.0;
static constexpr float kMidiVelocityMax = 127.0f;
static constexpr float kMutationDropThreshold = 0.4f;
static constexpr float kMutationGhostThreshold = 0.7f;
static constexpr float kAccentVelocityBoost = 0.15f;
static constexpr float kHumanizeEnvelopeScale = 10.0f;
static constexpr double kTimingSafetyMarginMs = 20.0;
static constexpr double kDefaultDurationFraction = 0.5;
static constexpr double kSwingSyncopationDivisor = 3.0;

static double stepPpq(const Cycle& cycle) {
    return kPpqPerBar / cycle.subdivision;
}

struct EnvelopeMods {
    float velocity = 1.0f;
    float probability = 0.0f;
    float accent = 0.0f;
    float duration = 1.0f;
    float humanize = 0.0f;
    float activation = 0.0f;
    float fill = 0.0f;
};

static void accumulateEnvelope(const Envelope& env, double ppq, EnvelopeMods& mods) {
    double phase = computeEnvelopePhase(ppq, env.periodBars, env.phaseOffset);
    float value = evaluateShapeFull(env, static_cast<float>(phase));
    switch (env.target) {
    case EnvTarget::Velocity:
        mods.velocity *= (1.0f - env.depth * (1.0f - value));
        break;
    case EnvTarget::Density:
    case EnvTarget::Probability:
        mods.probability += env.depth * (value * 2.0f - 1.0f);
        break;
    case EnvTarget::AccentBias:
        mods.accent += env.depth * (value * 2.0f - 1.0f);
        break;
    case EnvTarget::NoteLength:
        mods.duration *= (1.0f - env.depth * (1.0f - value));
        break;
    case EnvTarget::TimingLooseness:
        mods.humanize += env.depth * value;
        break;
    case EnvTarget::ActivationWeight:
        mods.activation += env.depth * (value * 2.0f - 1.0f);
        break;
    case EnvTarget::FillLikelihood:
        mods.fill += env.depth * value;
        break;
    }
}

static EnvelopeMods computeEnvelopeMods(const LaneConfig& cfg, const GrooveState& state, double ppq) {
    EnvelopeMods mods{};
    for (int e = 0; e < cfg.envelopeCount; ++e) {
        const auto& ea = cfg.envelopes[e];
        if (!ea.active)
            continue;
        accumulateEnvelope(ea.envelope, ppq, mods);
    }
    for (int e = 0; e < state.globalEnvelopeCount; ++e) {
        accumulateEnvelope(state.globalEnvelopes[e], ppq, mods);
    }
    return mods;
}

static void buildLanePattern(const LaneConfig& cfg, const GrooveState& state, int lane,
                             std::array<bool, kMaxSteps>& pattern) {
    pattern = {};
    if (cfg.timeline) {
        int patLen = cfg.fixedPatternLength > 0 ? cfg.fixedPatternLength : cfg.cycle.steps;
        for (int s = 0; s < patLen && s < kMaxSteps; ++s)
            pattern[s] = cfg.fixedPattern[s];
    } else {
        // region:kotekan
        bool useKotekan = cfg.kotekanSourceLane >= 0 && cfg.kotekanSourceLane < state.activeLaneCount &&
                          cfg.kotekanSourceLane != lane;
        if (useKotekan) {
            const auto& src = state.lanes[cfg.kotekanSourceLane];
            if (src.kotekanSourceLane == lane)
                useKotekan = false;
        }
        if (useKotekan) {
            const auto& src = state.lanes[cfg.kotekanSourceLane];
            std::array<bool, kMaxSteps> srcPattern{};
            euclidean(src.hitCount, src.cycle.steps, src.rotation, srcPattern);
            for (int s = 0; s < cfg.cycle.steps && s < src.cycle.steps; ++s)
                pattern[s] = !srcPattern[s];
            for (int s = src.cycle.steps; s < cfg.cycle.steps; ++s)
                pattern[s] = true;
            // endregion:kotekan
        } else {
            euclidean(cfg.hitCount, cfg.cycle.steps, cfg.rotation, pattern);
        }
    }
}

static float computeMaxEnvelopeHumanizeMs(const LaneConfig& cfg, const GrooveState& state) {
    float maxMs = 0.0f;
    for (int e = 0; e < cfg.envelopeCount; ++e) {
        const auto& ea = cfg.envelopes[e];
        if (ea.active && ea.envelope.target == EnvTarget::TimingLooseness)
            maxMs += std::abs(ea.envelope.depth) * kHumanizeEnvelopeScale;
    }
    for (int e = 0; e < state.globalEnvelopeCount; ++e) {
        if (state.globalEnvelopes[e].target == EnvTarget::TimingLooseness)
            maxMs += std::abs(state.globalEnvelopes[e].depth) * kHumanizeEnvelopeScale;
    }
    return maxMs;
}

// M045 S01 T01: classifyStep supersedes shouldEmitStep. Returns one of five
// outcomes so the caller can both decide whether to emit a NoteEvent and
// record a truthful EmissionEvent for the display: Base/Ghost/Add all emit;
// Drop suppresses an on-pattern hit (mutation or probability/activation/fill
// cull); Silent means an off-pattern step that never fired — nothing worth
// recording.
enum class StepOutcome : uint8_t {
    Base = 0,
    Ghost = 1,
    Add = 2,
    Drop = 3,
    Silent = 255,
};

static StepOutcome classifyStep(const LaneConfig& cfg, const GrooveState& state, int64_t absStep, int64_t cycleStep,
                                bool isPatternStep, bool isAnchor, const EnvelopeMods& mods, int stepsInCycle) {
    const bool wasPatternStep = isPatternStep;
    bool mutatedToGhost = false;

    if (cfg.mutationRate > 0.0f && !isAnchor) {
        int64_t cycleIndex = (absStep >= 0) ? absStep / stepsInCycle : (absStep - stepsInCycle + 1) / stepsInCycle;
        float mutRoll = deterministicRand(state.seed, cfg.id, cycleIndex * kMaxSteps + cycleStep, 8);
        if (mutRoll < cfg.mutationRate) {
            float typeRoll = deterministicRand(state.seed, cfg.id, cycleIndex * kMaxSteps + cycleStep, 9);
            if (typeRoll < kMutationDropThreshold) {
                if (isPatternStep)
                    isPatternStep = false;
            } else if (typeRoll < kMutationGhostThreshold) {
                if (isPatternStep)
                    mutatedToGhost = true;
            } else {
                if (!isPatternStep)
                    isPatternStep = true;
            }
        }
    }

    auto notEmitted = [wasPatternStep]() { return wasPatternStep ? StepOutcome::Drop : StepOutcome::Silent; };

    if (!isAnchor) {
        if (!isPatternStep) {
            if (mods.fill <= 0.0f)
                return notEmitted();
            float fillProb = std::clamp(mods.fill, 0.0f, 1.0f);
            float fillRoll = deterministicRand(state.seed, cfg.id, absStep, 4);
            if (fillRoll >= fillProb)
                return notEmitted();
        }

        if (mods.activation < 0.0f) {
            float activationProb = std::clamp(1.0f + mods.activation, 0.0f, 1.0f);
            float actRoll = deterministicRand(state.seed, cfg.id, absStep, 5);
            if (actRoll >= activationProb)
                return notEmitted();
        }

        float effectiveProb = std::clamp(cfg.probability + mods.probability, 0.0f, 1.0f);
        float probRoll = deterministicRand(state.seed, cfg.id, absStep, 0);
        if (probRoll >= effectiveProb)
            return notEmitted();
    }

    if (mutatedToGhost)
        return StepOutcome::Ghost;
    if (wasPatternStep)
        return StepOutcome::Base;
    return StepOutcome::Add;
}

static float computeStepVelocity(const LaneConfig& cfg, const GrooveState& state, int64_t absStep, int64_t cycleStep,
                                 const EnvelopeMods& mods, bool mutatedToGhost) {
    float velBase = cfg.baseVelocity / kMidiVelocityMax;
    float velRand = deterministicRand(state.seed, cfg.id, absStep, 1);
    float spread = cfg.velocitySpread * (velRand * 2.0f - 1.0f);
    float vel = velBase + spread;

    float accentVal = cfg.accents.steps[static_cast<size_t>(cycleStep)];
    if (accentVal > 0.0f) {
        float effectiveEmphasis = std::clamp(cfg.emphasisProb + mods.accent, 0.0f, 1.0f);
        float emphRoll = deterministicRand(state.seed, cfg.id, absStep, 2);
        if (emphRoll < effectiveEmphasis) {
            vel += accentVal * kAccentVelocityBoost;
        }
    }

    float ghostFloor = cfg.ghostFloor / kMidiVelocityMax;
    if (vel < ghostFloor)
        vel = ghostFloor;

    vel *= mods.velocity;
    vel = std::clamp(vel, 0.0f, 1.0f);

    if (mutatedToGhost)
        vel = ghostFloor;

    return vel;
}

static double applyTimingShifts(const LaneConfig& cfg, const TransportContext& tc, const GrooveState& state, double ppq,
                                double stepDurPpq, int64_t absStep, int64_t cycleStep, float humanizeMod) {
    if (cfg.swingAmount > 0.0f && (cycleStep % 2) == 1) {
        ppq += cfg.swingAmount * stepDurPpq * (1.0 / kSwingSyncopationDivisor);
    }
    if (cfg.syncopationOffset > 0.0f && (cycleStep % 2) == 0) {
        ppq += static_cast<double>(cfg.syncopationOffset) * stepDurPpq * (1.0 / kSwingSyncopationDivisor);
    }

    float stepTimingMs = cfg.microTimingMs[static_cast<size_t>(cycleStep)];
    if (stepTimingMs != 0.0f && tc.tempo > 0.0) {
        ppq += static_cast<double>(stepTimingMs) * tc.tempo / kMsPerMinute;
    }

    float effectiveHumanize = cfg.humanizeMs + humanizeMod * kHumanizeEnvelopeScale;
    if (effectiveHumanize > 0.0f && tc.tempo > 0.0) {
        double jitterPpq = static_cast<double>(effectiveHumanize) * tc.tempo / kMsPerMinute;
        float jitterRand = deterministicRand(state.seed, cfg.id, absStep, 3);
        ppq += jitterPpq * (jitterRand * 2.0f - 1.0f);
    }

    if (cfg.timingOffsetMs != 0.0f && tc.tempo > 0.0) {
        ppq += static_cast<double>(cfg.timingOffsetMs) * tc.tempo / kMsPerMinute;
    }

    if (ppq < 0.0)
        ppq = 0.0;

    return ppq;
}

struct LaneRenderContext {
    std::array<bool, kMaxSteps> pattern{};
    AdditiveCellInfo additive{};
    double sPpq = 0.0;
    double cyclePpqLen = 0.0;
    double maxTimingShift = 0.0;
    int64_t firstStep = 0;
    int64_t lastStep = 0;
    double phraseLenPpq = 0.0;
    double phraseCyclePpq = 0.0;
    double phraseOffPpq = 0.0;
    bool hasPhraseGating = false;
    bool isAdditive = false;
    // M049 S02 (E2): authoritative cycle length for wrap. In timeline mode with
    // fixedPatternLength > 0, the pattern's own length governs playback wrap —
    // otherwise a lane with fixedPatternLength=10 / cycle.steps=16 plays 16
    // steps with a 6-step silent tail, contradicting every editor that shows
    // only the fixedPatternLength slots as editable.
    int stepsInCycle = 0;
};

static LaneRenderContext prepareLaneContext(const LaneConfig& cfg, const GrooveState& state, int lane,
                                            const TransportContext& tc) {
    LaneRenderContext ctx{};
    buildLanePattern(cfg, state, lane, ctx.pattern);

    const double tempoScale = (cfg.tempoMultiplier > 0.0f) ? 1.0 / static_cast<double>(cfg.tempoMultiplier) : 1.0;
    ctx.sPpq = stepPpq(cfg.cycle) * tempoScale;
    ctx.additive = computeAdditiveCells(cfg);
    if (tempoScale != 1.0 && ctx.additive.count > 0) {
        for (int c = 0; c < ctx.additive.count; ++c)
            ctx.additive.cumPpq[c] *= tempoScale;
        ctx.additive.totalPpq *= tempoScale;
    }
    ctx.isAdditive = ctx.additive.count > 0;
    // M049 S02 (E2): fixedPatternLength governs wrap in timeline mode.
    if (ctx.isAdditive)
        ctx.stepsInCycle = ctx.additive.count;
    else if (cfg.timeline && cfg.fixedPatternLength > 0)
        ctx.stepsInCycle = cfg.fixedPatternLength;
    else
        ctx.stepsInCycle = cfg.cycle.steps;
    ctx.cyclePpqLen = ctx.isAdditive ? ctx.additive.totalPpq : (ctx.stepsInCycle * ctx.sPpq);

    double maxStepDur = ctx.sPpq;
    if (ctx.isAdditive) {
        for (int c = 0; c < ctx.additive.count; ++c)
            maxStepDur = std::max(maxStepDur, cfg.cellSizes[c] * ctx.sPpq);
    }
    ctx.maxTimingShift = 0.0;
    ctx.maxTimingShift += static_cast<double>(std::max(0.0f, cfg.swingAmount)) * maxStepDur / kSwingSyncopationDivisor;
    ctx.maxTimingShift +=
        static_cast<double>(std::max(0.0f, cfg.syncopationOffset)) * maxStepDur / kSwingSyncopationDivisor;
    if (tc.tempo > 0.0) {
        float envHumanizeMs = computeMaxEnvelopeHumanizeMs(cfg, state);
        ctx.maxTimingShift +=
            static_cast<double>(std::max(0.0f, cfg.humanizeMs) + envHumanizeMs) * tc.tempo / kMsPerMinute;
        ctx.maxTimingShift += std::abs(static_cast<double>(cfg.timingOffsetMs)) * tc.tempo / kMsPerMinute;
        ctx.maxTimingShift += kTimingSafetyMarginMs * tc.tempo / kMsPerMinute;
    }

    if (ctx.isAdditive) {
        int64_t firstCycle = static_cast<int64_t>(std::floor((tc.ppqStart - ctx.maxTimingShift) / ctx.cyclePpqLen));
        int64_t lastCycle = static_cast<int64_t>(std::floor((tc.ppqEnd + ctx.maxTimingShift) / ctx.cyclePpqLen)) + 1;
        ctx.firstStep = firstCycle * ctx.additive.count;
        ctx.lastStep = lastCycle * ctx.additive.count;
    } else {
        ctx.firstStep = static_cast<int64_t>(std::floor((tc.ppqStart - ctx.maxTimingShift) / ctx.sPpq));
        ctx.lastStep = static_cast<int64_t>(std::ceil((tc.ppqEnd + ctx.maxTimingShift) / ctx.sPpq));
    }

    ctx.hasPhraseGating = cfg.phraseLength > 0.0f;
    ctx.phraseLenPpq = static_cast<double>(cfg.phraseLength);
    ctx.phraseCyclePpq = ctx.phraseLenPpq + static_cast<double>(cfg.phraseGap);
    ctx.phraseOffPpq = static_cast<double>(cfg.phraseOffset);

    return ctx;
}

static void computeStepPpqAndDuration(const LaneRenderContext& ctx, const LaneConfig& cfg, int64_t absStep, double& ppq,
                                      double& stepDurPpq) {
    if (ctx.isAdditive) {
        int localCell = static_cast<int>(((absStep % ctx.additive.count) + ctx.additive.count) % ctx.additive.count);
        int64_t cycleIdx =
            (absStep >= 0) ? absStep / ctx.additive.count : (absStep - ctx.additive.count + 1) / ctx.additive.count;
        ppq = cycleIdx * ctx.cyclePpqLen + ctx.additive.cumPpq[localCell];
        stepDurPpq = cfg.cellSizes[localCell] * ctx.sPpq;
    } else {
        ppq = absStep * ctx.sPpq;
        stepDurPpq = ctx.sPpq;
    }
}

static bool passesPhraseGating(const LaneRenderContext& ctx, double ppq) {
    if (!ctx.hasPhraseGating || ctx.phraseCyclePpq <= 0.0)
        return true;
    double phrasePos = std::fmod(ppq - ctx.phraseOffPpq, ctx.phraseCyclePpq);
    if (phrasePos < 0.0)
        phrasePos += ctx.phraseCyclePpq;
    return phrasePos < ctx.phraseLenPpq;
}

// region:drift-accumulator
static int64_t computeDriftedCycleStep(const LaneConfig& cfg, const LaneRenderContext& ctx, int64_t absStep,
                                       double ppq) {
    int stepsInCycle = ctx.stepsInCycle;
    int64_t cycleStep = ((absStep % stepsInCycle) + stepsInCycle) % stepsInCycle;
    if (cfg.driftRate != 0.0f) {
        double barPos = ppq / kPpqPerBar;
        auto driftSteps = static_cast<int64_t>(std::floor(barPos * static_cast<double>(cfg.driftRate)));
        cycleStep = ((cycleStep + driftSteps) % stepsInCycle + stepsInCycle) % stepsInCycle;
    }
    return cycleStep;
}
// endregion:drift-accumulator

static void buildNoteEvent(NoteEventBuffer& out, const LaneConfig& cfg, int lane, double ppq, float vel,
                           double stepDurPpq, const EnvelopeMods& mods) {
    NoteEvent ev{};
    ev.ppqPosition = ppq;
    ev.pitch = cfg.midiNote;
    ev.velocity = vel;
    double baseDuration =
        cfg.noteDuration > 0.0f ? static_cast<double>(cfg.noteDuration) : stepDurPpq * kDefaultDurationFraction;
    ev.duration = baseDuration * static_cast<double>(std::clamp(mods.duration, 0.01f, 4.0f));
    ev.channel = (cfg.midiChannel >= 0) ? cfg.midiChannel : static_cast<int16_t>(lane);
    ev.laneIndex = static_cast<int16_t>(lane);
    out.push(ev);
}

void Engine::renderRange(const TransportContext& tc, const GrooveState& state, NoteEventBuffer& out,
                         EmissionEventBuffer* emissions) {
    out.clear();
    if (emissions)
        emissions->clear();

    if (!tc.playing || tc.ppqEnd <= tc.ppqStart)
        return;

    const int laneCount = std::clamp(state.activeLaneCount, 0, kMaxLanes);
    for (int lane = 0; lane < laneCount; ++lane) {
        const auto& cfg = state.lanes[lane];
        if (!cfg.active || cfg.cycle.steps <= 0 || cfg.cycle.steps > kMaxSteps || cfg.cycle.subdivision <= 0)
            continue;

        auto ctx = prepareLaneContext(cfg, state, lane, tc);

        if (ctx.cyclePpqLen <= 0.0)
            continue;

        for (int64_t absStep = ctx.firstStep; absStep < ctx.lastStep; ++absStep) {
            double ppq = 0.0;
            double stepDurPpq = 0.0;
            computeStepPpqAndDuration(ctx, cfg, absStep, ppq, stepDurPpq);

            if (ppq < tc.ppqStart - ctx.maxTimingShift || ppq >= tc.ppqEnd + ctx.maxTimingShift)
                continue;
            if (!passesPhraseGating(ctx, ppq))
                continue;

            int64_t cycleStep = computeDriftedCycleStep(cfg, ctx, absStep, ppq);
            EnvelopeMods mods = computeEnvelopeMods(cfg, state, ppq);

            bool isPatternStep = ctx.pattern[static_cast<size_t>(cycleStep)];
            bool isAnchor = cfg.constraints.anchorSteps.steps[static_cast<size_t>(cycleStep)] > 0.0f;
            StepOutcome outcome =
                classifyStep(cfg, state, absStep, cycleStep, isPatternStep, isAnchor, mods, ctx.stepsInCycle);

            // Record classification for the display, but only for steps whose
            // pre-timing-shift ppq falls inside the current render window —
            // otherwise the outer safety margin double-counts across block
            // boundaries. Silent (off-pattern, no fire) is never recorded.
            if (emissions != nullptr && outcome != StepOutcome::Silent && ppq >= tc.ppqStart && ppq < tc.ppqEnd) {
                EmissionEvent ee{};
                ee.ppqPosition = ppq;
                ee.cycleStep = static_cast<int16_t>(cycleStep);
                ee.laneIndex = static_cast<int16_t>(lane);
                ee.kind = static_cast<uint8_t>(outcome);
                emissions->push(ee);
            }

            if (outcome == StepOutcome::Drop || outcome == StepOutcome::Silent)
                continue;

            bool mutatedToGhost = (outcome == StepOutcome::Ghost);
            float vel = computeStepVelocity(cfg, state, absStep, cycleStep, mods, mutatedToGhost);
            ppq = applyTimingShifts(cfg, tc, state, ppq, stepDurPpq, absStep, cycleStep, mods.humanize);
            if (ppq < tc.ppqStart || ppq >= tc.ppqEnd)
                continue;

            buildNoteEvent(out, cfg, lane, ppq, vel, stepDurPpq, mods);
        }
    }
}

} // namespace poly
