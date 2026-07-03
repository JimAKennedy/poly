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
