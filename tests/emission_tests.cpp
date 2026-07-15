// M045 S01 T01: EmissionEventBuffer classification tests.
//
// Verify the engine populates EmissionEventBuffer with per-step
// Base/Ghost/Add/Drop classifications so the desk overlay can display the
// truth (mutation-added, mutation/probability-dropped, ghost-velocity hits)
// on top of the base Euclidean pattern. This closes the "Jazz Bop Ride
// snare is all over the place" bug where the display could only show the
// intended pattern, not the emitted one.

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/presets.h"
#include "poly/types.h"

namespace {

struct EmissionCounts {
    int base = 0;
    int ghost = 0;
    int add = 0;
    int drop = 0;
    int total() const { return base + ghost + add + drop; }
};

int findFactoryPresetIndex(const char* name) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        const auto& info = poly::getFactoryPresetInfo(i);
        if (info.name && std::string_view{info.name} == name)
            return i;
    }
    return -1;
}

EmissionCounts countLaneEmissions(const poly::EmissionEventBuffer& emissions, int lane) {
    EmissionCounts c{};
    for (size_t i = 0; i < emissions.count; ++i) {
        if (emissions.events[i].laneIndex != lane)
            continue;
        switch (static_cast<poly::EmissionKind>(emissions.events[i].kind)) {
        case poly::EmissionKind::Base:
            ++c.base;
            break;
        case poly::EmissionKind::Ghost:
            ++c.ghost;
            break;
        case poly::EmissionKind::Add:
            ++c.add;
            break;
        case poly::EmissionKind::Drop:
            ++c.drop;
            break;
        }
    }
    return c;
}

// Locate a lane by midiNote. Multiple lanes can share a note (e.g. Jazz Bop
// Ride has two snare lanes on 38) — take the one with the highest mutation
// rate to maximise the test signal.
int findLaneByMidiNote(const poly::GrooveState& state, int16_t midiNote) {
    int best = -1;
    float bestMut = -1.0f;
    for (int i = 0; i < state.activeLaneCount; ++i) {
        const auto& l = state.lanes[i];
        if (!l.active || l.midiNote != midiNote)
            continue;
        if (l.mutationRate > bestMut) {
            bestMut = l.mutationRate;
            best = i;
        }
    }
    return best;
}

std::vector<poly::EmissionEvent> renderBar(const poly::GrooveState& state, poly::EmissionEventBuffer& emissions) {
    poly::Engine engine;
    poly::NoteEventBuffer notes;

    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0; // one 4/4 bar
    tc.tempo = 120.0;
    tc.playing = true;

    engine.renderRange(tc, state, notes, &emissions);

    std::vector<poly::EmissionEvent> copy;
    copy.reserve(emissions.count);
    for (size_t i = 0; i < emissions.count; ++i)
        copy.push_back(emissions.events[i]);
    return copy;
}

} // namespace

TEST(EmissionClassification, NullBufferPreservesLegacyContract) {
    // renderRange with nullptr emissions is the pre-M045 contract used by
    // audio thread / plugin process / most tests. Must still work and emit
    // the same NoteEvent stream regardless of whether emissions are captured.
    int presetIdx = findFactoryPresetIndex("Jazz Bop Ride");
    ASSERT_GE(presetIdx, 0);

    auto state = poly::makeFactoryPreset(presetIdx);

    poly::Engine engine;
    poly::NoteEventBuffer notesA, notesB;
    poly::EmissionEventBuffer emissions;

    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.playing = true;

    engine.renderRange(tc, state, notesA, nullptr);
    engine.renderRange(tc, state, notesB, &emissions);

    ASSERT_EQ(notesA.count, notesB.count);
    for (size_t i = 0; i < notesA.count; ++i) {
        EXPECT_EQ(notesA.events[i].pitch, notesB.events[i].pitch);
        EXPECT_EQ(notesA.events[i].velocity, notesB.events[i].velocity);
        EXPECT_DOUBLE_EQ(notesA.events[i].ppqPosition, notesB.events[i].ppqPosition);
    }
    EXPECT_GT(emissions.count, 0u);
}

TEST(EmissionClassification, JazzBopRideSnareShowsAddAndDrop) {
    // Jazz Bop Ride snare has mutationRate = 0.25 and probability = 0.8,
    // which is exactly the scenario that caused the M045 investigation:
    // the desk shows 5 base pattern dots but the engine emits a different
    // pattern each bar. Over enough bars the classification stream must
    // include both Add events (mutation added off-pattern hits) and Drop
    // events (mutation or probability dropped on-pattern hits).
    int presetIdx = findFactoryPresetIndex("Jazz Bop Ride");
    ASSERT_GE(presetIdx, 0);

    auto state = poly::makeFactoryPreset(presetIdx);
    int snareLane = findLaneByMidiNote(state, 38);
    ASSERT_GE(snareLane, 0) << "expected a snare lane on note 38 in Jazz Bop Ride";

    poly::Engine engine;
    poly::NoteEventBuffer notes;
    poly::EmissionEventBuffer emissions;

    EmissionCounts total{};
    // Render 8 bars back-to-back. The deterministic RNG advances with
    // absStep so this covers 8x the per-bar mutation surface without
    // depending on any single bar's roll.
    for (int bar = 0; bar < 8; ++bar) {
        poly::TransportContext tc{};
        tc.ppqStart = bar * 4.0;
        tc.ppqEnd = tc.ppqStart + 4.0;
        tc.tempo = 120.0;
        tc.playing = true;
        engine.renderRange(tc, state, notes, &emissions);

        auto c = countLaneEmissions(emissions, snareLane);
        total.base += c.base;
        total.ghost += c.ghost;
        total.add += c.add;
        total.drop += c.drop;
    }

    EXPECT_GT(total.base, 0) << "snare lane should still fire base hits";
    EXPECT_GT(total.add, 0) << "mutation should have added off-pattern snare hits over 8 bars";
    EXPECT_GT(total.drop, 0) << "mutation or probability should have dropped on-pattern snare hits over 8 bars";
}

TEST(EmissionClassification, DeterministicUnderSameSeed) {
    // Truthful display depends on deterministic classification. Same preset
    // + same transport + same seed must produce byte-identical emission
    // streams across renders. If this ever flakes, the golden gate breaks
    // and the overlay will pop between visits.
    int presetIdx = findFactoryPresetIndex("Jazz Bop Ride");
    ASSERT_GE(presetIdx, 0);
    auto state = poly::makeFactoryPreset(presetIdx);

    poly::EmissionEventBuffer emissions;
    auto first = renderBar(state, emissions);
    auto second = renderBar(state, emissions);

    ASSERT_EQ(first.size(), second.size());
    for (size_t i = 0; i < first.size(); ++i) {
        EXPECT_DOUBLE_EQ(first[i].ppqPosition, second[i].ppqPosition);
        EXPECT_EQ(first[i].cycleStep, second[i].cycleStep);
        EXPECT_EQ(first[i].laneIndex, second[i].laneIndex);
        EXPECT_EQ(first[i].kind, second[i].kind);
    }
}

TEST(EmissionClassification, ZeroMutationYieldsOnlyBaseHits) {
    // A lane with mutationRate=0 and probability=1 should only ever produce
    // Base emissions — no Ghost/Add/Drop. This is the "display truth == base
    // pattern" case, which the overlay collapses to a no-op.
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;
    auto& lane = state.lanes[0];
    lane.id = 0;
    lane.midiNote = 36;
    lane.cycle = {.steps = 8, .subdivision = 8}; // 8 eighth-note steps = 1 bar
    lane.hitCount = 3;
    lane.baseVelocity = 100;
    lane.probability = 1.0f;
    lane.mutationRate = 0.0f;
    lane.emphasisProb = 1.0f;
    lane.velocitySpread = 0.0f;
    lane.ghostFloor = 0;
    lane.active = true;

    poly::EmissionEventBuffer emissions;
    renderBar(state, emissions);

    auto c = countLaneEmissions(emissions, 0);
    EXPECT_GT(c.base, 0);
    EXPECT_EQ(c.ghost, 0);
    EXPECT_EQ(c.add, 0);
    EXPECT_EQ(c.drop, 0);
    EXPECT_EQ(c.base, 3) << "8-step lane with 3 hits should emit exactly 3 base events per bar";
}
