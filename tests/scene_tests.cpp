#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/scene.h"
#include "poly/state_io.h"

// --- Interpolation tests ---

TEST(Scene, InterpolateAtZero_ReturnsA) {
    poly::GrooveState a{}, b{};
    a.macros.complexity = 0.2f;
    a.macros.density = 0.8f;
    a.lanes[0].probability = 0.9f;
    a.lanes[0].baseVelocity = 100;
    a.lanes[0].role = poly::Role::AnchorPulse;
    a.lanes[0].midiNote = 36;

    b.macros.complexity = 0.8f;
    b.macros.density = 0.2f;
    b.lanes[0].probability = 0.3f;
    b.lanes[0].baseVelocity = 50;
    b.lanes[0].role = poly::Role::Ghost;
    b.lanes[0].midiNote = 45;

    auto result = poly::interpolateGrooveState(a, b, 0.0f);

    EXPECT_FLOAT_EQ(result.macros.complexity, 0.2f);
    EXPECT_FLOAT_EQ(result.macros.density, 0.8f);
    EXPECT_FLOAT_EQ(result.lanes[0].probability, 0.9f);
    EXPECT_EQ(result.lanes[0].baseVelocity, 100);
    EXPECT_EQ(static_cast<uint8_t>(result.lanes[0].role), static_cast<uint8_t>(poly::Role::AnchorPulse));
    EXPECT_EQ(result.lanes[0].midiNote, 36);
}

TEST(Scene, InterpolateAtOne_ReturnsB) {
    poly::GrooveState a{}, b{};
    a.macros.complexity = 0.2f;
    a.lanes[0].probability = 0.9f;
    a.lanes[0].baseVelocity = 100;

    b.macros.complexity = 0.8f;
    b.lanes[0].probability = 0.3f;
    b.lanes[0].baseVelocity = 50;

    auto result = poly::interpolateGrooveState(a, b, 1.0f);

    EXPECT_FLOAT_EQ(result.macros.complexity, 0.8f);
    EXPECT_FLOAT_EQ(result.lanes[0].probability, 0.3f);
    EXPECT_EQ(result.lanes[0].baseVelocity, 50);
}

TEST(Scene, InterpolateAtHalf_LerpsNumeric) {
    poly::GrooveState a{}, b{};
    a.macros.complexity = 0.0f;
    a.lanes[0].probability = 0.0f;
    a.lanes[0].baseVelocity = 0;

    b.macros.complexity = 1.0f;
    b.lanes[0].probability = 1.0f;
    b.lanes[0].baseVelocity = 100;

    auto result = poly::interpolateGrooveState(a, b, 0.5f);

    EXPECT_FLOAT_EQ(result.macros.complexity, 0.5f);
    EXPECT_FLOAT_EQ(result.lanes[0].probability, 0.5f);
    EXPECT_EQ(result.lanes[0].baseVelocity, 50);
}

TEST(Scene, DiscreteFieldsSnapAtHalf) {
    poly::GrooveState a{}, b{};
    a.lanes[0].role = poly::Role::AnchorPulse;
    a.lanes[0].midiNote = 36;
    a.lanes[0].cycle = {4, 4};

    b.lanes[0].role = poly::Role::Ghost;
    b.lanes[0].midiNote = 45;
    b.lanes[0].cycle = {5, 16};

    auto r1 = poly::interpolateGrooveState(a, b, 0.49f);
    EXPECT_EQ(static_cast<uint8_t>(r1.lanes[0].role), static_cast<uint8_t>(poly::Role::AnchorPulse));
    EXPECT_EQ(r1.lanes[0].midiNote, 36);
    EXPECT_EQ(r1.lanes[0].cycle.steps, 4);

    auto r2 = poly::interpolateGrooveState(a, b, 0.5f);
    EXPECT_EQ(static_cast<uint8_t>(r2.lanes[0].role), static_cast<uint8_t>(poly::Role::Ghost));
    EXPECT_EQ(r2.lanes[0].midiNote, 45);
    EXPECT_EQ(r2.lanes[0].cycle.steps, 5);
}

TEST(Scene, InterpolateEnvelopes) {
    poly::GrooveState a{}, b{};
    a.lanes[0].envelopeCount = 1;
    a.lanes[0].envelopes[0].active = true;
    a.lanes[0].envelopes[0].envelope.depth = 0.0f;
    a.lanes[0].envelopes[0].envelope.periodBars = 4.0f;

    b.lanes[0].envelopeCount = 1;
    b.lanes[0].envelopes[0].active = true;
    b.lanes[0].envelopes[0].envelope.depth = 1.0f;
    b.lanes[0].envelopes[0].envelope.periodBars = 8.0f;

    auto result = poly::interpolateGrooveState(a, b, 0.5f);
    EXPECT_FLOAT_EQ(result.lanes[0].envelopes[0].envelope.depth, 0.5f);
    EXPECT_FLOAT_EQ(result.lanes[0].envelopes[0].envelope.periodBars, 6.0f);
}

TEST(Scene, InterpolateSeedSnaps) {
    poly::GrooveState a{}, b{};
    a.seed = 100;
    b.seed = 200;

    auto r1 = poly::interpolateGrooveState(a, b, 0.49f);
    EXPECT_EQ(r1.seed, 100u);

    auto r2 = poly::interpolateGrooveState(a, b, 0.5f);
    EXPECT_EQ(r2.seed, 200u);
}

TEST(Scene, InterpolateActiveLaneCount) {
    poly::GrooveState a{}, b{};
    a.activeLaneCount = 2;
    b.activeLaneCount = 8;

    auto result = poly::interpolateGrooveState(a, b, 0.5f);
    EXPECT_EQ(result.activeLaneCount, 5);
}

// --- Scene serialization tests ---

TEST(SceneIO, RoundTrip) {
    poly::SceneState original{};
    original.sceneA.activeLaneCount = 4;
    original.sceneA.macros.complexity = 0.7f;
    original.sceneA.lanes[0].probability = 0.9f;
    original.sceneA.lanes[0].baseVelocity = 110;
    original.sceneA.lanes[0].envelopeCount = 1;
    original.sceneA.lanes[0].envelopes[0].active = true;
    original.sceneA.lanes[0].envelopes[0].envelope.target = poly::EnvTarget::Velocity;
    original.sceneA.lanes[0].envelopes[0].envelope.depth = 0.8f;

    original.sceneB.activeLaneCount = 6;
    original.sceneB.macros.complexity = 0.3f;
    original.sceneB.lanes[0].probability = 0.4f;
    original.sceneB.lanes[0].baseVelocity = 60;

    original.select = poly::SceneSelect::Morph;
    original.morphAmount = 0.65f;

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };
    ASSERT_TRUE(poly::writeSceneState(write, original));

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };

    poly::SceneState restored{};
    ASSERT_TRUE(poly::readSceneState(read, restored));

    EXPECT_EQ(restored.sceneA.activeLaneCount, 4);
    EXPECT_FLOAT_EQ(restored.sceneA.macros.complexity, 0.7f);
    EXPECT_FLOAT_EQ(restored.sceneA.lanes[0].probability, 0.9f);
    EXPECT_EQ(restored.sceneA.lanes[0].baseVelocity, 110);
    EXPECT_FLOAT_EQ(restored.sceneA.lanes[0].envelopes[0].envelope.depth, 0.8f);

    EXPECT_EQ(restored.sceneB.activeLaneCount, 6);
    EXPECT_FLOAT_EQ(restored.sceneB.macros.complexity, 0.3f);
    EXPECT_FLOAT_EQ(restored.sceneB.lanes[0].probability, 0.4f);
    EXPECT_EQ(restored.sceneB.lanes[0].baseVelocity, 60);

    EXPECT_EQ(static_cast<uint8_t>(restored.select), static_cast<uint8_t>(poly::SceneSelect::Morph));
    EXPECT_FLOAT_EQ(restored.morphAmount, 0.65f);
}

TEST(SceneIO, V2BackwardsCompat) {
    poly::GrooveState gs{};
    gs.activeLaneCount = 3;
    gs.macros.density = 0.42f;
    gs.lanes[0].probability = 0.77f;
    gs.seed = 42;

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };
    int32_t v2 = 2;
    ASSERT_TRUE(write(&v2, sizeof(v2)));
    ASSERT_TRUE(poly::writeGrooveStateBody(write, gs, 2));

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };

    poly::SceneState scene{};
    ASSERT_TRUE(poly::readSceneState(read, scene));

    EXPECT_EQ(scene.sceneA.activeLaneCount, 3);
    EXPECT_FLOAT_EQ(scene.sceneA.macros.density, 0.42f);
    EXPECT_FLOAT_EQ(scene.sceneA.lanes[0].probability, 0.77f);
    EXPECT_EQ(scene.sceneA.seed, 42u);

    EXPECT_EQ(scene.sceneB.activeLaneCount, 3);
    EXPECT_FLOAT_EQ(scene.sceneB.macros.density, 0.42f);
    EXPECT_FLOAT_EQ(scene.sceneB.lanes[0].probability, 0.77f);

    EXPECT_EQ(static_cast<uint8_t>(scene.select), static_cast<uint8_t>(poly::SceneSelect::A));
    EXPECT_FLOAT_EQ(scene.morphAmount, 0.0f);
}

// --- Scene Chain tests ---

TEST(SceneChain, AdvancesAtBarBoundary) {
    poly::SceneChainConfig config{};
    config.entryCount = 2;
    config.entries[0] = {poly::SceneSelect::A, 2};
    config.entries[1] = {poly::SceneSelect::B, 2};
    config.mode = poly::ChainMode::Loop;
    config.enabled = true;

    poly::SceneChainState state{};

    EXPECT_EQ(state.update(config, 0.0), poly::SceneSelect::A);
    EXPECT_EQ(state.update(config, 4.0), poly::SceneSelect::A);
    EXPECT_EQ(state.update(config, 8.0), poly::SceneSelect::B);
    EXPECT_EQ(state.update(config, 12.0), poly::SceneSelect::B);
    EXPECT_EQ(state.update(config, 16.0), poly::SceneSelect::A);
}

TEST(SceneChain, OneShotStopsAtEnd) {
    poly::SceneChainConfig config{};
    config.entryCount = 2;
    config.entries[0] = {poly::SceneSelect::A, 1};
    config.entries[1] = {poly::SceneSelect::B, 1};
    config.mode = poly::ChainMode::OneShot;

    poly::SceneChainState state{};

    EXPECT_EQ(state.update(config, 0.0), poly::SceneSelect::A);
    EXPECT_EQ(state.update(config, 4.0), poly::SceneSelect::B);
    EXPECT_EQ(state.update(config, 8.0), poly::SceneSelect::B);
    EXPECT_EQ(state.update(config, 12.0), poly::SceneSelect::B);
}

TEST(SceneChain, PingPongReversesDirection) {
    poly::SceneChainConfig config{};
    config.entryCount = 3;
    config.entries[0] = {poly::SceneSelect::A, 1};
    config.entries[1] = {poly::SceneSelect::Morph, 1};
    config.entries[2] = {poly::SceneSelect::B, 1};
    config.mode = poly::ChainMode::PingPong;

    poly::SceneChainState state{};

    EXPECT_EQ(state.update(config, 0.0), poly::SceneSelect::A);
    EXPECT_EQ(state.update(config, 4.0), poly::SceneSelect::Morph);
    EXPECT_EQ(state.update(config, 8.0), poly::SceneSelect::B);
    EXPECT_EQ(state.update(config, 12.0), poly::SceneSelect::Morph);
    EXPECT_EQ(state.update(config, 16.0), poly::SceneSelect::A);
    EXPECT_EQ(state.update(config, 20.0), poly::SceneSelect::Morph);
}

TEST(SceneChain, ResetClearsState) {
    poly::SceneChainConfig config{};
    config.entryCount = 2;
    config.entries[0] = {poly::SceneSelect::A, 1};
    config.entries[1] = {poly::SceneSelect::B, 1};
    config.mode = poly::ChainMode::Loop;

    poly::SceneChainState state{};
    state.update(config, 0.0);
    state.update(config, 4.0);
    EXPECT_EQ(state.update(config, 8.0), poly::SceneSelect::A);

    state.reset();
    EXPECT_EQ(state.update(config, 0.0), poly::SceneSelect::A);
}

TEST(SceneChain, SingleEntryStaysFixed) {
    poly::SceneChainConfig config{};
    config.entryCount = 1;
    config.entries[0] = {poly::SceneSelect::B, 4};
    config.mode = poly::ChainMode::Loop;

    poly::SceneChainState state{};

    EXPECT_EQ(state.update(config, 0.0), poly::SceneSelect::B);
    EXPECT_EQ(state.update(config, 16.0), poly::SceneSelect::B);
    EXPECT_EQ(state.update(config, 32.0), poly::SceneSelect::B);
}

TEST(SceneChain, EmptyChainDefaultsToA) {
    poly::SceneChainConfig config{};
    config.entryCount = 0;

    poly::SceneChainState state{};
    EXPECT_EQ(state.update(config, 0.0), poly::SceneSelect::A);
}

TEST(SceneChain, MultiBarEntries) {
    poly::SceneChainConfig config{};
    config.entryCount = 2;
    config.entries[0] = {poly::SceneSelect::A, 4};
    config.entries[1] = {poly::SceneSelect::B, 2};
    config.mode = poly::ChainMode::Loop;

    poly::SceneChainState state{};

    // A for 4 bars (0-15 PPQ)
    EXPECT_EQ(state.update(config, 0.0), poly::SceneSelect::A);
    EXPECT_EQ(state.update(config, 4.0), poly::SceneSelect::A);
    EXPECT_EQ(state.update(config, 8.0), poly::SceneSelect::A);
    EXPECT_EQ(state.update(config, 12.0), poly::SceneSelect::A);
    // B for 2 bars (16-23 PPQ)
    EXPECT_EQ(state.update(config, 16.0), poly::SceneSelect::B);
    EXPECT_EQ(state.update(config, 20.0), poly::SceneSelect::B);
    // Back to A
    EXPECT_EQ(state.update(config, 24.0), poly::SceneSelect::A);
}

// --- Scene Chain serialization ---

TEST(SceneChainIO, RoundTrip) {
    poly::SceneState original{};
    original.sceneA.activeLaneCount = 4;
    original.sceneB.activeLaneCount = 6;
    original.select = poly::SceneSelect::A;
    original.morphAmount = 0.0f;

    original.chain.enabled = true;
    original.chain.entryCount = 3;
    original.chain.mode = poly::ChainMode::PingPong;
    original.chain.entries[0] = {poly::SceneSelect::A, 4};
    original.chain.entries[1] = {poly::SceneSelect::Morph, 2};
    original.chain.entries[2] = {poly::SceneSelect::B, 8};

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };
    ASSERT_TRUE(poly::writeSceneState(write, original));

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };

    poly::SceneState restored{};
    ASSERT_TRUE(poly::readSceneState(read, restored));

    EXPECT_EQ(restored.chain.enabled, true);
    EXPECT_EQ(restored.chain.entryCount, 3);
    EXPECT_EQ(static_cast<uint8_t>(restored.chain.mode), static_cast<uint8_t>(poly::ChainMode::PingPong));
    EXPECT_EQ(static_cast<uint8_t>(restored.chain.entries[0].scene), static_cast<uint8_t>(poly::SceneSelect::A));
    EXPECT_EQ(restored.chain.entries[0].bars, 4);
    EXPECT_EQ(static_cast<uint8_t>(restored.chain.entries[1].scene), static_cast<uint8_t>(poly::SceneSelect::Morph));
    EXPECT_EQ(restored.chain.entries[1].bars, 2);
    EXPECT_EQ(static_cast<uint8_t>(restored.chain.entries[2].scene), static_cast<uint8_t>(poly::SceneSelect::B));
    EXPECT_EQ(restored.chain.entries[2].bars, 8);
}

TEST(SceneChainIO, V12BackwardsCompat_NoChain) {
    // Write a v12 scene state (no chain data) and verify it reads with chain disabled
    poly::SceneState original{};
    original.sceneA.activeLaneCount = 4;
    original.sceneB.activeLaneCount = 6;
    original.select = poly::SceneSelect::B;
    original.morphAmount = 0.5f;

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };

    // Write as v12 (manually write version header + body without chain)
    int32_t v12 = 12;
    ASSERT_TRUE(write(&v12, sizeof(v12)));
    ASSERT_TRUE(poly::writeGrooveStateBody(write, original.sceneA, 12));
    ASSERT_TRUE(poly::writeGrooveStateBody(write, original.sceneB, 12));
    auto select = static_cast<uint8_t>(original.select);
    ASSERT_TRUE(write(&select, sizeof(select)));
    ASSERT_TRUE(write(&original.morphAmount, sizeof(original.morphAmount)));
    for (int i = 0; i < 128; ++i) {
        ASSERT_TRUE(write(&original.noteMap.map[static_cast<size_t>(i)], sizeof(int16_t)));
    }

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };

    poly::SceneState restored{};
    ASSERT_TRUE(poly::readSceneState(read, restored));

    EXPECT_EQ(restored.chain.enabled, false);
    EXPECT_EQ(restored.chain.entryCount, 0);
    EXPECT_EQ(restored.sceneA.activeLaneCount, 4);
    EXPECT_EQ(static_cast<uint8_t>(restored.select), static_cast<uint8_t>(poly::SceneSelect::B));
}

// --- Metric modulation tests ---

TEST(MetricModulation, DefaultMultiplierHasNoEffect) {
    poly::GrooveState gs{};
    gs.activeLaneCount = 1;
    gs.lanes[0].active = true;
    gs.lanes[0].cycle = {4, 4};
    gs.lanes[0].hitCount = 4;
    gs.lanes[0].probability = 1.0f;
    gs.lanes[0].tempoMultiplier = 1.0f;

    poly::TransportContext tc{};
    tc.playing = true;
    tc.tempo = 120.0;
    tc.sampleRate = 44100.0;
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;

    poly::Engine engine;
    poly::NoteEventBuffer buf;
    engine.renderRange(tc, gs, buf);

    ASSERT_EQ(buf.count, 4u);
    EXPECT_NEAR(buf.events[0].ppqPosition, 0.0, 0.001);
    EXPECT_NEAR(buf.events[1].ppqPosition, 1.0, 0.001);
    EXPECT_NEAR(buf.events[2].ppqPosition, 2.0, 0.001);
    EXPECT_NEAR(buf.events[3].ppqPosition, 3.0, 0.001);
}

TEST(MetricModulation, DoubleSpeedDoublesNoteCount) {
    poly::GrooveState gs{};
    gs.activeLaneCount = 1;
    gs.lanes[0].active = true;
    gs.lanes[0].cycle = {4, 4};
    gs.lanes[0].hitCount = 4;
    gs.lanes[0].probability = 1.0f;
    gs.lanes[0].tempoMultiplier = 2.0f;

    poly::TransportContext tc{};
    tc.playing = true;
    tc.tempo = 120.0;
    tc.sampleRate = 44100.0;
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;

    poly::Engine engine;
    poly::NoteEventBuffer buf;
    engine.renderRange(tc, gs, buf);

    ASSERT_EQ(buf.count, 8u);
    EXPECT_NEAR(buf.events[0].ppqPosition, 0.0, 0.001);
    EXPECT_NEAR(buf.events[1].ppqPosition, 0.5, 0.001);
    EXPECT_NEAR(buf.events[2].ppqPosition, 1.0, 0.001);
    EXPECT_NEAR(buf.events[3].ppqPosition, 1.5, 0.001);
}

TEST(MetricModulation, HalfSpeedHalvesNoteCount) {
    poly::GrooveState gs{};
    gs.activeLaneCount = 1;
    gs.lanes[0].active = true;
    gs.lanes[0].cycle = {4, 4};
    gs.lanes[0].hitCount = 4;
    gs.lanes[0].probability = 1.0f;
    gs.lanes[0].tempoMultiplier = 0.5f;

    poly::TransportContext tc{};
    tc.playing = true;
    tc.tempo = 120.0;
    tc.sampleRate = 44100.0;
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;

    poly::Engine engine;
    poly::NoteEventBuffer buf;
    engine.renderRange(tc, gs, buf);

    ASSERT_EQ(buf.count, 2u);
    EXPECT_NEAR(buf.events[0].ppqPosition, 0.0, 0.001);
    EXPECT_NEAR(buf.events[1].ppqPosition, 2.0, 0.001);
}

TEST(MetricModulation, TwoLanesIndependentMultipliers) {
    poly::GrooveState gs{};
    gs.activeLaneCount = 2;
    gs.lanes[0].active = true;
    gs.lanes[0].cycle = {4, 4};
    gs.lanes[0].hitCount = 4;
    gs.lanes[0].probability = 1.0f;
    gs.lanes[0].midiNote = 36;
    gs.lanes[0].tempoMultiplier = 1.0f;

    gs.lanes[1].active = true;
    gs.lanes[1].id = 1;
    gs.lanes[1].cycle = {4, 4};
    gs.lanes[1].hitCount = 4;
    gs.lanes[1].probability = 1.0f;
    gs.lanes[1].midiNote = 38;
    gs.lanes[1].tempoMultiplier = 2.0f;

    poly::TransportContext tc{};
    tc.playing = true;
    tc.tempo = 120.0;
    tc.sampleRate = 44100.0;
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;

    poly::Engine engine;
    poly::NoteEventBuffer buf;
    engine.renderRange(tc, gs, buf);

    int lane0Count = 0, lane1Count = 0;
    for (size_t i = 0; i < buf.count; ++i) {
        if (buf.events[i].channel == 0)
            ++lane0Count;
        else if (buf.events[i].channel == 1)
            ++lane1Count;
    }
    EXPECT_EQ(lane0Count, 4);
    EXPECT_EQ(lane1Count, 8);
}

TEST(MetricModulation, SerializationRoundTrip) {
    poly::SceneState original{};
    original.sceneA.activeLaneCount = 2;
    original.sceneA.lanes[0].tempoMultiplier = 2.5f;
    original.sceneA.lanes[1].tempoMultiplier = 0.25f;
    original.sceneB.lanes[0].tempoMultiplier = 4.0f;

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };
    ASSERT_TRUE(poly::writeSceneState(write, original));

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };

    poly::SceneState restored{};
    ASSERT_TRUE(poly::readSceneState(read, restored));

    EXPECT_FLOAT_EQ(restored.sceneA.lanes[0].tempoMultiplier, 2.5f);
    EXPECT_FLOAT_EQ(restored.sceneA.lanes[1].tempoMultiplier, 0.25f);
    EXPECT_FLOAT_EQ(restored.sceneB.lanes[0].tempoMultiplier, 4.0f);
    EXPECT_FLOAT_EQ(restored.sceneB.lanes[1].tempoMultiplier, 1.0f);
}

TEST(MetricModulation, V13BackwardsCompat_DefaultsTo1x) {
    poly::SceneState original{};
    original.sceneA.activeLaneCount = 2;
    original.sceneA.lanes[0].probability = 0.8f;
    original.sceneB.activeLaneCount = 2;
    original.select = poly::SceneSelect::A;
    original.morphAmount = 0.0f;

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };

    int32_t v13 = 13;
    ASSERT_TRUE(write(&v13, sizeof(v13)));
    ASSERT_TRUE(poly::writeGrooveStateBody(write, original.sceneA, 13));
    ASSERT_TRUE(poly::writeGrooveStateBody(write, original.sceneB, 13));
    auto select = static_cast<uint8_t>(original.select);
    ASSERT_TRUE(write(&select, sizeof(select)));
    ASSERT_TRUE(write(&original.morphAmount, sizeof(original.morphAmount)));
    for (int i = 0; i < 128; ++i) {
        ASSERT_TRUE(write(&original.noteMap.map[static_cast<size_t>(i)], sizeof(int16_t)));
    }
    // v13 chain data
    uint8_t chainEnabled = 0;
    ASSERT_TRUE(write(&chainEnabled, sizeof(chainEnabled)));
    int entryCount = 0;
    ASSERT_TRUE(write(&entryCount, sizeof(entryCount)));
    uint8_t chainMode = 0;
    ASSERT_TRUE(write(&chainMode, sizeof(chainMode)));
    for (int i = 0; i < poly::kMaxChainEntries; ++i) {
        uint8_t entryScene = 0;
        ASSERT_TRUE(write(&entryScene, sizeof(entryScene)));
        int bars = 1;
        ASSERT_TRUE(write(&bars, sizeof(bars)));
    }

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };

    poly::SceneState restored{};
    ASSERT_TRUE(poly::readSceneState(read, restored));

    EXPECT_FLOAT_EQ(restored.sceneA.lanes[0].tempoMultiplier, 1.0f);
    EXPECT_FLOAT_EQ(restored.sceneA.lanes[1].tempoMultiplier, 1.0f);
    EXPECT_FLOAT_EQ(restored.sceneA.lanes[0].probability, 0.8f);
}

TEST(MetricModulation, InterpolatesBetweenScenes) {
    poly::GrooveState a{}, b{};
    a.lanes[0].tempoMultiplier = 1.0f;
    b.lanes[0].tempoMultiplier = 3.0f;

    auto result = poly::interpolateGrooveState(a, b, 0.5f);
    EXPECT_FLOAT_EQ(result.lanes[0].tempoMultiplier, 2.0f);
}

TEST(SceneIO, V1BackwardsCompat) {
    poly::GrooveState gs{};
    gs.activeLaneCount = 2;
    gs.lanes[0].probability = 0.5f;

    std::vector<uint8_t> buffer;
    // Manually write v1 format (no curvature/stepValues)
    int32_t version = 1;
    auto pushRaw = [&buffer](const void* data, size_t size) {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
    };
    pushRaw(&version, sizeof(version));
    pushRaw(&gs.activeLaneCount, sizeof(gs.activeLaneCount));
    pushRaw(&gs.seed, sizeof(gs.seed));
    pushRaw(&gs.macros, sizeof(gs.macros));

    for (int i = 0; i < poly::kMaxLanes; ++i) {
        const auto& lane = gs.lanes[i];
        pushRaw(&lane.id, sizeof(lane.id));
        uint8_t role = static_cast<uint8_t>(lane.role);
        pushRaw(&role, sizeof(role));
        pushRaw(&lane.midiNote, sizeof(lane.midiNote));
        pushRaw(&lane.cycle.steps, sizeof(lane.cycle.steps));
        pushRaw(&lane.cycle.subdivision, sizeof(lane.cycle.subdivision));
        pushRaw(&lane.hitCount, sizeof(lane.hitCount));
        pushRaw(&lane.rotation, sizeof(lane.rotation));
        pushRaw(&lane.probability, sizeof(lane.probability));
        pushRaw(&lane.baseVelocity, sizeof(lane.baseVelocity));
        uint64_t accentBits = 0;
        pushRaw(&accentBits, sizeof(accentBits));
        pushRaw(&lane.emphasisProb, sizeof(lane.emphasisProb));
        pushRaw(&lane.ghostFloor, sizeof(lane.ghostFloor));
        pushRaw(&lane.velocitySpread, sizeof(lane.velocitySpread));
        pushRaw(&lane.humanizeMs, sizeof(lane.humanizeMs));
        pushRaw(&lane.swingAmount, sizeof(lane.swingAmount));
        pushRaw(&lane.noteDuration, sizeof(lane.noteDuration));
        uint8_t active = lane.active ? 1 : 0;
        pushRaw(&active, sizeof(active));
        pushRaw(&lane.envelopeCount, sizeof(lane.envelopeCount));

        for (int e = 0; e < poly::kMaxEnvelopesPerLane; ++e) {
            const auto& ea = lane.envelopes[e];
            uint8_t target = static_cast<uint8_t>(ea.envelope.target);
            pushRaw(&target, sizeof(target));
            pushRaw(&ea.envelope.periodBars, sizeof(ea.envelope.periodBars));
            uint8_t shape = static_cast<uint8_t>(ea.envelope.shape);
            pushRaw(&shape, sizeof(shape));
            pushRaw(&ea.envelope.depth, sizeof(ea.envelope.depth));
            pushRaw(&ea.envelope.phaseOffset, sizeof(ea.envelope.phaseOffset));
            // v1: no curvature, no stepCount, no stepValues
            uint8_t envActive = ea.active ? 1 : 0;
            pushRaw(&envActive, sizeof(envActive));
        }
    }

    pushRaw(&gs.globalEnvelopeCount, sizeof(gs.globalEnvelopeCount));
    for (int e = 0; e < poly::kMaxGlobalEnvelopes; ++e) {
        const auto& env = gs.globalEnvelopes[e];
        uint8_t target = static_cast<uint8_t>(env.target);
        pushRaw(&target, sizeof(target));
        pushRaw(&env.periodBars, sizeof(env.periodBars));
        uint8_t shape = static_cast<uint8_t>(env.shape);
        pushRaw(&shape, sizeof(shape));
        pushRaw(&env.depth, sizeof(env.depth));
        pushRaw(&env.phaseOffset, sizeof(env.phaseOffset));
        // v1: no curvature, no stepCount, no stepValues
    }

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };

    poly::SceneState scene{};
    ASSERT_TRUE(poly::readSceneState(read, scene));

    EXPECT_EQ(scene.sceneA.activeLaneCount, 2);
    EXPECT_FLOAT_EQ(scene.sceneA.lanes[0].probability, 0.5f);
    EXPECT_EQ(scene.sceneA.lanes[0].envelopes[0].envelope.curvature, 0.0f);
    EXPECT_EQ(scene.sceneA.lanes[0].envelopes[0].envelope.stepCount, 0);
    EXPECT_EQ(static_cast<uint8_t>(scene.select), static_cast<uint8_t>(poly::SceneSelect::A));
}
