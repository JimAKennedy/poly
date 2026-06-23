#include "poly/engine.h"

#include <algorithm>
#include <cmath>

#include "poly/envelope.h"
#include "poly/euclidean.h"
#include "poly/rng.h"

namespace poly {

// PPQ duration of one step given a cycle's subdivision.
// subdivision=4 means quarter notes (1.0 PPQ each),
// subdivision=8 means eighths (0.5 PPQ), subdivision=16 means sixteenths (0.25 PPQ).
static double stepPpq(const Cycle& cycle) {
    return 4.0 / cycle.subdivision;
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

        const double sPpq = stepPpq(cfg.cycle);
        const auto additive = computeAdditiveCells(cfg);
        const bool isAdditive = additive.count > 0;
        const double cyclePpqLen = isAdditive ? additive.totalPpq : (cfg.cycle.steps * sPpq);

        int64_t firstStep, lastStep;
        if (isAdditive) {
            int64_t firstCycle = static_cast<int64_t>(std::floor(tc.ppqStart / cyclePpqLen));
            int64_t lastCycle = static_cast<int64_t>(std::floor(tc.ppqEnd / cyclePpqLen)) + 1;
            firstStep = firstCycle * additive.count;
            lastStep = lastCycle * additive.count;
        } else {
            firstStep = static_cast<int64_t>(std::ceil(tc.ppqStart / sPpq));
            lastStep = static_cast<int64_t>(std::ceil(tc.ppqEnd / sPpq));
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
                double basePpq = 4.0 / cfg.cycle.subdivision;
                stepDurPpq = cfg.cellSizes[localCell] * basePpq;
            } else {
                ppq = absStep * sPpq;
                stepDurPpq = sPpq;
            }

            // Floating point guard: must be within [ppqStart, ppqEnd)
            if (ppq < tc.ppqStart || ppq >= tc.ppqEnd)
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

            // Phase drift: rotate pattern lookup by driftRate steps per bar
            if (cfg.driftRate != 0.0f) {
                double barPos = ppq / 4.0;
                auto driftSteps = static_cast<int64_t>(std::floor(barPos * static_cast<double>(cfg.driftRate)));
                cycleStep = ((cycleStep + driftSteps) % stepsInCycle + stepsInCycle) % stepsInCycle;
            }

            // Envelope modulation: evaluate before pattern check (FillLikelihood needs it)
            float velMod = 1.0f;
            float probMod = 0.0f;
            float accentMod = 0.0f;
            float durationMod = 1.0f;
            float humanizeMod = 0.0f;
            float activationMod = 0.0f;
            float fillMod = 0.0f;

            auto applyEnvelope = [&](const Envelope& env) {
                double phase = computeEnvelopePhase(ppq, env.periodBars, env.phaseOffset);
                float value = evaluateShapeFull(env, static_cast<float>(phase));
                switch (env.target) {
                case EnvTarget::Velocity:
                    velMod *= (1.0f - env.depth * (1.0f - value));
                    break;
                case EnvTarget::Density:
                    probMod += env.depth * (value * 2.0f - 1.0f);
                    break;
                case EnvTarget::Probability:
                    probMod += env.depth * (value * 2.0f - 1.0f);
                    break;
                case EnvTarget::AccentBias:
                    accentMod += env.depth * (value * 2.0f - 1.0f);
                    break;
                case EnvTarget::NoteLength:
                    durationMod *= (1.0f - env.depth * (1.0f - value));
                    break;
                case EnvTarget::TimingLooseness:
                    humanizeMod += env.depth * value;
                    break;
                case EnvTarget::ActivationWeight:
                    activationMod += env.depth * (value * 2.0f - 1.0f);
                    break;
                case EnvTarget::FillLikelihood:
                    fillMod += env.depth * value;
                    break;
                }
            };

            for (int e = 0; e < cfg.envelopeCount; ++e) {
                const auto& ea = cfg.envelopes[e];
                if (!ea.active)
                    continue;
                applyEnvelope(ea.envelope);
            }

            for (int e = 0; e < state.globalEnvelopeCount; ++e) {
                applyEnvelope(state.globalEnvelopes[e]);
            }

            bool isPatternStep = pattern[static_cast<size_t>(cycleStep)];
            bool isAnchor = cfg.constraints.anchorSteps.steps[static_cast<size_t>(cycleStep)];

            bool mutatedToGhost = false;
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
                    if (fillMod <= 0.0f)
                        continue;
                    float fillProb = std::clamp(fillMod, 0.0f, 1.0f);
                    float fillRoll = deterministicRand(state.seed, cfg.id, absStep, 4);
                    if (fillRoll >= fillProb)
                        continue;
                }

                if (activationMod < 0.0f) {
                    float activationProb = std::clamp(1.0f + activationMod, 0.0f, 1.0f);
                    float actRoll = deterministicRand(state.seed, cfg.id, absStep, 5);
                    if (actRoll >= activationProb)
                        continue;
                }

                float effectiveProb = std::clamp(cfg.probability + probMod, 0.0f, 1.0f);
                float probRoll = deterministicRand(state.seed, cfg.id, absStep, 0);
                if (probRoll >= effectiveProb)
                    continue;
            }

            // Velocity with spread: channel 1 = velocity variation
            float velBase = cfg.baseVelocity / 127.0f;
            float velRand = deterministicRand(state.seed, cfg.id, absStep, 1);
            float spread = cfg.velocitySpread * (velRand * 2.0f - 1.0f);
            float vel = velBase + spread;

            // Accent mask boost gated by emphasis probability + AccentBias envelope
            if (cfg.accents.steps[static_cast<size_t>(cycleStep)]) {
                float effectiveEmphasis = std::clamp(cfg.emphasisProb + accentMod, 0.0f, 1.0f);
                float emphRoll = deterministicRand(state.seed, cfg.id, absStep, 2);
                if (emphRoll < effectiveEmphasis) {
                    vel += 0.15f;
                }
            }

            // Ghost floor: minimum velocity presence
            float ghostFloor = cfg.ghostFloor / 127.0f;
            if (vel < ghostFloor)
                vel = ghostFloor;

            // Apply velocity envelope modulation
            vel *= velMod;

            vel = std::clamp(vel, 0.0f, 1.0f);

            if (mutatedToGhost)
                vel = ghostFloor;

            // Swing: shift odd cycle-steps (1, 3, 5...) forward
            if (cfg.swingAmount > 0.0f && (cycleStep % 2) == 1) {
                ppq += cfg.swingAmount * stepDurPpq * (1.0 / 3.0);
            }

            // Per-step micro-timing map
            float stepTimingMs = cfg.microTimingMs[static_cast<size_t>(cycleStep)];
            if (stepTimingMs != 0.0f && tc.tempo > 0.0) {
                ppq += static_cast<double>(stepTimingMs) * tc.tempo / 60000.0;
            }

            // Humanize: bounded PPQ jitter from deterministic RNG
            // TimingLooseness envelope adds additional jitter range
            float effectiveHumanize = cfg.humanizeMs + humanizeMod * 10.0f;
            if (effectiveHumanize > 0.0f && tc.tempo > 0.0) {
                double jitterPpq = static_cast<double>(effectiveHumanize) * tc.tempo / 60000.0;
                float jitterRand = deterministicRand(state.seed, cfg.id, absStep, 3);
                ppq += jitterPpq * (jitterRand * 2.0f - 1.0f);
            }

            if (cfg.timingOffsetMs != 0.0f && tc.tempo > 0.0) {
                double offsetPpq = static_cast<double>(cfg.timingOffsetMs) * tc.tempo / 60000.0;
                ppq += offsetPpq;
                if (ppq < 0.0)
                    ppq = 0.0;
            }

            NoteEvent ev{};
            ev.ppqPosition = ppq;
            ev.pitch = cfg.midiNote;
            ev.velocity = vel;
            double baseDuration = cfg.noteDuration > 0.0f ? static_cast<double>(cfg.noteDuration) : stepDurPpq * 0.5;
            ev.duration = baseDuration * static_cast<double>(std::clamp(durationMod, 0.01f, 4.0f));
            ev.channel = static_cast<int16_t>(lane);

            out.push(ev);
        }
    }
}

} // namespace poly
