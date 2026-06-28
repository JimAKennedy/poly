#pragma once

#include <algorithm>
#include <cmath>

#include "poly/types.h"

namespace poly {

enum class SceneSelect : uint8_t { A, B, Morph };

// --- Scene Chain ---

static constexpr int kMaxChainEntries = 16;

enum class ChainMode : uint8_t {
    OneShot,
    Loop,
    PingPong,
};

struct SceneChainEntry {
    SceneSelect scene = SceneSelect::A;
    int bars = 4;
};

struct SceneChainConfig {
    std::array<SceneChainEntry, kMaxChainEntries> entries{};
    int entryCount = 0;
    ChainMode mode = ChainMode::Loop;
    bool enabled = false;
};

struct SceneChainState {
    int currentIndex = 0;
    int barsInCurrentEntry = 0;
    int direction = 1;
    int64_t lastBarNumber = -1;

    void reset() {
        currentIndex = 0;
        barsInCurrentEntry = 0;
        direction = 1;
        lastBarNumber = -1;
    }

    SceneSelect update(const SceneChainConfig& config, double ppqPosition) {
        if (config.entryCount <= 0)
            return SceneSelect::A;

        int64_t barNumber = static_cast<int64_t>(std::floor(ppqPosition / 4.0));

        if (lastBarNumber < 0) {
            lastBarNumber = barNumber;
            currentIndex = 0;
            barsInCurrentEntry = 0;
            return config.entries[0].scene;
        }

        while (barNumber > lastBarNumber) {
            ++lastBarNumber;
            ++barsInCurrentEntry;

            if (barsInCurrentEntry >= config.entries[static_cast<size_t>(currentIndex)].bars) {
                barsInCurrentEntry = 0;
                advance(config);
            }
        }

        int idx = std::clamp(currentIndex, 0, config.entryCount - 1);
        return config.entries[static_cast<size_t>(idx)].scene;
    }

private:
    void advance(const SceneChainConfig& config) {
        int next = currentIndex + direction;

        switch (config.mode) {
        case ChainMode::OneShot:
            if (next >= config.entryCount)
                next = config.entryCount - 1;
            else if (next < 0)
                next = 0;
            break;

        case ChainMode::Loop:
            next = next % config.entryCount;
            if (next < 0)
                next += config.entryCount;
            break;

        case ChainMode::PingPong:
            if (next >= config.entryCount) {
                direction = -1;
                next = config.entryCount - 2;
                if (next < 0)
                    next = 0;
            } else if (next < 0) {
                direction = 1;
                next = 1;
                if (next >= config.entryCount)
                    next = 0;
            }
            break;
        }

        currentIndex = next;
    }
};

// --- Scene State ---

struct SceneState {
    GrooveState sceneA{};
    GrooveState sceneB{};
    SceneSelect select = SceneSelect::A;
    float morphAmount = 0.0f;
    NoteMap noteMap{};
    SceneChainConfig chain{};
};

GrooveState interpolateGrooveState(const GrooveState& a, const GrooveState& b, float t);

} // namespace poly
