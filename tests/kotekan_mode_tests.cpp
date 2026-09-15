// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M002/S01 (EC06). Kotekan interlock modes and controlled polos-sangsih
// overlap. Before this slice the engine derived sangsih as the exact
// complement of its source, which is why theory-gamelan's Rule 4 could not be
// checked as written and Rule 1 could not be checked at all: the composite was
// complete by construction, so no mutation of a static patch could make either
// predicate fail.
#include <algorithm>
#include <cstring>
#include <iterator>
#include <set>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/presets.h"
#include "poly/sanitize.h"
#include "poly/types.h"

namespace {

// The source is E(2,12) = {0, 6} deliberately. It is the smallest cycle on
// which all three modes produce different complements; for most sources two of
// the three coincide, and a test that cannot tell two modes apart proves
// nothing about either.
poly::GrooveState makePair(poly::KotekanMode mode, int overlap) {
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = 42;

    auto& polos = state.lanes[0];
    polos.id = 0;
    polos.role = poly::Role::AnchorPulse;
    polos.midiNote = 60;
    polos.cycle = {.steps = 12, .subdivision = 8};
    polos.hitCount = 2;
    polos.rotation = 0;
    polos.probability = 1.0f;
    polos.baseVelocity = 100;

    auto& sangsih = state.lanes[1];
    sangsih.id = 1;
    sangsih.role = poly::Role::Accent;
    sangsih.midiNote = 62;
    sangsih.cycle = {.steps = 12, .subdivision = 8};
    sangsih.hitCount = 2;
    sangsih.rotation = 0;
    sangsih.probability = 1.0f;
    sangsih.baseVelocity = 100;
    sangsih.kotekanSourceLane = 0;
    sangsih.kotekanMode = mode;
    sangsih.kotekanOverlap = overlap;
    return state;
}

std::vector<int> onsetsOfLane(const poly::GrooveState& state, int lane) {
    poly::Engine engine;
    poly::NoteEventBuffer notes;
    poly::EmissionEventBuffer emissions;

    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 6.0; // 12 steps at 1/8 is 6 quarter notes: exactly one cycle
    tc.tempo = 120.0;
    tc.playing = true;
    engine.renderRange(tc, state, notes, &emissions);

    std::set<int> steps;
    for (size_t i = 0; i < emissions.count; ++i) {
        const auto& e = emissions.events[i];
        if (e.laneIndex == lane && e.kind != static_cast<uint8_t>(poly::EmissionKind::Drop))
            steps.insert(static_cast<int>(e.cycleStep));
    }
    return {steps.begin(), steps.end()};
}

std::vector<int> sharedSteps(const poly::GrooveState& state) {
    const auto polos = onsetsOfLane(state, 0);
    const auto sangsih = onsetsOfLane(state, 1);
    std::vector<int> shared;
    std::set_intersection(polos.begin(), polos.end(), sangsih.begin(), sangsih.end(), std::back_inserter(shared));
    return shared;
}

} // namespace

TEST(KotekanMode, NyogCagIsTheStrictComplement) {
    EXPECT_EQ(onsetsOfLane(makePair(poly::KotekanMode::NyogCag, 0), 1),
              (std::vector<int>{1, 2, 3, 4, 5, 7, 8, 9, 10, 11}));
}

TEST(KotekanMode, TeluRepeatsOnAThreePulseCell) {
    // Task 6: the cell complement is filled wherever it would leave a pulse
    // unstruck by either part, so the composite stays continuous as Rule 1
    // requires. For this source that makes telu coincide with the strict
    // complement — the cell adds a strike only where the source sounds but the
    // cell index says it does not, which E(2,12) never does.
    EXPECT_EQ(onsetsOfLane(makePair(poly::KotekanMode::Telu, 0), 1),
              (std::vector<int>{1, 2, 3, 4, 5, 7, 8, 9, 10, 11}));
}

TEST(KotekanMode, EmpatRepeatsOnAFourPulseCell) {
    // Task 6: continuity-filled. Empat still differs from the strict complement
    // — it strikes 6, which the source also strikes, so the pair double there.
    EXPECT_EQ(onsetsOfLane(makePair(poly::KotekanMode::Empat, 0), 1),
              (std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}));
}

// Task 6. Rule 1 on theory-gamelan: "the composite must be continuous … gaps in
// the composite are errors". Before the continuity fill, Empat left pulses
// unstruck by either part. This asserts the property directly for every mode,
// so a later derivation change cannot reintroduce a gap silently.
TEST(KotekanMode, EveryModeLeavesTheCompositeContinuous) {
    for (const auto mode : {poly::KotekanMode::NyogCag, poly::KotekanMode::Telu, poly::KotekanMode::Empat}) {
        const auto state = makePair(mode, 0);
        const auto polos = onsetsOfLane(state, 0);
        const auto sangsih = onsetsOfLane(state, 1);
        for (int step = 0; step < 12; ++step) {
            const bool struck = std::find(polos.begin(), polos.end(), step) != polos.end() ||
                                std::find(sangsih.begin(), sangsih.end(), step) != sangsih.end();
            EXPECT_TRUE(struck) << "mode " << static_cast<int>(mode) << " leaves pulse " << step
                                << " unstruck by either part";
        }
    }
}

TEST(KotekanMode, StrictComplementLeavesAnEmptyIntersection) {
    // This is the property that made Rule 4 unfalsifiable: with the strict
    // complement there is nothing to find, whatever the patch says.
    EXPECT_TRUE(sharedSteps(makePair(poly::KotekanMode::NyogCag, 0)).empty());
}

TEST(KotekanMode, OverlapForcesAStructuralSharedStrike) {
    const auto shared = sharedSteps(makePair(poly::KotekanMode::NyogCag, 1));
    ASSERT_FALSE(shared.empty()) << "Rule 4's structural overlap must be reachable";
    EXPECT_EQ(shared.front(), 0) << "the cycle boundary is the first structural point";
}

TEST(KotekanMode, SanitizeClampsAnOutOfRangeMode) {
    poly::GrooveState state{};
    state.lanes[0].kotekanMode = static_cast<poly::KotekanMode>(99);
    poly::sanitizeGrooveState(state);
    EXPECT_EQ(state.lanes[0].kotekanMode, poly::KotekanMode::NyogCag);
}

TEST(KotekanMode, SanitizeClampsOverlapToTheCycle) {
    poly::GrooveState state{};
    state.lanes[0].cycle = {.steps = 8, .subdivision = 8};
    state.lanes[0].kotekanOverlap = 99;
    poly::sanitizeGrooveState(state);
    EXPECT_LE(state.lanes[0].kotekanOverlap, 8);
    state.lanes[0].kotekanOverlap = -5;
    poly::sanitizeGrooveState(state);
    EXPECT_GE(state.lanes[0].kotekanOverlap, 0);
}

// M002/S01 task 5. Balinese Kotekan is the one shipped preset named for the
// practice, so it carries the capability rather than leaving it demonstrated
// only by a test fixture. Empat rather than telu: this preset's polos is
// E(5,8), and at that cycle telu computes exactly the strict complement, so a
// patch table naming it would report a choice that changes nothing.
TEST(KotekanMode, BalineseKotekanShipsALiveInterlockStyle) {
    int index = -1;
    for (int i = 0; i < poly::kFactoryPresetCount; ++i)
        if (std::strcmp(poly::getFactoryPresetInfo(i).name, "Balinese Kotekan") == 0)
            index = i;
    ASSERT_GE(index, 0) << "no factory preset named Balinese Kotekan";

    const poly::GrooveState state = poly::makeFactoryPreset(index);
    const poly::LaneConfig& sangsih = state.lanes[1];
    ASSERT_EQ(sangsih.kotekanSourceLane, 0) << "the sangsih lane must derive from the polos";
    EXPECT_EQ(sangsih.kotekanMode, poly::KotekanMode::Empat);
    EXPECT_EQ(sangsih.kotekanOverlap, 1);

    // The property Rule 4 is about: the pair strike together somewhere.
    const auto shared = sharedSteps(state);
    EXPECT_FALSE(shared.empty()) << "polos and sangsih must share at least one structural strike";
}
