// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "poly/types.h"
#include "poly/wasm_api.h"

// central-africa M006/S02: the WASM API carries the four engine features the
// browser host dropped -- Fill Every N, Seed Lock, the active lane count and
// Fill Now. Each mirrors the plugin's handling in processor.cpp
// (kCoreFillEveryN, kCoreSeedLock, kActiveLaneCount, kFillManualTrigger).

namespace {

// Appended to the end of LaneFieldInt in wasm_api.cpp.
constexpr int kFieldFillEveryNBars = 15;
constexpr int kFieldSeedLocked = 16;
constexpr int kFieldsPerEvent = 6;

// Render `quarters` one-quarter passes from ppq `from`; return one string per
// event: ppq, note, velocity, lane.
std::vector<std::string> render(PolyContext ctx, int from, int quarters) {
    std::vector<std::string> out;
    for (int q = from; q < from + quarters; ++q) {
        const int n = poly_render(ctx, q, q + 1, 120.0, 44100.0, 512, 1, 0, 0.0, 0.0, q == from ? 1 : 0);
        const double* ev = poly_event_buffer(ctx);
        for (int i = 0; i < n; ++i) {
            const double* e = ev + i * kFieldsPerEvent;
            out.push_back(std::to_string(e[0]) + ":" + std::to_string(static_cast<int>(e[1])) + ":" +
                          std::to_string(e[2]) + ":" + std::to_string(static_cast<int>(e[5])));
        }
    }
    return out;
}

std::vector<std::string> laneEvents(const std::vector<std::string>& events, int lane) {
    std::vector<std::string> out;
    const std::string suffix = ":" + std::to_string(lane);
    for (const auto& e : events)
        if (e.size() >= suffix.size() && e.compare(e.size() - suffix.size(), suffix.size(), suffix) == 0)
            out.push_back(e);
    return out;
}

// Aka: Ngbolu, whose macros leave its Euclidean lanes as written (the default
// macros recompute a Euclidean lane's hits, which would hide an edit). Its
// lane 0, E(1,3), has off-pattern steps for a fill to add; probability below 1
// on lanes 0 and 1 puts the seed into their output.
PolyContext sparseContext(uint64_t seed = 111) {
    PolyContext ctx = poly_create();
    int index = -1;
    for (int i = 0; i < poly_preset_count(); ++i)
        if (std::string(poly_preset_name(i)) == "Aka: Ngbolu")
            index = i;
    EXPECT_GE(index, 0) << "no factory preset named Aka: Ngbolu";
    poly_load_preset(ctx, index);
    poly_edit_lane_float(ctx, 0, /*Probability*/ 0, 0.6);
    poly_edit_lane_float(ctx, 1, /*Probability*/ 0, 0.6);
    poly_set_seed(ctx, seed);
    return ctx;
}

TEST(WasmApiM006, FillEveryNBarsRoundTripsAndClamps) {
    PolyContext ctx = poly_create();
    poly_edit_lane_int(ctx, 0, kFieldFillEveryNBars, 4);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldFillEveryNBars), 4);
    poly_edit_lane_int(ctx, 0, kFieldFillEveryNBars, 2000);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldFillEveryNBars), 1024);
    poly_edit_lane_int(ctx, 0, kFieldFillEveryNBars, -3);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldFillEveryNBars), 0);
    poly_destroy(ctx);
}

TEST(WasmApiM006, FillEveryBarChangesTheOutput) {
    PolyContext plain = sparseContext();
    PolyContext filled = sparseContext();
    poly_edit_lane_int(filled, 0, kFieldFillEveryNBars, 1);
    EXPECT_NE(laneEvents(render(plain, 0, 4), 0), laneEvents(render(filled, 0, 4), 0));
    poly_destroy(plain);
    poly_destroy(filled);
}

TEST(WasmApiM006, SeedLockCapturesTheSeedOnTheEdgeAndHoldsTheLane) {
    PolyContext ctx = sparseContext();
    poly_edit_lane_int(ctx, 0, kFieldSeedLocked, 1);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldSeedLocked), 1);
    const auto before = render(ctx, 0, 8);
    poly_set_seed(ctx, 222);
    const auto after = render(ctx, 0, 8);
    EXPECT_EQ(laneEvents(before, 0), laneEvents(after, 0)) << "a locked lane changed with the global seed";
    EXPECT_NE(laneEvents(before, 1), laneEvents(after, 1)) << "the unlocked lane did not re-roll";

    // Unlock and relock after the seed change: the lane now holds seed 222, so
    // under seed 333 it plays what an unlocked lane plays under seed 222.
    poly_edit_lane_int(ctx, 0, kFieldSeedLocked, 0);
    poly_edit_lane_int(ctx, 0, kFieldSeedLocked, 1);
    poly_set_seed(ctx, 333);
    const auto relocked = render(ctx, 0, 8);
    PolyContext at222 = sparseContext(222);
    EXPECT_EQ(laneEvents(render(at222, 0, 8), 0), laneEvents(relocked, 0)) << "relocking did not capture seed 222";
    poly_destroy(at222);
    poly_destroy(ctx);
}

TEST(WasmApiM006, ActiveLaneCountClampsAndLimitsTheLanesThatPlay) {
    PolyContext ctx = poly_create();
    poly_set_active_lane_count(ctx, 2);
    EXPECT_EQ(poly_active_lane_count(ctx), 2);
    for (const auto& e : render(ctx, 0, 8)) {
        const int lane = std::stoi(e.substr(e.rfind(':') + 1));
        EXPECT_LT(lane, 2) << "lane " << lane << " played with an active lane count of 2";
    }
    poly_set_active_lane_count(ctx, 0);
    EXPECT_EQ(poly_active_lane_count(ctx), 1);
    poly_set_active_lane_count(ctx, 99);
    EXPECT_EQ(poly_active_lane_count(ctx), poly::kMaxLanes);
    poly_destroy(ctx);
}

TEST(WasmApiM006, ManualFillCoversExactlyOneRenderPass) {
    PolyContext plain = sparseContext();
    PolyContext filled = sparseContext();
    poly_trigger_manual_fill(filled);
    // The pass after the trigger is a fill; the pass after that is not.
    EXPECT_NE(laneEvents(render(plain, 0, 1), 0), laneEvents(render(filled, 0, 1), 0));
    EXPECT_EQ(laneEvents(render(plain, 1, 1), 0), laneEvents(render(filled, 1, 1), 0));
    poly_destroy(plain);
    poly_destroy(filled);
}

} // namespace
