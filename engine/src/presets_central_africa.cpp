// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// Central African factory presets: the worked formulae of Simha Arom, *African
// Polyphony and Polyrhythm* (Cambridge University Press, 1991), Book VI,
// Chapter 3, "Strict polyrhythmics", pp. 229–306.
//
// Each preset plays Arom's marks and nothing else — his own reduction of a
// figure to the positions that define it (p. 253). A performer fills the cells
// between the marks and the master drummer varies; neither is transcribed, and
// inventing them would be composing.
//
// The rules these presets keep:
// - No lane varies. Probability 1, and mutation, drift, swing, humanize and
//   phrase gating all 0: a figure's identity is the position of its marks
//   (p. 253), and the movement is strictly proportional (p. 229).
// - Macros sit at identity — complexity 0.5, density 0.5, syncopation 0 — where
//   resolveMacros leaves a Euclidean lane's hits and rotation as set. That is
//   what lets the dì.kétɔ̀ lanes stay Euclidean, so turning Rotation moves the
//   "mould" (p. 298) from one formula's placement to another's.
// - Lane periods divide the formula's period (p. 231), and every mark lane
//   sounds on its own MIDI note, because formulae are recognised by which tone
//   colours coincide (p. 304).
//
// tests/central_african_preset_tests.cpp renders every preset through the
// engine and compares each lane, position by position, with Arom's grid.

#include <cstddef>

#include "poly/presets.h"

namespace poly {

namespace {

constexpr float kNoteDuration = 0.1f;

void steady(LaneConfig& lane, int id, Role role, int note, int steps, int subdivision, uint8_t velocity) {
    lane.id = id;
    lane.role = role;
    lane.midiNote = static_cast<int16_t>(note);
    lane.cycle = {steps, subdivision};
    lane.baseVelocity = velocity;
    lane.probability = 1.0f;
    lane.noteDuration = kNoteDuration;
    lane.velocitySpread = 0.0f;
    lane.emphasisProb = 0.0f;
    lane.mutationRate = 0.0f;
    lane.driftRate = 0.0f;
    lane.swingAmount = 0.0f;
    lane.humanizeMs = 0.0f;
    lane.phraseLength = 0.0f;
}

// A Euclidean lane: E(hits, steps) rotated right by `rotation`.
void euclideanLane(LaneConfig& lane, int id, Role role, int note, int steps, int subdivision, uint8_t velocity,
                   int hits, int rotation) {
    steady(lane, id, role, note, steps, subdivision, velocity);
    lane.hitCount = hits;
    lane.rotation = rotation;
}

// A drawn lane: Arom's row as a fixed pattern. 'x' is a mark, '.' is none, and
// every other character is for reading only.
void drawnLane(LaneConfig& lane, int id, Role role, int note, int subdivision, uint8_t velocity, const char* row) {
    int steps = 0;
    int hits = 0;
    lane.fixedPattern = {};
    for (const char* c = row; *c; ++c) {
        if (*c != 'x' && *c != '.')
            continue;
        if (*c == 'x') {
            lane.fixedPattern[static_cast<size_t>(steps)] = true;
            ++hits;
        }
        ++steps;
    }
    steady(lane, id, role, note, steps, subdivision, velocity);
    lane.timeline = true;
    lane.fixedPatternLength = steps;
    lane.hitCount = hits;
}

void identityMacros(GrooveState& s) {
    s.macros.complexity = 0.5f;
    s.macros.density = 0.5f;
    s.macros.syncopation = 0.0f;
    s.macros.swing = 0.0f;
    s.macros.tension = 0.0f;
    s.macros.humanize = 0.0f;
}

// General MIDI notes, one per tone colour.
constexpr int kSideStick = 37;
constexpr int kLowConga = 64;
constexpr int kHighAgogo = 67;

} // namespace

// Ngbɔ̀lù, exx. 104–105, p. 288: eight ternary pulsations, 24 positions, three
// periodicities 24 : 12 : 3. Twenty positions marked; 3 and 21 by all three.
GrooveState makeAkaNgbolu() {
    GrooveState s{};
    s.activeLaneCount = 3;
    s.seed = 288;
    euclideanLane(s.lanes[0], 0, Role::Ghost, kSideStick, 3, 8, 80, 1, 2);             // dì.kpàkpà
    drawnLane(s.lanes[1], 1, Role::AnchorPulse, kLowConga, 8, 110, "x.x|.x.|..x|.x."); // ngúé
    euclideanLane(s.lanes[2], 2, Role::Accent, kHighAgogo, 24, 8, 100, 13, 4);         // dì.kétɔ̀
    identityMacros(s);
    return s;
}

} // namespace poly
