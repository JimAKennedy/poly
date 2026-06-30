#pragma once

#include <cstddef>
#include <cstdint>

#include "poly/scene.h"
#include "poly/types.h"

namespace poly {

static constexpr int32_t kCurrentStateVersion = 15;

// --- Envelope serialization helpers ---

template <typename WriteFn> [[nodiscard]] bool writeEnvelope(WriteFn&& write, const Envelope& env) {
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
    return true;
}

template <typename ReadFn> [[nodiscard]] bool readEnvelope(ReadFn&& read, Envelope& env, int32_t version) {
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
    return true;
}

} // namespace poly
#include "poly/state_io_read_lane.h"
#include "poly/state_io_write_lane.h"
namespace poly {

template <typename WriteFn>
[[nodiscard]] bool writeGrooveStateBody(WriteFn&& write, const GrooveState& state,
                                        int32_t bodyVersion = kCurrentStateVersion) {
    if (!write(&state.activeLaneCount, sizeof(state.activeLaneCount)))
        return false;
    if (!write(&state.seed, sizeof(state.seed)))
        return false;
    if (!write(&state.macros, sizeof(state.macros)))
        return false;

    for (int i = 0; i < kMaxLanes; ++i) {
        if (!writeLaneConfig(write, state.lanes[static_cast<size_t>(i)], bodyVersion))
            return false;
    }

    if (!write(&state.globalEnvelopeCount, sizeof(state.globalEnvelopeCount)))
        return false;
    for (int e = 0; e < kMaxGlobalEnvelopes; ++e) {
        if (!writeEnvelope(write, state.globalEnvelopes[static_cast<size_t>(e)]))
            return false;
    }

    if (bodyVersion >= 4) {
        if (!write(&state.globalDensityCeiling, sizeof(state.globalDensityCeiling)))
            return false;
    }

    return true;
}

// --- Internal body-only read (version-aware, no version header) ---

template <typename ReadFn> [[nodiscard]] bool readGrooveStateBody(ReadFn&& read, GrooveState& state, int32_t version) {
    if (!read(&state.activeLaneCount, sizeof(state.activeLaneCount)))
        return false;
    if (!read(&state.seed, sizeof(state.seed)))
        return false;
    if (!read(&state.macros, sizeof(state.macros)))
        return false;

    for (int i = 0; i < kMaxLanes; ++i) {
        if (!readLaneConfig(read, state.lanes[static_cast<size_t>(i)], version))
            return false;
    }

    if (!read(&state.globalEnvelopeCount, sizeof(state.globalEnvelopeCount)))
        return false;
    for (int e = 0; e < kMaxGlobalEnvelopes; ++e) {
        if (!readEnvelope(read, state.globalEnvelopes[static_cast<size_t>(e)], version))
            return false;
    }

    if (version >= 4) {
        if (!read(&state.globalDensityCeiling, sizeof(state.globalDensityCeiling)))
            return false;
    }

    return true;
}

// --- Public: single GrooveState (for engine-layer tests) ---

template <typename WriteFn> [[nodiscard]] bool writeGrooveState(WriteFn&& write, const GrooveState& state) {
    int32_t version = kCurrentStateVersion;
    if (!write(&version, sizeof(version)))
        return false;
    return writeGrooveStateBody(write, state);
}

template <typename ReadFn> [[nodiscard]] bool readGrooveState(ReadFn&& read, GrooveState& state) {
    int32_t version = 0;
    if (!read(&version, sizeof(version)))
        return false;
    if (version < 1 || version > kCurrentStateVersion)
        return false;
    return readGrooveStateBody(read, state, version);
}

// --- Public: SceneState (v3 format, used by processor/controller) ---

template <typename WriteFn> [[nodiscard]] bool writeSceneState(WriteFn&& write, const SceneState& scene) {
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

    uint8_t chainEnabled = scene.chain.enabled ? 1 : 0;
    if (!write(&chainEnabled, sizeof(chainEnabled)))
        return false;
    if (!write(&scene.chain.entryCount, sizeof(scene.chain.entryCount)))
        return false;
    auto chainMode = static_cast<uint8_t>(scene.chain.mode);
    if (!write(&chainMode, sizeof(chainMode)))
        return false;
    for (int i = 0; i < kMaxChainEntries; ++i) {
        auto entryScene = static_cast<uint8_t>(scene.chain.entries[static_cast<size_t>(i)].scene);
        if (!write(&entryScene, sizeof(entryScene)))
            return false;
        if (!write(&scene.chain.entries[static_cast<size_t>(i)].bars, sizeof(int)))
            return false;
    }

    return true;
}

template <typename ReadFn> [[nodiscard]] bool readSceneState(ReadFn&& read, SceneState& scene) {
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

    if (version >= 13) {
        uint8_t chainEnabled = 0;
        if (!read(&chainEnabled, sizeof(chainEnabled)))
            return false;
        scene.chain.enabled = (chainEnabled != 0);
        if (!read(&scene.chain.entryCount, sizeof(scene.chain.entryCount)))
            return false;
        uint8_t chainMode = 0;
        if (!read(&chainMode, sizeof(chainMode)))
            return false;
        scene.chain.mode = static_cast<ChainMode>(chainMode);
        for (int i = 0; i < kMaxChainEntries; ++i) {
            uint8_t entryScene = 0;
            if (!read(&entryScene, sizeof(entryScene)))
                return false;
            scene.chain.entries[static_cast<size_t>(i)].scene = static_cast<SceneSelect>(entryScene);
            if (!read(&scene.chain.entries[static_cast<size_t>(i)].bars, sizeof(int)))
                return false;
        }
    } else {
        scene.chain = SceneChainConfig{};
    }

    return true;
}

} // namespace poly
