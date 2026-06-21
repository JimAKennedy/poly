#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/macro.h"
#include "poly/presets.h"

namespace {

void verifyPresetProducesOutput(const poly::GrooveState& state, const char* name) {
    poly::Engine engine;
    auto resolved = poly::resolveMacros(state);
    poly::NoteEventBuffer buf;

    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.playing = true;

    engine.renderRange(tc, resolved, buf);
    EXPECT_GT(buf.count, 0u) << name << " produced no events in first bar";
}

} // namespace

TEST(Presets, AllFactoryPresetsProduceOutput) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        auto info = poly::getFactoryPresetInfo(i);
        auto state = poly::makeFactoryPreset(i);
        verifyPresetProducesOutput(state, info.name);
    }
}

TEST(Presets, FactoryPresetInfoValid) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        auto info = poly::getFactoryPresetInfo(i);
        EXPECT_NE(info.name[0], '\0') << "Preset " << i << " has empty name";
        EXPECT_NE(info.description[0], '\0') << "Preset " << i << " has empty description";
    }
}

TEST(Presets, PresetsHaveValidLaneConfig) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        auto state = poly::makeFactoryPreset(i);
        auto info = poly::getFactoryPresetInfo(i);

        EXPECT_GE(state.activeLaneCount, 1) << info.name;
        EXPECT_LE(state.activeLaneCount, poly::kMaxLanes) << info.name;

        for (int lane = 0; lane < state.activeLaneCount; ++lane) {
            const auto& cfg = state.lanes[lane];
            EXPECT_GT(cfg.cycle.steps, 0) << info.name << " lane " << lane;
            EXPECT_GT(cfg.cycle.subdivision, 0) << info.name << " lane " << lane;
            EXPECT_GT(cfg.hitCount, 0) << info.name << " lane " << lane;
            EXPECT_LE(cfg.hitCount, cfg.cycle.steps) << info.name << " lane " << lane;
            EXPECT_GE(cfg.probability, 0.0f) << info.name << " lane " << lane;
            EXPECT_LE(cfg.probability, 1.0f) << info.name << " lane " << lane;
            EXPECT_GT(cfg.baseVelocity, 0) << info.name << " lane " << lane;
            EXPECT_LE(cfg.baseVelocity, 127) << info.name << " lane " << lane;
        }
    }
}

TEST(Presets, PresetsDeterministic) {
    poly::Engine engine;

    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        auto state = poly::makeFactoryPreset(i);
        auto resolved = poly::resolveMacros(state);

        poly::NoteEventBuffer buf1, buf2;
        poly::TransportContext tc{};
        tc.ppqStart = 0.0;
        tc.ppqEnd = 16.0;
        tc.tempo = 120.0;
        tc.playing = true;

        engine.renderRange(tc, resolved, buf1);
        engine.renderRange(tc, resolved, buf2);

        ASSERT_EQ(buf1.count, buf2.count) << "Preset " << i << " non-deterministic";
        for (size_t j = 0; j < buf1.count; ++j) {
            EXPECT_EQ(buf1.events[j].ppqPosition, buf2.events[j].ppqPosition)
                << "Preset " << i << " event " << j << " PPQ differs";
            EXPECT_EQ(buf1.events[j].velocity, buf2.events[j].velocity)
                << "Preset " << i << " event " << j << " velocity differs";
        }
    }
}

TEST(Presets, OutOfRangeReturnsDefault) {
    auto state = poly::makeFactoryPreset(-1);
    EXPECT_EQ(state.activeLaneCount, 4);

    auto state2 = poly::makeFactoryPreset(999);
    EXPECT_EQ(state2.activeLaneCount, 4);

    auto info = poly::getFactoryPresetInfo(-1);
    EXPECT_EQ(info.name[0], '\0');
}
