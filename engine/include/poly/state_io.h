#pragma once

#include <cstddef>
#include <cstdint>

#include "poly/scene.h"
#include "poly/types.h"

namespace poly {

static constexpr int32_t kCurrentStateVersion = 12;

// --- Internal body-only write (no version header, always writes latest body format) ---

template <typename WriteFn>
bool writeGrooveStateBody(WriteFn&& write, const GrooveState& state, int32_t bodyVersion = kCurrentStateVersion) {
    if (!write(&state.activeLaneCount, sizeof(state.activeLaneCount)))
        return false;
    if (!write(&state.seed, sizeof(state.seed)))
        return false;
    if (!write(&state.macros, sizeof(state.macros)))
        return false;

    for (int i = 0; i < kMaxLanes; ++i) {
        const auto& lane = state.lanes[static_cast<size_t>(i)];
        if (!write(&lane.id, sizeof(lane.id)))
            return false;
        auto role = static_cast<uint8_t>(lane.role);
        if (!write(&role, sizeof(role)))
            return false;
        if (!write(&lane.midiNote, sizeof(lane.midiNote)))
            return false;
        if (!write(&lane.cycle.steps, sizeof(lane.cycle.steps)))
            return false;
        if (!write(&lane.cycle.subdivision, sizeof(lane.cycle.subdivision)))
            return false;
        if (!write(&lane.hitCount, sizeof(lane.hitCount)))
            return false;
        if (!write(&lane.rotation, sizeof(lane.rotation)))
            return false;
        if (!write(&lane.probability, sizeof(lane.probability)))
            return false;
        if (!write(&lane.baseVelocity, sizeof(lane.baseVelocity)))
            return false;

        if (bodyVersion >= 12) {
            for (int s = 0; s < kMaxSteps; ++s) {
                if (!write(&lane.accents.steps[static_cast<size_t>(s)], sizeof(float)))
                    return false;
            }
        } else {
            uint64_t accentBits = 0;
            for (int s = 0; s < kMaxSteps; ++s) {
                if (lane.accents.steps[static_cast<size_t>(s)] > 0.0f)
                    accentBits |= (uint64_t{1} << s);
            }
            if (!write(&accentBits, sizeof(accentBits)))
                return false;
        }

        if (!write(&lane.emphasisProb, sizeof(lane.emphasisProb)))
            return false;
        if (!write(&lane.ghostFloor, sizeof(lane.ghostFloor)))
            return false;
        if (!write(&lane.velocitySpread, sizeof(lane.velocitySpread)))
            return false;
        if (!write(&lane.humanizeMs, sizeof(lane.humanizeMs)))
            return false;
        if (!write(&lane.swingAmount, sizeof(lane.swingAmount)))
            return false;
        if (!write(&lane.noteDuration, sizeof(lane.noteDuration)))
            return false;
        uint8_t active = lane.active ? 1 : 0;
        if (!write(&active, sizeof(active)))
            return false;
        if (!write(&lane.envelopeCount, sizeof(lane.envelopeCount)))
            return false;

        for (int e = 0; e < kMaxEnvelopesPerLane; ++e) {
            const auto& ea = lane.envelopes[static_cast<size_t>(e)];
            auto target = static_cast<uint8_t>(ea.envelope.target);
            if (!write(&target, sizeof(target)))
                return false;
            if (!write(&ea.envelope.periodBars, sizeof(ea.envelope.periodBars)))
                return false;
            auto shape = static_cast<uint8_t>(ea.envelope.shape);
            if (!write(&shape, sizeof(shape)))
                return false;
            if (!write(&ea.envelope.depth, sizeof(ea.envelope.depth)))
                return false;
            if (!write(&ea.envelope.phaseOffset, sizeof(ea.envelope.phaseOffset)))
                return false;
            if (!write(&ea.envelope.curvature, sizeof(ea.envelope.curvature)))
                return false;
            if (!write(&ea.envelope.stepCount, sizeof(ea.envelope.stepCount)))
                return false;
            for (int s = 0; s < kMaxStepListEntries; ++s) {
                if (!write(&ea.envelope.stepValues[static_cast<size_t>(s)], sizeof(float)))
                    return false;
            }
            uint8_t envActive = ea.active ? 1 : 0;
            if (!write(&envActive, sizeof(envActive)))
                return false;
        }

        if (bodyVersion >= 4) {
            uint64_t anchorBits = 0;
            for (int s = 0; s < kMaxSteps; ++s) {
                if (lane.constraints.anchorSteps.steps[static_cast<size_t>(s)] > 0.0f)
                    anchorBits |= (uint64_t{1} << s);
            }
            if (!write(&anchorBits, sizeof(anchorBits)))
                return false;
            uint8_t bbp = lane.constraints.backbeatProtect ? 1 : 0;
            if (!write(&bbp, sizeof(bbp)))
                return false;
            if (!write(&lane.constraints.densityMin, sizeof(lane.constraints.densityMin)))
                return false;
            if (!write(&lane.constraints.densityMax, sizeof(lane.constraints.densityMax)))
                return false;
        }

        if (bodyVersion >= 5) {
            if (!write(&lane.phraseLength, sizeof(lane.phraseLength)))
                return false;
            if (!write(&lane.phraseGap, sizeof(lane.phraseGap)))
                return false;
            if (!write(&lane.phraseOffset, sizeof(lane.phraseOffset)))
                return false;
        }

        if (bodyVersion >= 6) {
            if (!write(&lane.mutationRate, sizeof(lane.mutationRate)))
                return false;
        }

        if (bodyVersion >= 7) {
            if (!write(&lane.driftRate, sizeof(lane.driftRate)))
                return false;
        }

        if (bodyVersion >= 8) {
            if (!write(&lane.timingOffsetMs, sizeof(lane.timingOffsetMs)))
                return false;
        }

        if (bodyVersion >= 9) {
            if (!write(&lane.kotekanSourceLane, sizeof(lane.kotekanSourceLane)))
                return false;
        }

        if (bodyVersion >= 10) {
            if (!write(&lane.cellCount, sizeof(lane.cellCount)))
                return false;
            for (int s = 0; s < kMaxSteps; ++s) {
                if (!write(&lane.cellSizes[static_cast<size_t>(s)], sizeof(int)))
                    return false;
            }
            uint8_t tl = lane.timeline ? 1 : 0;
            if (!write(&tl, sizeof(tl)))
                return false;
            if (!write(&lane.fixedPatternLength, sizeof(lane.fixedPatternLength)))
                return false;
            uint64_t fpBits = 0;
            for (int s = 0; s < kMaxSteps; ++s) {
                if (lane.fixedPattern[static_cast<size_t>(s)])
                    fpBits |= (uint64_t{1} << s);
            }
            if (!write(&fpBits, sizeof(fpBits)))
                return false;
            for (int s = 0; s < kMaxSteps; ++s) {
                if (!write(&lane.microTimingMs[static_cast<size_t>(s)], sizeof(float)))
                    return false;
            }
        }
    }

    if (!write(&state.globalEnvelopeCount, sizeof(state.globalEnvelopeCount)))
        return false;
    for (int e = 0; e < kMaxGlobalEnvelopes; ++e) {
        const auto& env = state.globalEnvelopes[static_cast<size_t>(e)];
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
        if (!write(&env.curvature, sizeof(env.curvature)))
            return false;
        if (!write(&env.stepCount, sizeof(env.stepCount)))
            return false;
        for (int s = 0; s < kMaxStepListEntries; ++s) {
            if (!write(&env.stepValues[static_cast<size_t>(s)], sizeof(float)))
                return false;
        }
    }

    if (bodyVersion >= 4) {
        if (!write(&state.globalDensityCeiling, sizeof(state.globalDensityCeiling)))
            return false;
    }

    return true;
}

// --- Internal body-only read (version-aware, no version header) ---

template <typename ReadFn> bool readGrooveStateBody(ReadFn&& read, GrooveState& state, int32_t version) {
    if (!read(&state.activeLaneCount, sizeof(state.activeLaneCount)))
        return false;
    if (!read(&state.seed, sizeof(state.seed)))
        return false;
    if (!read(&state.macros, sizeof(state.macros)))
        return false;

    for (int i = 0; i < kMaxLanes; ++i) {
        auto& lane = state.lanes[static_cast<size_t>(i)];
        if (!read(&lane.id, sizeof(lane.id)))
            return false;
        uint8_t role = 0;
        if (!read(&role, sizeof(role)))
            return false;
        lane.role = static_cast<Role>(role);
        if (!read(&lane.midiNote, sizeof(lane.midiNote)))
            return false;
        if (!read(&lane.cycle.steps, sizeof(lane.cycle.steps)))
            return false;
        if (!read(&lane.cycle.subdivision, sizeof(lane.cycle.subdivision)))
            return false;
        if (!read(&lane.hitCount, sizeof(lane.hitCount)))
            return false;
        if (!read(&lane.rotation, sizeof(lane.rotation)))
            return false;
        if (!read(&lane.probability, sizeof(lane.probability)))
            return false;
        if (!read(&lane.baseVelocity, sizeof(lane.baseVelocity)))
            return false;

        if (version >= 12) {
            for (int s = 0; s < kMaxSteps; ++s) {
                if (!read(&lane.accents.steps[static_cast<size_t>(s)], sizeof(float)))
                    return false;
            }
        } else {
            uint64_t accentBits = 0;
            if (!read(&accentBits, sizeof(accentBits)))
                return false;
            for (int s = 0; s < kMaxSteps; ++s) {
                lane.accents.steps[static_cast<size_t>(s)] = (accentBits & (uint64_t{1} << s)) != 0 ? 1.0f : 0.0f;
            }
        }

        if (!read(&lane.emphasisProb, sizeof(lane.emphasisProb)))
            return false;
        if (!read(&lane.ghostFloor, sizeof(lane.ghostFloor)))
            return false;
        if (!read(&lane.velocitySpread, sizeof(lane.velocitySpread)))
            return false;
        if (!read(&lane.humanizeMs, sizeof(lane.humanizeMs)))
            return false;
        if (!read(&lane.swingAmount, sizeof(lane.swingAmount)))
            return false;
        if (!read(&lane.noteDuration, sizeof(lane.noteDuration)))
            return false;
        uint8_t active = 0;
        if (!read(&active, sizeof(active)))
            return false;
        lane.active = (active != 0);
        if (!read(&lane.envelopeCount, sizeof(lane.envelopeCount)))
            return false;

        for (int e = 0; e < kMaxEnvelopesPerLane; ++e) {
            auto& ea = lane.envelopes[static_cast<size_t>(e)];
            uint8_t target = 0;
            if (!read(&target, sizeof(target)))
                return false;
            ea.envelope.target = static_cast<EnvTarget>(target);
            if (!read(&ea.envelope.periodBars, sizeof(ea.envelope.periodBars)))
                return false;
            uint8_t shape = 0;
            if (!read(&shape, sizeof(shape)))
                return false;
            ea.envelope.shape = static_cast<Shape>(shape);
            if (!read(&ea.envelope.depth, sizeof(ea.envelope.depth)))
                return false;
            if (!read(&ea.envelope.phaseOffset, sizeof(ea.envelope.phaseOffset)))
                return false;
            if (version >= 2) {
                if (!read(&ea.envelope.curvature, sizeof(ea.envelope.curvature)))
                    return false;
                if (!read(&ea.envelope.stepCount, sizeof(ea.envelope.stepCount)))
                    return false;
                for (int s = 0; s < kMaxStepListEntries; ++s) {
                    if (!read(&ea.envelope.stepValues[static_cast<size_t>(s)], sizeof(float)))
                        return false;
                }
            }
            uint8_t envActive = 0;
            if (!read(&envActive, sizeof(envActive)))
                return false;
            ea.active = (envActive != 0);
        }

        if (version >= 4) {
            uint64_t anchorBits = 0;
            if (!read(&anchorBits, sizeof(anchorBits)))
                return false;
            for (int s = 0; s < kMaxSteps; ++s) {
                lane.constraints.anchorSteps.steps[static_cast<size_t>(s)] =
                    (anchorBits & (uint64_t{1} << s)) != 0 ? 1.0f : 0.0f;
            }
            uint8_t bbp = 0;
            if (!read(&bbp, sizeof(bbp)))
                return false;
            lane.constraints.backbeatProtect = (bbp != 0);
            if (!read(&lane.constraints.densityMin, sizeof(lane.constraints.densityMin)))
                return false;
            if (!read(&lane.constraints.densityMax, sizeof(lane.constraints.densityMax)))
                return false;
        }

        if (version >= 5) {
            if (!read(&lane.phraseLength, sizeof(lane.phraseLength)))
                return false;
            if (!read(&lane.phraseGap, sizeof(lane.phraseGap)))
                return false;
            if (!read(&lane.phraseOffset, sizeof(lane.phraseOffset)))
                return false;
        }

        if (version >= 6) {
            if (!read(&lane.mutationRate, sizeof(lane.mutationRate)))
                return false;
        }

        if (version >= 7) {
            if (!read(&lane.driftRate, sizeof(lane.driftRate)))
                return false;
        }

        if (version >= 8) {
            if (!read(&lane.timingOffsetMs, sizeof(lane.timingOffsetMs)))
                return false;
        }

        if (version >= 9) {
            if (!read(&lane.kotekanSourceLane, sizeof(lane.kotekanSourceLane)))
                return false;
        }

        if (version >= 10) {
            if (!read(&lane.cellCount, sizeof(lane.cellCount)))
                return false;
            for (int s = 0; s < kMaxSteps; ++s) {
                if (!read(&lane.cellSizes[static_cast<size_t>(s)], sizeof(int)))
                    return false;
            }
            uint8_t tl = 0;
            if (!read(&tl, sizeof(tl)))
                return false;
            lane.timeline = (tl != 0);
            if (!read(&lane.fixedPatternLength, sizeof(lane.fixedPatternLength)))
                return false;
            uint64_t fpBits = 0;
            if (!read(&fpBits, sizeof(fpBits)))
                return false;
            for (int s = 0; s < kMaxSteps; ++s) {
                lane.fixedPattern[static_cast<size_t>(s)] = (fpBits & (uint64_t{1} << s)) != 0;
            }
            for (int s = 0; s < kMaxSteps; ++s) {
                if (!read(&lane.microTimingMs[static_cast<size_t>(s)], sizeof(float)))
                    return false;
            }
        }
    }

    if (!read(&state.globalEnvelopeCount, sizeof(state.globalEnvelopeCount)))
        return false;
    for (int e = 0; e < kMaxGlobalEnvelopes; ++e) {
        auto& env = state.globalEnvelopes[static_cast<size_t>(e)];
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
    }

    if (version >= 4) {
        if (!read(&state.globalDensityCeiling, sizeof(state.globalDensityCeiling)))
            return false;
    }

    return true;
}

// --- Public: single GrooveState (for engine-layer tests) ---

template <typename WriteFn> bool writeGrooveState(WriteFn&& write, const GrooveState& state) {
    int32_t version = kCurrentStateVersion;
    if (!write(&version, sizeof(version)))
        return false;
    return writeGrooveStateBody(write, state);
}

template <typename ReadFn> bool readGrooveState(ReadFn&& read, GrooveState& state) {
    int32_t version = 0;
    if (!read(&version, sizeof(version)))
        return false;
    if (version < 1 || version > kCurrentStateVersion)
        return false;
    return readGrooveStateBody(read, state, version);
}

// --- Public: SceneState (v3 format, used by processor/controller) ---

template <typename WriteFn> bool writeSceneState(WriteFn&& write, const SceneState& scene) {
    int32_t version = kCurrentStateVersion;
    if (!write(&version, sizeof(version)))
        return false;
    if (!writeGrooveStateBody(write, scene.sceneA))
        return false;
    if (!writeGrooveStateBody(write, scene.sceneB))
        return false;
    auto select = static_cast<uint8_t>(scene.select);
    if (!write(&select, sizeof(select)))
        return false;
    if (!write(&scene.morphAmount, sizeof(scene.morphAmount)))
        return false;
    for (int i = 0; i < 128; ++i) {
        if (!write(&scene.noteMap.map[static_cast<size_t>(i)], sizeof(int16_t)))
            return false;
    }
    return true;
}

template <typename ReadFn> bool readSceneState(ReadFn&& read, SceneState& scene) {
    int32_t version = 0;
    if (!read(&version, sizeof(version)))
        return false;
    if (version < 1 || version > kCurrentStateVersion)
        return false;

    if (version <= 2) {
        if (!readGrooveStateBody(read, scene.sceneA, version))
            return false;
        scene.sceneB = scene.sceneA;
        scene.select = SceneSelect::A;
        scene.morphAmount = 0.0f;
    } else {
        int32_t bodyVersion = (version == 3) ? 2 : version;
        if (!readGrooveStateBody(read, scene.sceneA, bodyVersion))
            return false;
        if (!readGrooveStateBody(read, scene.sceneB, bodyVersion))
            return false;
        uint8_t select = 0;
        if (!read(&select, sizeof(select)))
            return false;
        scene.select = static_cast<SceneSelect>(select);
        if (!read(&scene.morphAmount, sizeof(scene.morphAmount)))
            return false;
    }
    if (version >= 11) {
        for (int i = 0; i < 128; ++i) {
            if (!read(&scene.noteMap.map[static_cast<size_t>(i)], sizeof(int16_t)))
                return false;
        }
    } else {
        scene.noteMap.reset();
    }
    return true;
}

} // namespace poly
