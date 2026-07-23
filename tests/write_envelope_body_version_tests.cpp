#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#include "poly/state_io.h"
#include "poly/types.h"

// Regression tests for M049 S05 / E5 from the 2026-07-16 product review.
//
// Bug: writeEnvelope() always emitted the v2+ suffix (curvature + stepCount +
// 16 stepValues) regardless of writeGrooveStateBody(bodyVersion). readEnvelope
// gates that suffix on `version >= 2`, so any writeGrooveStateBody(1) produced
// bytes readGrooveStateBody(1) could not align — every subsequent field read
// wrong values or the whole read failed.
//
// Fix: writeEnvelope takes an explicit version parameter, mirrors the read
// side's gate, and writeLaneConfig / writeGrooveStateBody pass bodyVersion
// through.

namespace {

struct Buf {
    std::vector<uint8_t> bytes;
    size_t readPos = 0;

    auto writeFn() {
        return [this](const void* data, size_t size) -> bool {
            auto p = static_cast<const uint8_t*>(data);
            bytes.insert(bytes.end(), p, p + size);
            return true;
        };
    }
    auto readFn() {
        readPos = 0;
        return [this](void* data, size_t size) -> bool {
            if (readPos + size > bytes.size())
                return false;
            std::memcpy(data, bytes.data() + readPos, size);
            readPos += size;
            return true;
        };
    }
};

poly::Envelope makeEnvelope() {
    poly::Envelope env{};
    env.target = poly::EnvTarget::Velocity;
    env.periodBars = 4.0f;
    env.shape = poly::Shape::Sine;
    env.depth = 0.5f;
    env.phaseOffset = 0.25f;
    env.curvature = 0.3f; // v2+ only
    env.stepCount = 4;    // v2+ only
    for (int i = 0; i < poly::kMaxStepListEntries; ++i)
        env.stepValues[i] = 0.1f * static_cast<float>(i); // v2+ only
    return env;
}

poly::GrooveState makeStateForRoundTrip() {
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = 42;
    state.macros.complexity = 0.5f;
    state.macros.density = 0.6f;
    state.macros.syncopation = 0.3f;
    for (int i = 0; i < poly::kMaxLanes; ++i) {
        auto& lane = state.lanes[i];
        lane.id = i;
        lane.role = poly::Role::AnchorPulse;
        lane.midiNote = 36 + i;
        lane.cycle = {.steps = 8, .subdivision = 8};
        lane.hitCount = 4;
        lane.probability = 0.9f;
        lane.baseVelocity = 100;
        lane.emphasisProb = 0.5f;
        lane.ghostFloor = 30;
        lane.velocitySpread = 0.05f;
        lane.humanizeMs = 5.0f;
        lane.swingAmount = 0.1f;
        lane.noteDuration = 0.5f;
        lane.active = true;
        lane.envelopeCount = 2;
        lane.envelopes[0].envelope = makeEnvelope();
        lane.envelopes[0].active = true;
        lane.envelopes[1].envelope = makeEnvelope();
        lane.envelopes[1].envelope.target = poly::EnvTarget::Density;
        lane.envelopes[1].active = false;
    }
    state.globalEnvelopeCount = 1;
    state.globalEnvelopes[0] = makeEnvelope();
    state.globalEnvelopes[0].target = poly::EnvTarget::AccentBias;
    return state;
}

} // namespace

TEST(WriteEnvelopeBodyVersion, RoundTripsAtV1) {
    // v1 = no curvature/stepCount/stepValues suffix on envelopes.
    Buf buf;
    auto state = makeStateForRoundTrip();
    ASSERT_TRUE(poly::writeGrooveStateBody(buf.writeFn(), state, /*bodyVersion=*/1));

    poly::GrooveState restored{};
    ASSERT_TRUE(poly::readGrooveStateBody(buf.readFn(), restored, /*version=*/1));

    // Fields present in v1 must survive.
    EXPECT_EQ(restored.activeLaneCount, state.activeLaneCount);
    EXPECT_EQ(restored.seed, state.seed);
    EXPECT_FLOAT_EQ(restored.macros.complexity, state.macros.complexity);
    EXPECT_FLOAT_EQ(restored.macros.density, state.macros.density);

    for (int i = 0; i < poly::kMaxLanes; ++i) {
        const auto& lo = state.lanes[i];
        const auto& lr = restored.lanes[i];
        EXPECT_EQ(lr.id, lo.id);
        EXPECT_EQ(lr.midiNote, lo.midiNote);
        EXPECT_EQ(lr.cycle.steps, lo.cycle.steps);
        EXPECT_EQ(lr.cycle.subdivision, lo.cycle.subdivision);
        EXPECT_EQ(lr.hitCount, lo.hitCount);
        EXPECT_FLOAT_EQ(lr.probability, lo.probability);
        EXPECT_EQ(lr.baseVelocity, lo.baseVelocity);
        EXPECT_EQ(lr.envelopeCount, lo.envelopeCount);

        // v1 envelopes preserve target/periodBars/shape/depth/phaseOffset.
        EXPECT_EQ(static_cast<uint8_t>(lr.envelopes[0].envelope.target),
                  static_cast<uint8_t>(lo.envelopes[0].envelope.target));
        EXPECT_FLOAT_EQ(lr.envelopes[0].envelope.periodBars, lo.envelopes[0].envelope.periodBars);
        EXPECT_EQ(static_cast<uint8_t>(lr.envelopes[0].envelope.shape),
                  static_cast<uint8_t>(lo.envelopes[0].envelope.shape));
        EXPECT_FLOAT_EQ(lr.envelopes[0].envelope.depth, lo.envelopes[0].envelope.depth);
        EXPECT_FLOAT_EQ(lr.envelopes[0].envelope.phaseOffset, lo.envelopes[0].envelope.phaseOffset);
        EXPECT_EQ(lr.envelopes[0].active, lo.envelopes[0].active);
        EXPECT_EQ(lr.envelopes[1].active, lo.envelopes[1].active);
    }
}

TEST(WriteEnvelopeBodyVersion, RoundTripsAtV2) {
    // v2 introduced curvature + step list. Full envelope round-trip.
    Buf buf;
    auto state = makeStateForRoundTrip();
    ASSERT_TRUE(poly::writeGrooveStateBody(buf.writeFn(), state, /*bodyVersion=*/2));

    poly::GrooveState restored{};
    ASSERT_TRUE(poly::readGrooveStateBody(buf.readFn(), restored, /*version=*/2));

    for (int i = 0; i < poly::kMaxLanes; ++i) {
        const auto& lo = state.lanes[i].envelopes[0].envelope;
        const auto& lr = restored.lanes[i].envelopes[0].envelope;
        EXPECT_FLOAT_EQ(lr.curvature, lo.curvature);
        EXPECT_EQ(lr.stepCount, lo.stepCount);
        for (int s = 0; s < poly::kMaxStepListEntries; ++s)
            EXPECT_FLOAT_EQ(lr.stepValues[s], lo.stepValues[s]);
    }
    // Global envelope suffix too.
    const auto& go = state.globalEnvelopes[0];
    const auto& gr = restored.globalEnvelopes[0];
    EXPECT_FLOAT_EQ(gr.curvature, go.curvature);
    EXPECT_EQ(gr.stepCount, go.stepCount);
}

TEST(WriteEnvelopeBodyVersion, RoundTripsAtCurrentVersion) {
    // Current version — every field survives.
    Buf buf;
    auto state = makeStateForRoundTrip();
    ASSERT_TRUE(poly::writeGrooveStateBody(buf.writeFn(), state, poly::kCurrentStateVersion));

    poly::GrooveState restored{};
    ASSERT_TRUE(poly::readGrooveStateBody(buf.readFn(), restored, poly::kCurrentStateVersion));

    EXPECT_EQ(restored.activeLaneCount, state.activeLaneCount);
    EXPECT_EQ(restored.lanes[0].envelopeCount, state.lanes[0].envelopeCount);
    EXPECT_FLOAT_EQ(restored.lanes[0].envelopes[0].envelope.curvature, state.lanes[0].envelopes[0].envelope.curvature);
}

TEST(WriteEnvelopeBodyVersion, V1WriteProducesShorterEnvelopeThanV2) {
    // Byte-length assertion: a v1 body must be strictly shorter than a v2 body,
    // by exactly (kMaxEnvelopesPerLane * kMaxLanes + kMaxGlobalEnvelopes) *
    // (curvature + stepCount + kMaxStepListEntries * sizeof(float)) bytes.
    // Pre-fix, both writes produced identical byte counts because writeEnvelope
    // always emitted the v2 suffix regardless of bodyVersion.
    Buf bufV1;
    Buf bufV2;
    auto state = makeStateForRoundTrip();
    ASSERT_TRUE(poly::writeGrooveStateBody(bufV1.writeFn(), state, 1));
    ASSERT_TRUE(poly::writeGrooveStateBody(bufV2.writeFn(), state, 2));

    const size_t envSuffix =
        sizeof(float) /*curvature*/ + sizeof(int) /*stepCount*/ + poly::kMaxStepListEntries * sizeof(float);
    const size_t envCount = poly::kMaxLanes * poly::kMaxEnvelopesPerLane + poly::kMaxGlobalEnvelopes;
    EXPECT_EQ(bufV2.bytes.size(), bufV1.bytes.size() + envCount * envSuffix);
}
