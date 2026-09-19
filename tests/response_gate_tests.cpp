// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M002/S04 (GP06). Call-and-response phrase coupling.
//
// 15-compositional-grammar describes the manual recipe: "identical Length and
// Gap values but different Offsets". That is fragile -- change one lane's
// phrase settings and the antiphony breaks silently, because there is no
// structural relationship between the lanes. Kotekan couples patterns; this
// couples phrasing.
#include <gtest/gtest.h>

#include "poly/types.h"

using namespace poly;

namespace {

LaneConfig caller(float length, float gap) {
    LaneConfig cfg{};
    cfg.id = 0;
    cfg.phraseLength = length;
    cfg.phraseGap = gap;
    return cfg;
}

LaneConfig responder(int source, float leadIn = 0.0f) {
    LaneConfig cfg{};
    cfg.id = 1;
    cfg.responseSourceLane = source;
    cfg.responseLeadIn = leadIn;
    return cfg;
}

} // namespace

// The whole claim: the response is open exactly when the call is closed,
// across a full cycle rather than at one instant.
TEST(ResponseGate, IsTheComplementOfTheSourcesGate) {
    const LaneConfig call = caller(2.0f, 2.0f); // open [0,2), closed [2,4)
    const LaneConfig response = responder(0);

    for (double ppq = 0.0; ppq < 8.0; ppq += 0.25) {
        const bool callOpen = phraseGateOpenFor(call, ppq);
        const bool responseOpen = responseGateOpen(response, call, ppq);
        EXPECT_NE(callOpen, responseOpen) << "ppq " << ppq << ": both lanes agree, which is not antiphony";
    }
}

// Both signs, because one would pass by magnitude alone.
TEST(ResponseGate, LeadInOpensEarlyAndOverlapOpensLate) {
    const LaneConfig call = caller(2.0f, 2.0f);

    // Just before the call closes at ppq 2.0, a lead-in should already be open.
    EXPECT_FALSE(responseGateOpen(responder(0, 0.0f), call, 1.75));
    EXPECT_TRUE(responseGateOpen(responder(0, 0.5f), call, 1.75)) << "a lead-in must open early";

    // Just after the call closes, a negative lead-in (overlap) should not yet
    // have opened.
    EXPECT_TRUE(responseGateOpen(responder(0, 0.0f), call, 2.25));
    EXPECT_FALSE(responseGateOpen(responder(0, -0.5f), call, 2.25)) << "an overlap must open late";
}

// The failure the manual recipe has, and the reason this row exists: change the
// caller's phrase length and the antiphony must survive.
TEST(ResponseGate, SurvivesAChangeToTheSourcesPhraseLength) {
    const LaneConfig response = responder(0);
    for (float length : {1.0f, 2.0f, 3.5f}) {
        const LaneConfig call = caller(length, 2.0f);
        for (double ppq = 0.0; ppq < 8.0; ppq += 0.25) {
            EXPECT_NE(phraseGateOpenFor(call, ppq), responseGateOpen(response, call, ppq))
                << "length " << length << ", ppq " << ppq;
        }
    }
}

// An ungated caller has no closed half to answer, so there is nothing to
// invert: the responder falls back to its own settings rather than gating off
// a lane that is always open.
TEST(ResponseGate, FallsBackWhenTheSourceIsUngated) {
    const LaneConfig call = caller(0.0f, 0.0f); // no phrase gating at all
    const LaneConfig response = responder(0);
    for (double ppq = 0.0; ppq < 4.0; ppq += 0.5)
        EXPECT_TRUE(responseGateOpen(response, call, ppq)) << "ppq " << ppq;
}

// --- Render-level and the guard ---

#include <algorithm>
#include <vector>

#include "poly/engine.h"
#include "poly/presets.h"

namespace {

std::vector<double> laneOnsets(const GrooveState& state, int laneIndex) {
    Engine engine;
    NoteEventBuffer notes;
    TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 8.0;
    tc.tempo = 120.0;
    tc.playing = true;
    engine.renderRange(tc, state, notes, nullptr);
    std::vector<double> out;
    for (size_t i = 0; i < notes.count; ++i)
        if (notes.events[i].laneIndex == laneIndex)
            out.push_back(notes.events[i].ppqPosition);
    std::sort(out.begin(), out.end());
    return out;
}

GrooveState callAndResponse(int responseSource, int callBackReference = -1) {
    GrooveState state{};
    state.activeLaneCount = 2;

    auto& call = state.lanes[0];
    call.id = 0;
    call.cycle = {16, 16};
    call.hitCount = 16;
    call.probability = 1.0f;
    call.baseVelocity = 100;
    call.phraseLength = 2.0f;
    call.phraseGap = 2.0f;
    call.responseSourceLane = callBackReference;

    auto& response = state.lanes[1];
    response.id = 1;
    response.midiNote = 38;
    response.cycle = {16, 16};
    response.hitCount = 16;
    response.probability = 1.0f;
    response.baseVelocity = 90;
    response.responseSourceLane = responseSource;
    return state;
}

} // namespace

// Through the render path: the two lanes never sound together.
TEST(ResponseGateRender, TheTwoLanesNeverOverlap) {
    const GrooveState state = callAndResponse(0);
    const auto call = laneOnsets(state, 0);
    const auto response = laneOnsets(state, 1);

    ASSERT_FALSE(call.empty());
    ASSERT_FALSE(response.empty());
    for (double c : call)
        for (double r : response)
            ASSERT_GT(std::abs(c - r), 1e-9) << "both lanes sound at ppq " << c;
}

// Two lanes naming each other must not silence both, and must not recurse.
//
// The guard M002/S01 built is reused here, but a probe removing it leaves this
// case green -- and that is worth stating rather than hiding. What actually
// prevents recursion is the implementation's choice to read the source's OWN
// phrase gate, never its response gate: the recursion has nowhere to go. The
// guard is belt-and-braces on top of that, and the next case pins the property
// doing the real work.
TEST(ResponseGateRender, AMutualReferenceIsRefused) {
    const GrooveState state = callAndResponse(0, 1); // lane 1 names 0, lane 0 names 1
    const auto call = laneOnsets(state, 0);
    const auto response = laneOnsets(state, 1);

    EXPECT_FALSE(call.empty()) << "the call must still sound";
    EXPECT_FALSE(response.empty()) << "the response must fall back, not go silent";
}

TEST(ResponseGateRender, EveryFactoryPresetIsUnmovedWithNoResponseSet) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        const poly::GrooveState preset = poly::makeFactoryPreset(i);
        const char* name = poly::getFactoryPresetInfo(i).name;
        for (int lane = 0; lane < preset.activeLaneCount; ++lane)
            ASSERT_EQ(preset.lanes[static_cast<size_t>(lane)].responseSourceLane, -1)
                << "preset " << i << " (" << name << ") lane " << lane << " ships a response reference";
    }
}

// The property that makes a mutual reference harmless, pinned directly: a
// response lane reads its source's OWN gate, so a chain of references cannot
// recurse however it is wired. Without this, someone later "simplifying"
// responseGateOpen to consult the source's response gate would introduce
// unbounded recursion and only the guard would stand between the engine and a
// stack overflow.
TEST(ResponseGate, ReadsTheSourcesOwnGateNotItsResponseGate) {
    LaneConfig a = caller(2.0f, 2.0f);
    a.id = 0;
    a.responseSourceLane = 1; // a names b
    LaneConfig b = caller(2.0f, 2.0f);
    b.id = 1;
    b.responseSourceLane = 0; // and b names a

    // If responseGateOpen consulted the source's RESPONSE gate this would not
    // terminate. That it returns at all is the property.
    const bool aOpen = responseGateOpen(a, b, 1.0);
    const bool bOpen = responseGateOpen(b, a, 1.0);

    // And the values are the plain complements of each other's own gates.
    EXPECT_EQ(aOpen, !phraseGateOpenFor(b, 1.0));
    EXPECT_EQ(bOpen, !phraseGateOpenFor(a, 1.0));
}
