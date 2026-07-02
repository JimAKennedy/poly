#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#include "poly/bridge.h"
#include "poly/engine.h"
#include "poly/state_io.h"
#include "poly/types.h"

// --- ppqToSampleOffset tests ---

TEST(Bridge, PpqToSampleOffset_BlockStart) {
    // PPQ at block start → sample 0
    auto offset = poly::ppqToSampleOffset(0.0, 0.0, 120.0, 44100.0, 512);
    EXPECT_EQ(offset, 0);
}

TEST(Bridge, PpqToSampleOffset_MidBlock) {
    // At 120 BPM, 0.5 beats = 0.25 seconds = 11025 samples
    auto offset = poly::ppqToSampleOffset(0.5, 0.0, 120.0, 44100.0, 44100);
    EXPECT_EQ(offset, 11025);
}

TEST(Bridge, PpqToSampleOffset_FractionalBeat) {
    // 0.25 beats at 120 BPM = 0.125 seconds = 5513 samples
    auto offset = poly::ppqToSampleOffset(4.25, 4.0, 120.0, 44100.0, 44100);
    EXPECT_EQ(offset, 5513);
}

TEST(Bridge, PpqToSampleOffset_ClampsToBlockSize) {
    // PPQ way past block end → clamped to blockSize - 1
    auto offset = poly::ppqToSampleOffset(100.0, 0.0, 120.0, 44100.0, 512);
    EXPECT_EQ(offset, 511);
}

TEST(Bridge, PpqToSampleOffset_ClampsNegative) {
    // PPQ before block start → clamped to 0
    auto offset = poly::ppqToSampleOffset(-1.0, 0.0, 120.0, 44100.0, 512);
    EXPECT_EQ(offset, 0);
}

TEST(Bridge, PpqToSampleOffset_ZeroTempo) {
    auto offset = poly::ppqToSampleOffset(1.0, 0.0, 0.0, 44100.0, 512);
    EXPECT_EQ(offset, 0);
}

// --- PendingNoteOffBuffer tests ---

TEST(PendingNoteOff, PushAndCount) {
    poly::PendingNoteOffBuffer buf;
    EXPECT_EQ(buf.count(), 0u);

    buf.push({.ppqOff = 1.0, .pitch = 36, .channel = 0});
    buf.push({.ppqOff = 2.0, .pitch = 38, .channel = 0});
    EXPECT_EQ(buf.count(), 2u);
}

TEST(PendingNoteOff, FlushDue_EmitsInRange) {
    poly::PendingNoteOffBuffer buf;
    buf.push({.ppqOff = 0.5, .pitch = 36, .channel = 0});
    buf.push({.ppqOff = 1.5, .pitch = 38, .channel = 0});
    buf.push({.ppqOff = 2.5, .pitch = 42, .channel = 0});

    poly::PendingNoteOff out[8];
    size_t n = buf.flushDue(1.0, 2.0, out, 8);

    EXPECT_EQ(n, 1u);
    EXPECT_EQ(out[0].pitch, 38);
    EXPECT_EQ(buf.count(), 2u);
}

TEST(PendingNoteOff, FlushDue_MultipleInRange) {
    poly::PendingNoteOffBuffer buf;
    buf.push({.ppqOff = 1.0, .pitch = 36, .channel = 0});
    buf.push({.ppqOff = 1.25, .pitch = 38, .channel = 0});
    buf.push({.ppqOff = 1.5, .pitch = 42, .channel = 0});

    poly::PendingNoteOff out[8];
    size_t n = buf.flushDue(1.0, 2.0, out, 8);

    EXPECT_EQ(n, 3u);
    EXPECT_EQ(buf.count(), 0u);
}

TEST(PendingNoteOff, FlushDue_NoneInRange) {
    poly::PendingNoteOffBuffer buf;
    buf.push({.ppqOff = 5.0, .pitch = 36, .channel = 0});

    poly::PendingNoteOff out[8];
    size_t n = buf.flushDue(0.0, 1.0, out, 8);

    EXPECT_EQ(n, 0u);
    EXPECT_EQ(buf.count(), 1u);
}

TEST(PendingNoteOff, FlushAcrossBlocks) {
    poly::PendingNoteOffBuffer buf;
    buf.push({.ppqOff = 0.5, .pitch = 36, .channel = 0});
    buf.push({.ppqOff = 1.5, .pitch = 38, .channel = 0});
    buf.push({.ppqOff = 2.5, .pitch = 42, .channel = 0});

    poly::PendingNoteOff out[8];

    // Block 1: [0, 1)
    size_t n1 = buf.flushDue(0.0, 1.0, out, 8);
    EXPECT_EQ(n1, 1u);
    EXPECT_EQ(buf.count(), 2u);

    // Block 2: [1, 2)
    size_t n2 = buf.flushDue(1.0, 2.0, out, 8);
    EXPECT_EQ(n2, 1u);
    EXPECT_EQ(buf.count(), 1u);

    // Block 3: [2, 3)
    size_t n3 = buf.flushDue(2.0, 3.0, out, 8);
    EXPECT_EQ(n3, 1u);
    EXPECT_EQ(buf.count(), 0u);
}

TEST(PendingNoteOff, Clear) {
    poly::PendingNoteOffBuffer buf;
    buf.push({.ppqOff = 1.0, .pitch = 36, .channel = 0});
    buf.push({.ppqOff = 2.0, .pitch = 38, .channel = 0});
    buf.clear();
    EXPECT_EQ(buf.count(), 0u);
}

TEST(PendingNoteOff, PushOverflow) {
    poly::PendingNoteOffBuffer buf;
    for (size_t i = 0; i < poly::PendingNoteOffBuffer::kCapacity; ++i) {
        EXPECT_TRUE(buf.push({.ppqOff = static_cast<double>(i), .pitch = 36, .channel = 0}));
    }
    EXPECT_FALSE(buf.push({.ppqOff = 999.0, .pitch = 36, .channel = 0}));
    EXPECT_EQ(buf.count(), poly::PendingNoteOffBuffer::kCapacity);
}

// --- State serialization round-trip tests ---

static poly::GrooveState makeTestState() {
    poly::GrooveState state{};
    state.activeLaneCount = 3;
    state.seed = 12345;
    state.macros.complexity = 0.7f;
    state.macros.density = 0.3f;
    state.macros.syncopation = 0.2f;
    state.macros.swing = 0.1f;
    state.macros.tension = 0.4f;
    state.macros.humanize = 0.15f;

    auto& lane0 = state.lanes[0];
    lane0.id = 1;
    lane0.role = poly::Role::AnchorPulse;
    lane0.midiNote = 36;
    lane0.cycle = {.steps = 4, .subdivision = 4};
    lane0.hitCount = 4;
    lane0.rotation = 1;
    lane0.probability = 0.9f;
    lane0.baseVelocity = 110;
    lane0.accents.steps[0] = 1.0f;
    lane0.accents.steps[2] = 1.0f;
    lane0.emphasisProb = 0.6f;
    lane0.ghostFloor = 25;
    lane0.velocitySpread = 0.1f;
    lane0.humanizeMs = 5.0f;
    lane0.swingAmount = 0.3f;
    lane0.noteDuration = 0.5f;
    lane0.active = true;
    lane0.envelopeCount = 1;
    lane0.envelopes[0].active = true;
    lane0.envelopes[0].envelope.target = poly::EnvTarget::Velocity;
    lane0.envelopes[0].envelope.periodBars = 4.0f;
    lane0.envelopes[0].envelope.shape = poly::Shape::Sine;
    lane0.envelopes[0].envelope.depth = 0.5f;
    lane0.envelopes[0].envelope.phaseOffset = 0.25f;

    auto& lane1 = state.lanes[1];
    lane1.id = 2;
    lane1.role = poly::Role::Ghost;
    lane1.midiNote = 45;
    lane1.cycle = {.steps = 5, .subdivision = 16};
    lane1.hitCount = 3;
    lane1.probability = 0.7f;
    lane1.baseVelocity = 50;
    lane1.ghostFloor = 20;
    lane1.active = true;

    state.globalEnvelopeCount = 1;
    state.globalEnvelopes[0].target = poly::EnvTarget::Density;
    state.globalEnvelopes[0].periodBars = 8.0f;
    state.globalEnvelopes[0].shape = poly::Shape::Triangle;
    state.globalEnvelopes[0].depth = 0.3f;
    state.globalEnvelopes[0].phaseOffset = 0.0f;

    return state;
}

TEST(StateIO, RoundTrip) {
    auto original = makeTestState();

    // Write
    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };
    ASSERT_TRUE(poly::writeGrooveState(write, original));
    EXPECT_GT(buffer.size(), 100u);

    // Read
    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };
    poly::GrooveState restored{};
    ASSERT_TRUE(poly::readGrooveState(read, restored));

    // Verify globals
    EXPECT_EQ(restored.activeLaneCount, original.activeLaneCount);
    EXPECT_EQ(restored.seed, original.seed);
    EXPECT_FLOAT_EQ(restored.macros.complexity, original.macros.complexity);
    EXPECT_FLOAT_EQ(restored.macros.density, original.macros.density);
    EXPECT_FLOAT_EQ(restored.macros.syncopation, original.macros.syncopation);
    EXPECT_FLOAT_EQ(restored.macros.swing, original.macros.swing);
    EXPECT_FLOAT_EQ(restored.macros.tension, original.macros.tension);
    EXPECT_FLOAT_EQ(restored.macros.humanize, original.macros.humanize);

    // Verify lane 0
    const auto& lo = original.lanes[0];
    const auto& lr = restored.lanes[0];
    EXPECT_EQ(lr.id, lo.id);
    EXPECT_EQ(static_cast<uint8_t>(lr.role), static_cast<uint8_t>(lo.role));
    EXPECT_EQ(lr.midiNote, lo.midiNote);
    EXPECT_EQ(lr.cycle.steps, lo.cycle.steps);
    EXPECT_EQ(lr.cycle.subdivision, lo.cycle.subdivision);
    EXPECT_EQ(lr.hitCount, lo.hitCount);
    EXPECT_EQ(lr.rotation, lo.rotation);
    EXPECT_FLOAT_EQ(lr.probability, lo.probability);
    EXPECT_EQ(lr.baseVelocity, lo.baseVelocity);
    EXPECT_FLOAT_EQ(lr.accents.steps[0], 1.0f);
    EXPECT_FLOAT_EQ(lr.accents.steps[1], 0.0f);
    EXPECT_FLOAT_EQ(lr.accents.steps[2], 1.0f);
    EXPECT_FLOAT_EQ(lr.emphasisProb, lo.emphasisProb);
    EXPECT_EQ(lr.ghostFloor, lo.ghostFloor);
    EXPECT_FLOAT_EQ(lr.velocitySpread, lo.velocitySpread);
    EXPECT_FLOAT_EQ(lr.humanizeMs, lo.humanizeMs);
    EXPECT_FLOAT_EQ(lr.swingAmount, lo.swingAmount);
    EXPECT_FLOAT_EQ(lr.noteDuration, lo.noteDuration);
    EXPECT_EQ(lr.active, lo.active);
    EXPECT_EQ(lr.envelopeCount, lo.envelopeCount);
    EXPECT_EQ(static_cast<uint8_t>(lr.envelopes[0].envelope.target),
              static_cast<uint8_t>(lo.envelopes[0].envelope.target));
    EXPECT_FLOAT_EQ(lr.envelopes[0].envelope.periodBars, lo.envelopes[0].envelope.periodBars);
    EXPECT_EQ(static_cast<uint8_t>(lr.envelopes[0].envelope.shape),
              static_cast<uint8_t>(lo.envelopes[0].envelope.shape));
    EXPECT_FLOAT_EQ(lr.envelopes[0].envelope.depth, lo.envelopes[0].envelope.depth);
    EXPECT_FLOAT_EQ(lr.envelopes[0].envelope.phaseOffset, lo.envelopes[0].envelope.phaseOffset);
    EXPECT_EQ(lr.envelopes[0].active, lo.envelopes[0].active);

    // Verify lane 1
    EXPECT_EQ(restored.lanes[1].id, original.lanes[1].id);
    EXPECT_EQ(restored.lanes[1].midiNote, original.lanes[1].midiNote);
    EXPECT_EQ(restored.lanes[1].cycle.steps, original.lanes[1].cycle.steps);
    EXPECT_FLOAT_EQ(restored.lanes[1].probability, original.lanes[1].probability);
    EXPECT_EQ(restored.lanes[1].active, original.lanes[1].active);

    // Verify global envelopes
    EXPECT_EQ(restored.globalEnvelopeCount, original.globalEnvelopeCount);
    EXPECT_EQ(static_cast<uint8_t>(restored.globalEnvelopes[0].target),
              static_cast<uint8_t>(original.globalEnvelopes[0].target));
    EXPECT_FLOAT_EQ(restored.globalEnvelopes[0].periodBars, original.globalEnvelopes[0].periodBars);
}

TEST(StateIO, BadVersionFails) {
    std::vector<uint8_t> buffer;
    int32_t badVersion = 99;
    auto p = reinterpret_cast<const uint8_t*>(&badVersion);
    buffer.insert(buffer.end(), p, p + sizeof(badVersion));

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };

    poly::GrooveState state{};
    EXPECT_FALSE(poly::readGrooveState(read, state));
}

TEST(StateIO, TruncatedStreamFails) {
    auto original = makeTestState();

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };
    ASSERT_TRUE(poly::writeGrooveState(write, original));

    // Truncate at 50% of the data
    buffer.resize(buffer.size() / 2);

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };

    poly::GrooveState restored{};
    EXPECT_FALSE(poly::readGrooveState(read, restored));
}

TEST(StateIO, DefaultStateRoundTrip) {
    poly::GrooveState original{};
    // Default-constructed state should round-trip cleanly

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };
    ASSERT_TRUE(poly::writeGrooveState(write, original));

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };

    poly::GrooveState restored{};
    ASSERT_TRUE(poly::readGrooveState(read, restored));

    EXPECT_EQ(restored.activeLaneCount, original.activeLaneCount);
    EXPECT_EQ(restored.seed, original.seed);
    EXPECT_FLOAT_EQ(restored.macros.complexity, original.macros.complexity);
}

TEST(StateIO, V11AccentBitmaskBackwardCompat) {
    auto original = makeTestState();
    original.lanes[0].accents.steps[0] = 1.0f;
    original.lanes[0].accents.steps[2] = 1.0f;
    original.lanes[0].accents.steps[5] = 1.0f;

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };

    int32_t v11 = 11;
    ASSERT_TRUE(write(&v11, sizeof(v11)));
    ASSERT_TRUE(poly::writeGrooveStateBody(write, original, 11));

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };

    poly::GrooveState restored{};
    ASSERT_TRUE(poly::readGrooveState(read, restored));

    EXPECT_FLOAT_EQ(restored.lanes[0].accents.steps[0], 1.0f);
    EXPECT_FLOAT_EQ(restored.lanes[0].accents.steps[1], 0.0f);
    EXPECT_FLOAT_EQ(restored.lanes[0].accents.steps[2], 1.0f);
    EXPECT_FLOAT_EQ(restored.lanes[0].accents.steps[3], 0.0f);
    EXPECT_FLOAT_EQ(restored.lanes[0].accents.steps[5], 1.0f);
}

TEST(CellSizes, AksakProducesDifferentTiming) {
    poly::GrooveState equalState{};
    equalState.activeLaneCount = 1;
    equalState.seed = 42;
    equalState.macros.density = 1.0f;
    auto& eqLane = equalState.lanes[0];
    eqLane.active = true;
    eqLane.midiNote = 36;
    eqLane.cycle = {.steps = 3, .subdivision = 8};
    eqLane.hitCount = 3;
    eqLane.probability = 1.0f;
    eqLane.baseVelocity = 100;
    eqLane.cellCount = 0;

    poly::GrooveState aksakState = equalState;
    auto& akLane = aksakState.lanes[0];
    akLane.cellCount = 3;
    akLane.cellSizes[0] = 2;
    akLane.cellSizes[1] = 2;
    akLane.cellSizes[2] = 3;

    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.sampleRate = 44100.0;
    tc.blockSize = 44100;
    tc.playing = true;

    poly::Engine engine;
    poly::NoteEventBuffer eqOut, akOut;
    engine.renderRange(tc, equalState, eqOut);
    engine.renderRange(tc, aksakState, akOut);

    ASSERT_GT(eqOut.count, 0u);
    ASSERT_GT(akOut.count, 0u);

    bool timingsDiffer = false;
    size_t minCount = std::min(eqOut.count, akOut.count);
    for (size_t i = 0; i < minCount; ++i) {
        if (std::abs(eqOut.events[i].ppqPosition - akOut.events[i].ppqPosition) > 1e-6) {
            timingsDiffer = true;
            break;
        }
    }
    if (eqOut.count != akOut.count)
        timingsDiffer = true;
    EXPECT_TRUE(timingsDiffer) << "Aksak cellSizes should produce different note timing than equal cells";
}

TEST(Timeline, FixedPatternSelectsSteps) {
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;
    state.macros.density = 1.0f;
    auto& lane = state.lanes[0];
    lane.active = true;
    lane.midiNote = 36;
    lane.cycle = {.steps = 4, .subdivision = 4};
    lane.hitCount = 4;
    lane.probability = 1.0f;
    lane.baseVelocity = 100;
    lane.timeline = true;
    lane.fixedPatternLength = 4;
    lane.fixedPattern[0] = true;
    lane.fixedPattern[1] = false;
    lane.fixedPattern[2] = true;
    lane.fixedPattern[3] = false;

    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.sampleRate = 44100.0;
    tc.blockSize = 44100;
    tc.playing = true;

    poly::Engine engine;
    poly::NoteEventBuffer out;
    engine.renderRange(tc, state, out);

    ASSERT_GT(out.count, 0u);
    EXPECT_LE(out.count, 2u) << "Only steps 0 and 2 are on; should produce at most 2 notes";
}

TEST(MicroTiming, OffsetsShiftNotePosition) {
    poly::GrooveState baseState{};
    baseState.activeLaneCount = 1;
    baseState.seed = 42;
    baseState.macros.density = 1.0f;
    auto& baseLane = baseState.lanes[0];
    baseLane.active = true;
    baseLane.midiNote = 36;
    baseLane.cycle = {.steps = 4, .subdivision = 4};
    baseLane.hitCount = 4;
    baseLane.probability = 1.0f;
    baseLane.baseVelocity = 100;

    poly::GrooveState offsetState = baseState;
    for (int i = 0; i < 4; ++i)
        offsetState.lanes[0].microTimingMs[i] = 10.0f;

    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.sampleRate = 44100.0;
    tc.blockSize = 44100;
    tc.playing = true;

    poly::Engine engine;
    poly::NoteEventBuffer baseOut, offsetOut;
    engine.renderRange(tc, baseState, baseOut);
    engine.renderRange(tc, offsetState, offsetOut);

    ASSERT_GT(baseOut.count, 0u);
    ASSERT_EQ(baseOut.count, offsetOut.count);

    bool anyShifted = false;
    for (size_t i = 0; i < baseOut.count; ++i) {
        if (std::abs(baseOut.events[i].ppqPosition - offsetOut.events[i].ppqPosition) > 1e-9) {
            anyShifted = true;
            break;
        }
    }
    EXPECT_TRUE(anyShifted) << "Micro-timing offsets should shift note positions";
}

TEST(StateIO, V12FloatAccentRoundTrip) {
    auto original = makeTestState();
    original.lanes[0].accents.steps[0] = 0.75f;
    original.lanes[0].accents.steps[1] = 0.25f;
    original.lanes[0].accents.steps[2] = 1.0f;
    original.lanes[0].accents.steps[3] = 0.0f;

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };
    ASSERT_TRUE(poly::writeGrooveState(write, original));

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };

    poly::GrooveState restored{};
    ASSERT_TRUE(poly::readGrooveState(read, restored));

    EXPECT_FLOAT_EQ(restored.lanes[0].accents.steps[0], 0.75f);
    EXPECT_FLOAT_EQ(restored.lanes[0].accents.steps[1], 0.25f);
    EXPECT_FLOAT_EQ(restored.lanes[0].accents.steps[2], 1.0f);
    EXPECT_FLOAT_EQ(restored.lanes[0].accents.steps[3], 0.0f);
}
