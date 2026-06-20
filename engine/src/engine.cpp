#include "poly/engine.h"
#include "poly/envelope.h"
#include "poly/euclidean.h"
#include "poly/rng.h"

#include <algorithm>
#include <cmath>

namespace poly {

// PPQ duration of one step given a cycle's subdivision.
// subdivision=4 means quarter notes (1.0 PPQ each),
// subdivision=8 means eighths (0.5 PPQ), subdivision=16 means sixteenths (0.25 PPQ).
static double stepPpq(const Cycle& cycle) {
    return 4.0 / cycle.subdivision;
}

static double cyclePpq(const Cycle& cycle) {
    return stepPpq(cycle) * cycle.steps;
}

void Engine::renderRange(const TransportContext& tc,
                         const GrooveState& state,
                         NoteEventBuffer& out) {
    out.clear();

    if (!tc.playing || tc.ppqEnd <= tc.ppqStart) return;

    for (int lane = 0; lane < state.activeLaneCount; ++lane) {
        const auto& cfg = state.lanes[lane];
        if (!cfg.active) continue;
        if (cfg.cycle.steps <= 0 || cfg.cycle.subdivision <= 0) continue;

        std::array<bool, kMaxSteps> pattern{};
        euclidean(cfg.hitCount, cfg.cycle.steps, cfg.rotation, pattern);

        const double sPpq = stepPpq(cfg.cycle);
        const double cPpq = cyclePpq(cfg.cycle);

        // Find the range of absolute step indices that could fall in [ppqStart, ppqEnd).
        // Step i occurs at PPQ = i * sPpq. We need i such that i*sPpq >= ppqStart
        // and i*sPpq < ppqEnd.
        int64_t firstStep = static_cast<int64_t>(std::ceil(tc.ppqStart / sPpq));
        int64_t lastStep  = static_cast<int64_t>(std::ceil(tc.ppqEnd / sPpq));

        for (int64_t absStep = firstStep; absStep < lastStep; ++absStep) {
            double ppq = absStep * sPpq;

            // Floating point guard: must be within [ppqStart, ppqEnd)
            if (ppq < tc.ppqStart || ppq >= tc.ppqEnd) continue;

            // Map to position within the cycle
            int stepsInCycle = cfg.cycle.steps;
            int64_t cycleStep = ((absStep % stepsInCycle) + stepsInCycle) % stepsInCycle;

            if (!pattern[static_cast<size_t>(cycleStep)]) continue;

            // Envelope modulation: evaluate all active envelopes at this PPQ
            float velMod = 1.0f;
            float probMod = 0.0f;

            for (int e = 0; e < cfg.envelopeCount; ++e) {
                const auto& ea = cfg.envelopes[e];
                if (!ea.active) continue;
                const auto& env = ea.envelope;
                double phase = computeEnvelopePhase(ppq, env.periodBars, env.phaseOffset);
                float value = evaluateShape(env.shape, static_cast<float>(phase));
                switch (env.target) {
                case EnvTarget::Velocity:
                    velMod *= (1.0f - env.depth * (1.0f - value));
                    break;
                case EnvTarget::Density:
                    probMod += env.depth * (value * 2.0f - 1.0f);
                    break;
                default:
                    break;
                }
            }

            for (int e = 0; e < state.globalEnvelopeCount; ++e) {
                const auto& env = state.globalEnvelopes[e];
                double phase = computeEnvelopePhase(ppq, env.periodBars, env.phaseOffset);
                float value = evaluateShape(env.shape, static_cast<float>(phase));
                switch (env.target) {
                case EnvTarget::Velocity:
                    velMod *= (1.0f - env.depth * (1.0f - value));
                    break;
                case EnvTarget::Density:
                    probMod += env.depth * (value * 2.0f - 1.0f);
                    break;
                default:
                    break;
                }
            }

            // Probability gate with density envelope modulation
            float effectiveProb = std::clamp(cfg.probability + probMod, 0.0f, 1.0f);
            float probRoll = deterministicRand(state.seed, cfg.id, absStep, 0);
            if (probRoll >= effectiveProb) continue;

            // Velocity with spread: channel 1 = velocity variation
            float velBase = cfg.baseVelocity / 127.0f;
            float velRand = deterministicRand(state.seed, cfg.id, absStep, 1);
            float spread = cfg.velocitySpread * (velRand * 2.0f - 1.0f);
            float vel = velBase + spread;

            // Accent mask boost gated by emphasis probability (channel 2)
            if (cfg.accents.steps[static_cast<size_t>(cycleStep)]) {
                float emphRoll = deterministicRand(state.seed, cfg.id, absStep, 2);
                if (emphRoll < cfg.emphasisProb) {
                    vel += 0.15f;
                }
            }

            // Ghost floor: minimum velocity presence
            float ghostFloor = cfg.ghostFloor / 127.0f;
            if (vel < ghostFloor) vel = ghostFloor;

            // Apply velocity envelope modulation
            vel *= velMod;

            vel = std::clamp(vel, 0.0f, 1.0f);

            NoteEvent ev{};
            ev.ppqPosition = ppq;
            ev.pitch = cfg.midiNote;
            ev.velocity = vel;
            ev.duration = sPpq * 0.5;
            ev.channel = 0;

            out.push(ev);
        }
    }
}

} // namespace poly
