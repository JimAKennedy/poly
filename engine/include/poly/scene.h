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
    int64_t startBarNumber = -1;

    void reset() {
        currentIndex = 0;
        barsInCurrentEntry = 0;
        direction = 1;
        lastBarNumber = -1;
        startBarNumber = -1;
    }

    SceneSelect update(const SceneChainConfig& config, double ppqPosition) {
        if (config.entryCount <= 0)
            return SceneSelect::A;

        int64_t barNumber = static_cast<int64_t>(std::floor(ppqPosition / 4.0));

        if (startBarNumber < 0) {
            startBarNumber = barNumber;
            lastBarNumber = barNumber;
            currentIndex = 0;
            barsInCurrentEntry = 0;
            return config.entries[0].scene;
        }

        if (barNumber > lastBarNumber) {
            lastBarNumber = barNumber;
            resolveAbsolutePosition(config, barNumber - startBarNumber);
        }

        int idx = std::clamp(currentIndex, 0, config.entryCount - 1);
        return config.entries[static_cast<size_t>(idx)].scene;
    }

private:
    void resolveAbsolutePosition(const SceneChainConfig& config, int64_t totalBarsElapsed) {
        int totalCycleBars = 0;
        for (int i = 0; i < config.entryCount; ++i)
            totalCycleBars += std::max(1, config.entries[i].bars);

        switch (config.mode) {
        case ChainMode::Loop: {
            int64_t pos = totalBarsElapsed % totalCycleBars;
            if (pos < 0)
                pos += totalCycleBars;
            findEntryAtPosition(config, pos);
            break;
        }
        case ChainMode::OneShot: {
            if (totalBarsElapsed >= totalCycleBars) {
                currentIndex = config.entryCount - 1;
                barsInCurrentEntry = std::max(1, config.entries[currentIndex].bars) - 1;
            } else {
                findEntryAtPosition(config, totalBarsElapsed);
            }
            break;
        }
        case ChainMode::PingPong: {
            if (config.entryCount == 1) {
                currentIndex = 0;
                barsInCurrentEntry = 0;
                break;
            }
            int pingPongPeriod = totalCycleBars * 2 - std::max(1, config.entries[0].bars) -
                                 std::max(1, config.entries[config.entryCount - 1].bars);
            if (pingPongPeriod <= 0) {
                currentIndex = 0;
                barsInCurrentEntry = 0;
                break;
            }
            int64_t pos = totalBarsElapsed % pingPongPeriod;
            if (pos < 0)
                pos += pingPongPeriod;
            if (pos < totalCycleBars) {
                findEntryAtPosition(config, pos);
                direction = 1;
            } else {
                int64_t reversePos = pos - totalCycleBars;
                int64_t cumulative = 0;
                for (int i = config.entryCount - 2; i >= 1; --i) {
                    int entryBars = std::max(1, config.entries[i].bars);
                    if (cumulative + entryBars > reversePos) {
                        currentIndex = i;
                        barsInCurrentEntry = static_cast<int>(reversePos - cumulative);
                        direction = -1;
                        return;
                    }
                    cumulative += entryBars;
                }
                currentIndex = 1;
                barsInCurrentEntry = 0;
                direction = -1;
            }
            break;
        }
        }
    }

    void findEntryAtPosition(const SceneChainConfig& config, int64_t posInCycle) {
        int64_t cumulative = 0;
        for (int i = 0; i < config.entryCount; ++i) {
            int entryBars = std::max(1, config.entries[i].bars);
            if (cumulative + entryBars > posInCycle) {
                currentIndex = i;
                barsInCurrentEntry = static_cast<int>(posInCycle - cumulative);
                return;
            }
            cumulative += entryBars;
        }
        currentIndex = config.entryCount - 1;
        barsInCurrentEntry = 0;
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
