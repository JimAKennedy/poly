// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M003/S01 (GP07). A lane can carry a sequence of pitches.
//
// A lane emits one fixed pitch: `ev.pitch = cfg.midiNote`, a single assignment.
// Hand-drum traditions are one voice with several strokes — djembe bass/tone/
// slap, tabla bols, conga open/muted/slap — so each articulation currently
// needs its own lane, competing for the eight-lane budget and unable to share a
// pattern. A sequence also makes a lane a pitched voice, so the polymetric
// machinery applies to melodic material.
//
// Two decisions from M003-decisions.md are load-bearing here. Per-note
// durations set GATE LENGTH ONLY — the step grid still decides when notes
// start, so this does not become a second mechanism for placing onsets beside
// subdivisionProfile. And pitch is indexed by ABSOLUTE STEP POSITION, not by
// hit ordinal, so a locate reproduces it and a dropped hit does not re-voice
// everything after it.
#include <array>

#include <gtest/gtest.h>

#include "poly/types.h"

using namespace poly;

namespace {

LaneConfig sequencedLane(std::initializer_list<int16_t> pitches) {
    LaneConfig cfg{};
    cfg.id = 0;
    cfg.cycle = {16, 16};
    cfg.midiNote = 36;
    int i = 0;
    for (int16_t p : pitches) {
        cfg.noteSequence[static_cast<size_t>(i)].pitch = p;
        ++i;
    }
    cfg.noteSequenceLength = i;
    return cfg;
}

} // namespace

// The byte-identity guarantee, as arithmetic: no sequence means midiNote.
TEST(NoteSequence, AnEmptySequenceLeavesTheLanesSinglePitch) {
    LaneConfig cfg{};
    cfg.midiNote = 42;
    for (int64_t step : {int64_t{0}, int64_t{5}, int64_t{-3}})
        EXPECT_EQ(noteSequencePitch(cfg, step), 42) << "step " << step;
}

// Indexed by position, asserted across more than one wrap so an off-by-one at
// the boundary shows rather than hiding in the first cycle.
TEST(NoteSequence, SuppliesPitchesByAbsoluteStepPosition) {
    const LaneConfig cfg = sequencedLane({60, 62, 64});
    const int16_t expected[] = {60, 62, 64, 60, 62, 64, 60};
    for (int64_t step = 0; step < 7; ++step)
        EXPECT_EQ(noteSequencePitch(cfg, step), expected[step]) << "step " << step;
}

// The device #245 asks for, and the reason indexing is positional: a five-note
// sequence over a seven-step cycle does not repeat cycle to cycle.
TEST(NoteSequence, FiveNotesOverSevenStepsPhases) {
    const LaneConfig cfg = sequencedLane({60, 62, 64, 65, 67});
    bool anyDiffer = false;
    for (int64_t step = 0; step < 7; ++step)
        if (noteSequencePitch(cfg, step) != noteSequencePitch(cfg, step + 7))
            anyDiffer = true;
    EXPECT_TRUE(anyDiffer) << "the sequence must not realign with the cycle after one bar";
}

// A negative absolute step happens whenever the transport sits before zero;
// it must wrap rather than index out of bounds.
TEST(NoteSequence, NegativeStepsWrapRatherThanReadOutOfBounds) {
    const LaneConfig cfg = sequencedLane({60, 62, 64});
    EXPECT_EQ(noteSequencePitch(cfg, -1), 64);
    EXPECT_EQ(noteSequencePitch(cfg, -3), 60);
    EXPECT_EQ(noteSequencePitch(cfg, -4), 64);
}

// A length beyond the array's bound is refused rather than read past the end.
TEST(NoteSequence, RefusesALengthBeyondTheArray) {
    LaneConfig cfg = sequencedLane({60, 62});
    cfg.noteSequenceLength = kMaxNoteSequence + 5;
    EXPECT_EQ(noteSequencePitch(cfg, 0), cfg.midiNote) << "an impossible length must fall back";
}

// --- Render-level ---
//
// Written before the wiring and before any probe, per this milestone's standing
// instructions: M002/S02's probe killed nothing because every case exercised
// the helper directly, and the render case it was missing found a real wiring
// defect the moment it existed.

#include <algorithm>
#include <vector>

#include "poly/engine.h"
#include "poly/presets.h"

namespace {

GrooveState laneWithSequence(std::initializer_list<int16_t> pitches, float entryDuration = 0.0f) {
    GrooveState state{};
    state.activeLaneCount = 1;
    auto& lane = state.lanes[0];
    lane = sequencedLane(pitches);
    lane.hitCount = 16;
    lane.probability = 1.0f;
    lane.baseVelocity = 100;
    if (entryDuration > 0.0f)
        for (int i = 0; i < lane.noteSequenceLength; ++i)
            lane.noteSequence[static_cast<size_t>(i)].durationBeats = entryDuration;
    return state;
}

std::vector<NoteEvent> render(const GrooveState& state, double to = 1.0) {
    Engine engine;
    NoteEventBuffer notes;
    TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = to;
    tc.tempo = 120.0;
    tc.playing = true;
    engine.renderRange(tc, state, notes, nullptr);
    std::vector<NoteEvent> out(notes.events.begin(), notes.events.begin() + static_cast<long>(notes.count));
    std::sort(out.begin(), out.end(),
              [](const NoteEvent& a, const NoteEvent& b) { return a.ppqPosition < b.ppqPosition; });
    return out;
}

} // namespace

TEST(NoteSequenceRender, EmittedPitchesFollowTheSequence) {
    const auto events = render(laneWithSequence({60, 62, 64}));
    ASSERT_GE(events.size(), 4u);
    const int16_t expected[] = {60, 62, 64, 60};
    for (size_t i = 0; i < 4; ++i)
        EXPECT_EQ(events[i].pitch, expected[i]) << "note " << i;
}

TEST(NoteSequenceRender, ALaneWithNoSequenceStillEmitsItsSinglePitch) {
    GrooveState state = laneWithSequence({60, 62, 64});
    state.lanes[0].noteSequenceLength = 0;
    for (const auto& e : render(state))
        EXPECT_EQ(e.pitch, state.lanes[0].midiNote);
}

// Gate length only: the ONSETS must be exactly where they were, whatever the
// per-note durations say. This is the decision that keeps pitch orthogonal to
// placement, and it is the case that would catch a regression turning the lane
// into a sequencer.
TEST(NoteSequenceRender, PerNoteDurationsChangeLengthButNotOnsets) {
    const auto plain = render(laneWithSequence({60, 62, 64}));
    const auto durated = render(laneWithSequence({60, 62, 64}, 0.75f));

    ASSERT_EQ(plain.size(), durated.size());
    for (size_t i = 0; i < plain.size(); ++i)
        EXPECT_NEAR(plain[i].ppqPosition, durated[i].ppqPosition, 1e-12)
            << "note " << i << " moved; durations must not advance the clock";

    ASSERT_FALSE(durated.empty());
    EXPECT_GT(durated[0].duration, plain[0].duration) << "but the gate length must change";
}

// The milestone's back-compatibility rule for this slice's field.
TEST(NoteSequenceRender, EveryFactoryPresetIsUnmovedWithNoSequenceSet) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        const poly::GrooveState preset = poly::makeFactoryPreset(i);
        const char* name = poly::getFactoryPresetInfo(i).name;
        for (int lane = 0; lane < preset.activeLaneCount; ++lane)
            ASSERT_EQ(preset.lanes[static_cast<size_t>(lane)].noteSequenceLength, 0)
                << "preset " << i << " (" << name << ") lane " << lane << " ships a sequence";
    }
}

// --- Existing lane features still apply ---
//
// The slice's third definition-of-done item asks for at least two. These are
// chosen because they could plausibly break, not because they are easy: drift
// changes which step index a position maps to, and kotekan derives the pattern
// from another lane. Either could have been written so that it overrode the
// sequence, or so that the sequence overrode it, and only a test distinguishes
// "they compose" from "one happens to win".

// Drift rotates the step index a position maps to. The pitch is indexed by
// ABSOLUTE step, so drift must move which STEPS sound without moving which
// pitch a given position takes -- the two are independent axes and both must
// still work.
TEST(NoteSequenceFeatures, DriftAndTheSequenceCompose) {
    GrooveState still = laneWithSequence({60, 62, 64});
    still.lanes[0].hitCount = 4;

    GrooveState drifting = still;
    drifting.lanes[0].driftRate = 2.0f;

    const auto a = render(still, 4.0);
    const auto b = render(drifting, 4.0);
    ASSERT_FALSE(a.empty());
    ASSERT_FALSE(b.empty());

    // Drift changed which steps sound.
    bool onsetsDiffer = a.size() != b.size();
    for (size_t i = 0; i < std::min(a.size(), b.size()) && !onsetsDiffer; ++i)
        if (std::abs(a[i].ppqPosition - b[i].ppqPosition) > 1e-9)
            onsetsDiffer = true;
    EXPECT_TRUE(onsetsDiffer) << "drift must move the pattern, or this proves nothing";

    // And every emitted pitch still comes from the sequence, at the position
    // it sounded -- not from midiNote, and not frozen at one entry.
    for (const auto& e : b) {
        const int64_t step = static_cast<int64_t>(std::llround(e.ppqPosition / 0.25));
        EXPECT_EQ(e.pitch, noteSequencePitch(drifting.lanes[0], step))
            << "pitch at ppq " << e.ppqPosition << " did not follow the sequence";
    }
}

// Kotekan derives a lane's PATTERN from another lane. The pitch source is a
// different axis, so a sequenced kotekan lane must play the complement's steps
// with the sequence's pitches. A test checking only one of the two would miss
// the other silently.
TEST(NoteSequenceFeatures, KotekanPatternAndTheSequenceAreIndependentAxes) {
    GrooveState state{};
    state.activeLaneCount = 2;

    auto& polos = state.lanes[0];
    polos.id = 0;
    polos.cycle = {8, 8};
    polos.hitCount = 5;
    polos.probability = 1.0f;
    polos.baseVelocity = 100;
    polos.midiNote = 70;

    auto& sangsih = state.lanes[1];
    sangsih.id = 1;
    sangsih.cycle = {8, 8};
    sangsih.hitCount = 5;
    sangsih.probability = 1.0f;
    sangsih.baseVelocity = 100;
    sangsih.midiNote = 72;
    sangsih.kotekanSourceLane = 0;
    sangsih.noteSequence[0].pitch = 80;
    sangsih.noteSequence[1].pitch = 82;
    sangsih.noteSequenceLength = 2;

    const auto events = render(state, 4.0);
    std::vector<double> polosSteps, sangsihSteps;
    for (const auto& e : events) {
        if (e.laneIndex == 0)
            polosSteps.push_back(e.ppqPosition);
        else {
            sangsihSteps.push_back(e.ppqPosition);
            // Pitch comes from the sequence, never from midiNote.
            const int64_t step = static_cast<int64_t>(std::llround(e.ppqPosition / 0.5));
            EXPECT_EQ(e.pitch, noteSequencePitch(sangsih, step)) << "ppq " << e.ppqPosition;
            EXPECT_NE(e.pitch, sangsih.midiNote) << "the sequence must override the single pitch";
        }
    }

    // And the pattern is still the complement: the pair never strike together.
    ASSERT_FALSE(polosSteps.empty());
    ASSERT_FALSE(sangsihSteps.empty());
    for (double p : polosSteps)
        for (double s : sangsihSteps)
            ASSERT_GT(std::abs(p - s), 1e-9) << "polos and sangsih both sound at " << p;
}
