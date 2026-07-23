#include <gtest/gtest.h>

#include "poly/types.h"
#include "poly/wasm_api.h"

// Regression test for the M049 S01 / E1 finding from the 2026-07-16 product review.
//
// Bug: poly_action_set_envelope(lane, index=N, ...) with envelopeCount==0 grew
// envelopeCount to N+1 without initializing the intervening slots [0, N). The
// default EnvelopeAssign has active=true / target=Velocity / depth=1.0 /
// periodBars=4.0, so the engine's read loop then accumulated (N) phantom
// full-depth velocity LFOs alongside the one envelope the caller asked for.
//
// Fix: gap slots between the previous count and the newly-written index are
// explicitly initialized as inactive (active=false, zeroed Envelope) so the
// read loop's "iterate up to envelopeCount; skip inactive" contract holds.

namespace {

TEST(WasmApiEnvelopeGapSlot, WritingIndex2FromEmptyLeavesLowerSlotsInactive) {
    PolyContext ctx = poly_create();
    ASSERT_NE(ctx, nullptr);

    // Write only slot 2 on lane 0. Slots 0 and 1 must not become active
    // phantom envelopes as a side effect.
    poly_action_set_envelope(ctx,
                             /*lane=*/0,
                             /*index=*/2,
                             /*target=*/static_cast<int>(poly::EnvTarget::Density),
                             /*period=*/8.0f,
                             /*depth=*/0.5f,
                             /*active=*/1);

    EXPECT_EQ(poly_lane_envelope_count(ctx, 0), 3);
    EXPECT_EQ(poly_lane_envelope_active(ctx, 0, 0), 0);
    EXPECT_EQ(poly_lane_envelope_active(ctx, 0, 1), 0);
    EXPECT_EQ(poly_lane_envelope_active(ctx, 0, 2), 1);

    poly_destroy(ctx);
}

TEST(WasmApiEnvelopeGapSlot, WritingIndex1AfterIndex0LeavesNoGap) {
    // Sequential writes must remain a no-op for lower slots — no gap to fill.
    PolyContext ctx = poly_create();
    ASSERT_NE(ctx, nullptr);

    poly_action_set_envelope(ctx, 0, 0, static_cast<int>(poly::EnvTarget::Velocity), 4.0f, 0.75f, 1);
    poly_action_set_envelope(ctx, 0, 1, static_cast<int>(poly::EnvTarget::Density), 2.0f, 0.5f, 1);

    EXPECT_EQ(poly_lane_envelope_count(ctx, 0), 2);
    EXPECT_EQ(poly_lane_envelope_active(ctx, 0, 0), 1);
    EXPECT_EQ(poly_lane_envelope_active(ctx, 0, 1), 1);
    EXPECT_EQ(static_cast<poly::EnvTarget>(poly_lane_envelope_target(ctx, 0, 0)), poly::EnvTarget::Velocity);
    EXPECT_EQ(static_cast<poly::EnvTarget>(poly_lane_envelope_target(ctx, 0, 1)), poly::EnvTarget::Density);

    poly_destroy(ctx);
}

TEST(WasmApiEnvelopeGapSlot, WritingIndex3FromCountOneFillsBothGapSlots) {
    // Two-slot gap: after writing index 0 then jumping to index 3, slots 1 and 2
    // are both intervening gaps and must be inactive.
    PolyContext ctx = poly_create();
    ASSERT_NE(ctx, nullptr);

    poly_action_set_envelope(ctx, 0, 0, static_cast<int>(poly::EnvTarget::Velocity), 4.0f, 0.75f, 1);
    poly_action_set_envelope(ctx, 0, 3, static_cast<int>(poly::EnvTarget::AccentBias), 1.0f, 0.25f, 1);

    EXPECT_EQ(poly_lane_envelope_count(ctx, 0), 4);
    EXPECT_EQ(poly_lane_envelope_active(ctx, 0, 0), 1);
    EXPECT_EQ(poly_lane_envelope_active(ctx, 0, 1), 0);
    EXPECT_EQ(poly_lane_envelope_active(ctx, 0, 2), 0);
    EXPECT_EQ(poly_lane_envelope_active(ctx, 0, 3), 1);

    poly_destroy(ctx);
}

} // namespace
