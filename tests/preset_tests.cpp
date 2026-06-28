#include <algorithm>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/macro.h"
#include "poly/presets.h"
#include "poly/state_io.h"

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

// --- MIDI Channel Routing Tests ---

TEST(MidiChannel, ExplicitChannelAssignment) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;
    state.lanes[0].id = 0;
    state.lanes[0].midiNote = 36;
    state.lanes[0].midiChannel = 9;
    state.lanes[0].cycle = {.steps = 4, .subdivision = 4};
    state.lanes[0].hitCount = 4;
    state.lanes[0].baseVelocity = 100;
    state.lanes[0].probability = 1.0f;
    state.lanes[0].active = true;

    poly::NoteEventBuffer buf;
    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.playing = true;

    engine.renderRange(tc, state, buf);
    ASSERT_GT(buf.count, 0u);
    for (size_t i = 0; i < buf.count; ++i) {
        EXPECT_EQ(buf.events[i].channel, 9) << "Event " << i << " should be on channel 9";
    }
}

TEST(MidiChannel, AutoChannelFallsBackToLaneIndex) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 3;
    state.seed = 42;
    for (int i = 0; i < 3; ++i) {
        state.lanes[i].id = i;
        state.lanes[i].midiNote = static_cast<int16_t>(36 + i);
        state.lanes[i].midiChannel = -1;
        state.lanes[i].cycle = {.steps = 4, .subdivision = 4};
        state.lanes[i].hitCount = 4;
        state.lanes[i].baseVelocity = 100;
        state.lanes[i].probability = 1.0f;
        state.lanes[i].active = true;
    }

    poly::NoteEventBuffer buf;
    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.playing = true;

    engine.renderRange(tc, state, buf);
    ASSERT_GT(buf.count, 0u);
    for (size_t i = 0; i < buf.count; ++i) {
        int16_t expectedChannel = -1;
        for (int lane = 0; lane < 3; ++lane) {
            if (buf.events[i].pitch == static_cast<int16_t>(36 + lane)) {
                expectedChannel = static_cast<int16_t>(lane);
                break;
            }
        }
        ASSERT_GE(expectedChannel, 0) << "Event " << i << " has unexpected pitch";
        EXPECT_EQ(buf.events[i].channel, expectedChannel)
            << "Event " << i << " should be on channel " << expectedChannel;
    }
}

TEST(MidiChannel, MultipleLanesShareChannel) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = 42;
    for (int i = 0; i < 2; ++i) {
        state.lanes[i].id = i;
        state.lanes[i].midiNote = static_cast<int16_t>(36 + i);
        state.lanes[i].midiChannel = 5;
        state.lanes[i].cycle = {.steps = 4, .subdivision = 4};
        state.lanes[i].hitCount = 4;
        state.lanes[i].baseVelocity = 100;
        state.lanes[i].probability = 1.0f;
        state.lanes[i].active = true;
    }

    poly::NoteEventBuffer buf;
    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.playing = true;

    engine.renderRange(tc, state, buf);
    ASSERT_GT(buf.count, 0u);
    for (size_t i = 0; i < buf.count; ++i) {
        EXPECT_EQ(buf.events[i].channel, 5) << "Event " << i << " should be on channel 5";
    }
}

TEST(MidiChannel, StateRoundTrip) {
    poly::GrooveState original{};
    original.activeLaneCount = 4;
    original.lanes[0].midiChannel = 9;
    original.lanes[1].midiChannel = -1;
    original.lanes[2].midiChannel = 0;
    original.lanes[3].midiChannel = 15;

    std::vector<uint8_t> data;
    auto writeFn = [&](const void* buf, size_t len) -> bool {
        auto* bytes = static_cast<const uint8_t*>(buf);
        data.insert(data.end(), bytes, bytes + len);
        return true;
    };

    ASSERT_TRUE(poly::writeGrooveState(writeFn, original));

    poly::GrooveState loaded{};
    size_t readPos = 0;
    auto readFn = [&](void* buf, size_t len) -> bool {
        if (readPos + len > data.size())
            return false;
        std::memcpy(buf, data.data() + readPos, len);
        readPos += len;
        return true;
    };

    ASSERT_TRUE(poly::readGrooveState(readFn, loaded));
    EXPECT_EQ(loaded.lanes[0].midiChannel, 9);
    EXPECT_EQ(loaded.lanes[1].midiChannel, -1);
    EXPECT_EQ(loaded.lanes[2].midiChannel, 0);
    EXPECT_EQ(loaded.lanes[3].midiChannel, 15);
}
