#include <cmath>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/sanitize.h"
#include "poly/state_io.h"

namespace {

poly::GrooveState makeHostileState() {
    poly::GrooveState state{};
    state.activeLaneCount = 1000;
    state.globalEnvelopeCount = 99;
    state.macros.density = std::nanf("");
    for (auto& lane : state.lanes) {
        lane.active = true;
        lane.cycle.steps = 5000;
        lane.cycle.subdivision = 0;
        lane.hitCount = -3;
        lane.probability = 7.0f;
        lane.envelopeCount = 77;
        lane.envelopes[0].envelope.periodBars = 0.0f;
        lane.tempoMultiplier = 0.0f;
        lane.kotekanSourceLane = 42;
        lane.cellCount = 4;
        lane.cellSizes.fill(0);
        lane.midiNote = 999;
    }
    return state;
}

std::vector<uint8_t> serialize(const poly::GrooveState& state) {
    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };
    EXPECT_TRUE(poly::writeGrooveState(write, state));
    return buffer;
}

} // namespace

TEST(Sanitize, ClampsHostileFieldsIntoEngineRanges) {
    auto state = makeHostileState();
    poly::sanitizeGrooveState(state);

    EXPECT_LE(state.activeLaneCount, poly::kMaxLanes);
    EXPECT_LE(state.globalEnvelopeCount, poly::kMaxGlobalEnvelopes);
    EXPECT_GE(state.macros.density, 0.0f);
    EXPECT_LE(state.macros.density, 1.0f);
    for (const auto& lane : state.lanes) {
        EXPECT_GE(lane.cycle.steps, 1);
        EXPECT_LE(lane.cycle.steps, poly::kMaxSteps);
        EXPECT_GE(lane.cycle.subdivision, 1);
        EXPECT_GE(lane.hitCount, 0);
        EXPECT_LE(lane.hitCount, lane.cycle.steps);
        EXPECT_LE(lane.probability, 1.0f);
        EXPECT_LE(lane.envelopeCount, poly::kMaxEnvelopesPerLane);
        EXPECT_GT(lane.envelopes[0].envelope.periodBars, 0.0f);
        EXPECT_GE(lane.tempoMultiplier, 0.25f);
        EXPECT_LE(lane.kotekanSourceLane, poly::kMaxLanes - 1);
        for (int i = 0; i < lane.cellCount; ++i)
            EXPECT_GE(lane.cellSizes[static_cast<size_t>(i)], 1);
        EXPECT_LE(lane.midiNote, 127);
    }
}

TEST(Sanitize, ReadGrooveStateSanitizesHostileStream) {
    auto buffer = serialize(makeHostileState());

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

    EXPECT_LE(restored.activeLaneCount, poly::kMaxLanes);
    EXPECT_LE(restored.lanes[0].cycle.steps, poly::kMaxSteps);
    EXPECT_GT(restored.lanes[0].envelopes[0].envelope.periodBars, 0.0f);
}

TEST(Sanitize, RenderRangeSurvivesHostileStateUnsanitized) {
    auto state = makeHostileState();
    for (auto& lane : state.lanes)
        lane.cycle.subdivision = 8;

    poly::TransportContext tc{};
    tc.playing = true;
    tc.ppqStart = 0.0;
    tc.ppqEnd = 1.0;
    tc.tempo = 120.0;

    poly::NoteEventBuffer out{};
    poly::Engine engine{};
    engine.renderRange(tc, state, out);
    EXPECT_EQ(out.count, 0u);
}

TEST(Sanitize, RenderRangeProducesEventsAfterSanitization) {
    auto state = makeHostileState();
    poly::sanitizeGrooveState(state);

    poly::TransportContext tc{};
    tc.playing = true;
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;

    poly::NoteEventBuffer out{};
    poly::Engine engine{};
    engine.renderRange(tc, state, out);
    EXPECT_GT(out.count, 0u);
}

// Regression tests for M049 S04 / E4 finding from the 2026-07-16 review.
//
// Bug: `deterministicRand(seed, laneId, absStep, channel)` in engine.cpp keys 8
// per-step decision channels on `cfg.id`. If two lanes share the same id
// (which is possible via a hostile preset or an editing mistake — sanitize
// did not enforce uniqueness), the two lanes produce byte-identical
// probability/mutation/ghost/activation rolls, silently correlating what
// should be independent voices.
//
// Fix: sanitizeLane repairs lane.id = laneIndex unconditionally.

namespace {

poly::GrooveState makeTwoLaneStateWithSharedId(int sharedId) {
    poly::GrooveState state{};
    state.seed = 42;
    state.activeLaneCount = 2;
    for (int i = 0; i < 2; ++i) {
        auto& lane = state.lanes[static_cast<size_t>(i)];
        lane.id = sharedId; // Deliberate duplicate.
        lane.active = true;
        lane.midiNote = static_cast<int16_t>(36 + i);
        lane.cycle = {.steps = 8, .subdivision = 4};
        lane.hitCount = 4;
        lane.baseVelocity = 100;
        // probability < 1 so deterministicRand actually gates emission —
        // otherwise both lanes fire on every step regardless of id and the
        // test can't distinguish correlated vs independent rolls.
        lane.probability = 0.5f;
        lane.emphasisProb = 0.5f;
        lane.ghostFloor = 0;
        lane.velocitySpread = 0.5f; // exercise channel 1
    }
    return state;
}

std::vector<poly::NoteEvent> renderAndCollect(const poly::GrooveState& state, int lane) {
    poly::Engine engine{};
    poly::TransportContext tc{};
    tc.playing = true;
    tc.ppqStart = 0.0;
    tc.ppqEnd = 16.0;
    tc.tempo = 120.0;
    poly::NoteEventBuffer buf{};
    engine.renderRange(tc, state, buf);
    std::vector<poly::NoteEvent> out;
    for (size_t i = 0; i < buf.count; ++i)
        if (buf.events[i].laneIndex == lane)
            out.push_back(buf.events[i]);
    return out;
}

// Compare two lanes' event streams ignoring pitch (they use different midiNotes)
// so we're only checking that the RNG-driven timing/velocity/emission decisions
// diverged.
bool rngDecisionsIdentical(const std::vector<poly::NoteEvent>& a, const std::vector<poly::NoteEvent>& b) {
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].ppqPosition != b[i].ppqPosition)
            return false;
        if (a[i].velocity != b[i].velocity)
            return false;
    }
    return true;
}

} // namespace

TEST(Sanitize, LaneIdsRepairedToLaneIndex) {
    poly::GrooveState state{};
    state.activeLaneCount = 3;
    state.lanes[0].id = 3;
    state.lanes[1].id = 3;
    state.lanes[2].id = 3;
    state.lanes[3].id = 3;

    poly::sanitizeGrooveState(state);

    EXPECT_EQ(state.lanes[0].id, 0);
    EXPECT_EQ(state.lanes[1].id, 1);
    EXPECT_EQ(state.lanes[2].id, 2);
    EXPECT_EQ(state.lanes[3].id, 3);
}

TEST(Sanitize, DuplicateLaneIdsProduceCorrelatedRollsBeforeFix) {
    // Without sanitize, two lanes with the same id emit byte-identical
    // RNG-driven decisions (same ppq timing offsets, same velocity randomization).
    // This test documents the bug and locks in the invariant that motivates the fix.
    auto state = makeTwoLaneStateWithSharedId(5);

    auto lane0 = renderAndCollect(state, 0);
    auto lane1 = renderAndCollect(state, 1);

    ASSERT_FALSE(lane0.empty()) << "test setup: lane 0 must emit at least one note";
    ASSERT_FALSE(lane1.empty()) << "test setup: lane 1 must emit at least one note";
    EXPECT_TRUE(rngDecisionsIdentical(lane0, lane1))
        << "pre-fix: two lanes with same id must produce correlated RNG rolls — "
        << "if they diverge here, the fix already landed or the test is checking the wrong invariant";
}

TEST(Sanitize, DuplicateLaneIdsProduceIndependentRollsAfterSanitize) {
    // After sanitize, the two lanes have distinct ids (0 and 1) so their RNG
    // channels diverge. The two event streams must differ on either timing,
    // velocity, or count.
    auto state = makeTwoLaneStateWithSharedId(5);
    poly::sanitizeGrooveState(state);

    ASSERT_EQ(state.lanes[0].id, 0);
    ASSERT_EQ(state.lanes[1].id, 1);

    auto lane0 = renderAndCollect(state, 0);
    auto lane1 = renderAndCollect(state, 1);

    ASSERT_FALSE(lane0.empty());
    ASSERT_FALSE(lane1.empty());
    EXPECT_FALSE(rngDecisionsIdentical(lane0, lane1))
        << "post-fix: sanitize must repair lane ids so RNG channels diverge";
}
