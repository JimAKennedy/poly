#include "poly/engine.h"

#include <algorithm>
#include <cmath>

#include "poly/envelope.h"
#include "poly/euclidean.h"
#include "poly/rng.h"

namespace poly {

static double stepPpq(const Cycle& cycle) {
    return 4.0 / cycle.subdivision;
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
        } else {
            euclidean(cfg.hitCount, cfg.cycle.steps, cfg.rotation, pattern);
        }
    }
}

static double computeMaxTimingShift(const LaneConfig& cfg, const TransportContext& tc, double maxStepDur,
                                    const AdditiveCellInfo& additive) {
    double sPpq = stepPpq(cfg.cycle);
    if (cfg.tempoMultiplier > 0.0f)
        sPpq *= 1.0 / static_cast<double>(cfg.tempoMultiplier);

    double effectiveMaxDur = maxStepDur;
    if (additive.count > 0) {
        for (int c = 0; c < additive.count; ++c)
            effectiveMaxDur = std::max(effectiveMaxDur, cfg.cellSizes[c] * sPpq);
    }

    double shift = 0.0;
    shift += static_cast<double>(std::max(0.0f, cfg.swingAmount)) * effectiveMaxDur / 3.0;
    shift += static_cast<double>(std::max(0.0f, cfg.syncopationOffset)) * effectiveMaxDur / 3.0;
    if (tc.tempo > 0.0) {
        shift += (static_cast<double>(std::max(0.0f, cfg.humanizeMs)) + 50.0) * tc.tempo / 60000.0;
        shift += std::abs(static_cast<double>(cfg.timingOffsetMs)) * tc.tempo / 60000.0;
        shift += 20.0 * tc.tempo / 60000.0;
    }
    return shift;
}

static bool shouldEmitStep(const LaneConfig& cfg, const GrooveState& state, int64_t absStep, int64_t cycleStep,
                           bool isPatternStep, bool isAnchor, const EnvelopeMods& mods, bool& mutatedToGhost) {
    int stepsInCycle = (cfg.cellCount > 0) ? cfg.cellCount : cfg.cycle.steps;
    mutatedToGhost = false;

    if (cfg.mutationRate > 0.0f && !isAnchor) {
        int64_t cycleIndex = absStep / stepsInCycle;
        float mutRoll = deterministicRand(state.seed, cfg.id, cycleIndex * kMaxSteps + cycleStep, 8);
        if (mutRoll < cfg.mutationRate) {
            float typeRoll = deterministicRand(state.seed, cfg.id, cycleIndex * kMaxSteps + cycleStep, 9);
            if (typeRoll < 0.4f) {
                if (isPatternStep)
                    isPatternStep = false;
            } else if (typeRoll < 0.7f) {
                if (isPatternStep)
                    mutatedToGhost = true;
            } else {
                if (!isPatternStep)
                    isPatternStep = true;
            }
        }
    }

    if (!isAnchor) {
        if (!isPatternStep) {
            if (mods.fill <= 0.0f)
                return false;
            float fillProb = std::clamp(mods.fill, 0.0f, 1.0f);
            float fillRoll = deterministicRand(state.seed, cfg.id, absStep, 4);
            if (fillRoll >= fillProb)
                return false;
        }

        if (mods.activation < 0.0f) {
            float activationProb = std::clamp(1.0f + mods.activation, 0.0f, 1.0f);
            float actRoll = deterministicRand(state.seed, cfg.id, absStep, 5);
            if (actRoll >= activationProb)
                return false;
        }

        float effectiveProb = std::clamp(cfg.probability + mods.probability, 0.0f, 1.0f);
        float probRoll = deterministicRand(state.seed, cfg.id, absStep, 0);
        if (probRoll >= effectiveProb)
            return false;
    }
    return true;
}

static float computeStepVelocity(const LaneConfig& cfg, const GrooveState& state, int64_t absStep, int64_t cycleStep,
                                 const EnvelopeMods& mods, bool mutatedToGhost) {
    float velBase = cfg.baseVelocity / 127.0f;
    float velRand = deterministicRand(state.seed, cfg.id, absStep, 1);
    float spread = cfg.velocitySpread * (velRand * 2.0f - 1.0f);
    float vel = velBase + spread;

    float accentVal = cfg.accents.steps[static_cast<size_t>(cycleStep)];
    if (accentVal > 0.0f) {
        float effectiveEmphasis = std::clamp(cfg.emphasisProb + mods.accent, 0.0f, 1.0f);
        float emphRoll = deterministicRand(state.seed, cfg.id, absStep, 2);
        if (emphRoll < effectiveEmphasis) {
            vel += accentVal * 0.15f;
        }
    }

    float ghostFloor = cfg.ghostFloor / 127.0f;
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
        ppq += cfg.swingAmount * stepDurPpq * (1.0 / 3.0);
    }
    if (cfg.syncopationOffset > 0.0f && (cycleStep % 2) == 0) {
        ppq += static_cast<double>(cfg.syncopationOffset) * stepDurPpq * (1.0 / 3.0);
    }

    float stepTimingMs = cfg.microTimingMs[static_cast<size_t>(cycleStep)];
    if (stepTimingMs != 0.0f && tc.tempo > 0.0) {
        ppq += static_cast<double>(stepTimingMs) * tc.tempo / 60000.0;
    }

    float effectiveHumanize = cfg.humanizeMs + humanizeMod * 10.0f;
    if (effectiveHumanize > 0.0f && tc.tempo > 0.0) {
        double jitterPpq = static_cast<double>(effectiveHumanize) * tc.tempo / 60000.0;
        float jitterRand = deterministicRand(state.seed, cfg.id, absStep, 3);
        ppq += jitterPpq * (jitterRand * 2.0f - 1.0f);
    }

    if (cfg.timingOffsetMs != 0.0f && tc.tempo > 0.0) {
        ppq += static_cast<double>(cfg.timingOffsetMs) * tc.tempo / 60000.0;
    }

    if (ppq < 0.0)
        ppq = 0.0;

    return ppq;
}

void Engine::renderRange(const TransportContext& tc, const GrooveState& state, NoteEventBuffer& out) {
    out.clear();

    if (!tc.playing || tc.ppqEnd <= tc.ppqStart)
        return;

    for (int lane = 0; lane < state.activeLaneCount; ++lane) {
        const auto& cfg = state.lanes[lane];
        if (!cfg.active)
            continue;
        if (cfg.cycle.steps <= 0 || cfg.cycle.subdivision <= 0)
            continue;

        std::array<bool, kMaxSteps> pattern{};
        buildLanePattern(cfg, state, lane, pattern);

        const double tempoScale = (cfg.tempoMultiplier > 0.0f) ? 1.0 / static_cast<double>(cfg.tempoMultiplier) : 1.0;
        const double sPpq = stepPpq(cfg.cycle) * tempoScale;
        auto additive = computeAdditiveCells(cfg);
        if (tempoScale != 1.0 && additive.count > 0) {
            for (int c = 0; c < additive.count; ++c)
                additive.cumPpq[c] *= tempoScale;
            additive.totalPpq *= tempoScale;
        }
        const bool isAdditive = additive.count > 0;
        const double cyclePpqLen = isAdditive ? additive.totalPpq : (cfg.cycle.steps * sPpq);

        double maxStepDur = sPpq;
        if (isAdditive) {
            for (int c = 0; c < additive.count; ++c)
                maxStepDur = std::max(maxStepDur, cfg.cellSizes[c] * sPpq);
        }
        double maxTimingShift = 0.0;
        maxTimingShift += static_cast<double>(std::max(0.0f, cfg.swingAmount)) * maxStepDur / 3.0;
        maxTimingShift += static_cast<double>(std::max(0.0f, cfg.syncopationOffset)) * maxStepDur / 3.0;
        if (tc.tempo > 0.0) {
            maxTimingShift += (static_cast<double>(std::max(0.0f, cfg.humanizeMs)) + 50.0) * tc.tempo / 60000.0;
            maxTimingShift += std::abs(static_cast<double>(cfg.timingOffsetMs)) * tc.tempo / 60000.0;
            maxTimingShift += 20.0 * tc.tempo / 60000.0;
        }

        int64_t firstStep, lastStep;
        if (isAdditive) {
            int64_t firstCycle = static_cast<int64_t>(std::floor((tc.ppqStart - maxTimingShift) / cyclePpqLen));
            int64_t lastCycle = static_cast<int64_t>(std::floor((tc.ppqEnd + maxTimingShift) / cyclePpqLen)) + 1;
            firstStep = firstCycle * additive.count;
            lastStep = lastCycle * additive.count;
        } else {
            firstStep = static_cast<int64_t>(std::floor((tc.ppqStart - maxTimingShift) / sPpq));
            lastStep = static_cast<int64_t>(std::ceil((tc.ppqEnd + maxTimingShift) / sPpq));
        }

        const bool hasPhraseGating = cfg.phraseLength > 0.0f;
        const double phraseLenPpq = static_cast<double>(cfg.phraseLength);
        const double phraseGapPpq = static_cast<double>(cfg.phraseGap);
        const double phraseCyclePpq = phraseLenPpq + phraseGapPpq;
        const double phraseOffPpq = static_cast<double>(cfg.phraseOffset);

        for (int64_t absStep = firstStep; absStep < lastStep; ++absStep) {
            double ppq;
            double stepDurPpq;
            if (isAdditive) {
                int localCell = static_cast<int>(((absStep % additive.count) + additive.count) % additive.count);
                int64_t cycleIdx =
                    (absStep >= 0) ? absStep / additive.count : (absStep - additive.count + 1) / additive.count;
                ppq = cycleIdx * cyclePpqLen + additive.cumPpq[localCell];
                stepDurPpq = cfg.cellSizes[localCell] * sPpq;
            } else {
                ppq = absStep * sPpq;
                stepDurPpq = sPpq;
            }

            if (ppq < tc.ppqStart - maxTimingShift || ppq >= tc.ppqEnd + maxTimingShift)
                continue;

            if (hasPhraseGating && phraseCyclePpq > 0.0) {
                double phrasePos = std::fmod(ppq - phraseOffPpq, phraseCyclePpq);
                if (phrasePos < 0.0)
                    phrasePos += phraseCyclePpq;
                if (phrasePos >= phraseLenPpq)
                    continue;
            }

            int stepsInCycle = isAdditive ? additive.count : cfg.cycle.steps;
            int64_t cycleStep = ((absStep % stepsInCycle) + stepsInCycle) % stepsInCycle;

            if (cfg.driftRate != 0.0f) {
                double barPos = ppq / 4.0;
                auto driftSteps = static_cast<int64_t>(std::floor(barPos * static_cast<double>(cfg.driftRate)));
                cycleStep = ((cycleStep + driftSteps) % stepsInCycle + stepsInCycle) % stepsInCycle;
            }

            EnvelopeMods mods = computeEnvelopeMods(cfg, state, ppq);

            bool isPatternStep = pattern[static_cast<size_t>(cycleStep)];
            bool isAnchor = cfg.constraints.anchorSteps.steps[static_cast<size_t>(cycleStep)] > 0.0f;
            bool mutatedToGhost = false;

            if (!shouldEmitStep(cfg, state, absStep, cycleStep, isPatternStep, isAnchor, mods, mutatedToGhost))
                continue;

            float vel = computeStepVelocity(cfg, state, absStep, cycleStep, mods, mutatedToGhost);

            ppq = applyTimingShifts(cfg, tc, state, ppq, stepDurPpq, absStep, cycleStep, mods.humanize);

            if (ppq < tc.ppqStart || ppq >= tc.ppqEnd)
                continue;

            NoteEvent ev{};
            ev.ppqPosition = ppq;
            ev.pitch = cfg.midiNote;
            ev.velocity = vel;
            double baseDuration = cfg.noteDuration > 0.0f ? static_cast<double>(cfg.noteDuration) : stepDurPpq * 0.5;
            ev.duration = baseDuration * static_cast<double>(std::clamp(mods.duration, 0.01f, 4.0f));
            ev.channel = (cfg.midiChannel >= 0) ? cfg.midiChannel : static_cast<int16_t>(lane);
            ev.laneIndex = static_cast<int16_t>(lane);

            out.push(ev);
        }
    }
}

} // namespace poly
