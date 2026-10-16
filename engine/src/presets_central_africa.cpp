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
constexpr int kHandClap = 39;
constexpr int kTambourine = 54;
constexpr int kOpenHiConga = 63;
constexpr int kLowConga = 64;
constexpr int kHighAgogo = 67;
constexpr int kMaracas = 70;
constexpr int kHiWoodBlock = 76;
constexpr int kLowWoodBlock = 77;

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

// Mò.kóndí, exx. 106–107, p. 289: four parts, ratio 1:2. Twenty-one positions
// marked; only position 1 by all four. The dì.kétɔ̀ mould sits five pulsations
// from its ngbɔ̀lù placement (p. 298).
GrooveState makeAkaMokondi() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 289;
    drawnLane(s.lanes[0], 0, Role::Accent, kOpenHiConga, 8, 95, "x..|x.x|x.x|x..");              // è.ndòmbà
    drawnLane(s.lanes[1], 1, Role::Ghost, kSideStick, 8, 80, "x.x|..x|.x.|.x.|.x.|.x.|x..|x.."); // dì.kpàkpà
    drawnLane(s.lanes[2], 2, Role::AnchorPulse, kLowConga, 8, 110, "x..|xx.|.xx|x..");           // ngúé
    euclideanLane(s.lanes[3], 3, Role::Accent, kHighAgogo, 24, 8, 100, 13, 13);                  // dì.kétɔ̀
    identityMacros(s);
    return s;
}

// Yómbè, exx. 108–109, p. 290: è.ndòmbà and dì.kpàkpà play one one-pulsation
// cell, a value apart (strict, regular interweaving). Twenty-two positions marked.
GrooveState makeAkaYombe() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 290;
    euclideanLane(s.lanes[0], 0, Role::Accent, kOpenHiConga, 3, 8, 95, 1, 1);          // è.ndòmbà
    euclideanLane(s.lanes[1], 1, Role::Ghost, kSideStick, 3, 8, 80, 1, 2);             // dì.kpàkpà
    drawnLane(s.lanes[2], 2, Role::AnchorPulse, kLowConga, 8, 110, ".x.|.xx|xx.|.x."); // ngúé
    euclideanLane(s.lanes[3], 3, Role::Accent, kHighAgogo, 24, 8, 100, 13, 19);        // dì.kétɔ̀
    identityMacros(s);
    return s;
}

// Mò.nzòlì, ex. 111, p. 292: è.ndòmbà's groups of four against dì.kpàkpà's
// groups of three, the vertical hemiola. Twenty-two positions marked; none by
// all four.
GrooveState makeAkaMonzoli() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 292;
    drawnLane(s.lanes[0], 0, Role::Accent, kOpenHiConga, 8, 95, "xx..");               // è.ndòmbà
    euclideanLane(s.lanes[1], 1, Role::Ghost, kSideStick, 3, 8, 80, 1, 2);             // dì.kpàkpà
    drawnLane(s.lanes[2], 2, Role::AnchorPulse, kLowConga, 8, 110, "x..|.x.|x..|.x."); // ngúé
    euclideanLane(s.lanes[3], 3, Role::Accent, kHighAgogo, 24, 8, 100, 13, 22);        // dì.kétɔ̀
    identityMacros(s);
    return s;
}

// Mò.mbénzélé, ex. 112, pp. 292–293: mò.nzòlì's è.ndòmbà and dì.kpàkpà, the
// mould one pulsation on, and a ngúé part that alone tells the two formulae
// apart (p. 291). Its ngúé runs the full 24 values, following p. 303.
GrooveState makeAkaMombenzele() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 293;
    drawnLane(s.lanes[0], 0, Role::Accent, kOpenHiConga, 8, 95, "xx..");                               // è.ndòmbà
    euclideanLane(s.lanes[1], 1, Role::Ghost, kSideStick, 3, 8, 80, 1, 2);                             // dì.kpàkpà
    drawnLane(s.lanes[2], 2, Role::AnchorPulse, kLowConga, 8, 110, "xxx|xxx|..x|..x|..x|..x|..x|..x"); // ngúé
    euclideanLane(s.lanes[3], 3, Role::Accent, kHighAgogo, 24, 8, 100, 13, 10);                        // dì.kétɔ̀
    identityMacros(s);
    return s;
}

// Zɔ̀bɔ̀kɔ̀, exx. 113–120, pp. 294–297: the mò.kóngò's oddity figure, 3+3+2 /
// 3+2+3 / 3+3+2, against handclaps that make the pulsation audible (p. 296).
// Seventeen positions marked; 1 and 7 by all three.
GrooveState makeAkaZoboko() {
    GrooveState s{};
    s.activeLaneCount = 3;
    s.seed = 294;
    drawnLane(s.lanes[0], 0, Role::AnchorPulse, kHiWoodBlock, 8, 100, "x..|x..|x.x|..x|.x.|.x.|.x.|.x."); // mò.kóngò
    euclideanLane(s.lanes[1], 1, Role::Ghost, kHandClap, 3, 8, 75, 1, 0);                                 // handclaps
    euclideanLane(s.lanes[2], 2, Role::Accent, kHighAgogo, 24, 8, 100, 13, 19);                           // dì.kétɔ̀
    identityMacros(s);
    return s;
}

// Bòndò, exx. 101–103, pp. 286–287: four ternary pulsations. The three drum
// parts share the oddity model 3+2+2+3+2 — one rhythm, three tone colours —
// dì.kétɔ̀ splits two of its 3-groups, and the rattle is regularly commetric.
// No lane is drawn, so the factory referent lock fixes the rattle from its own
// Euclidean recipe and its marks do not move.
GrooveState makeAkaBondo() {
    GrooveState s{};
    s.activeLaneCount = 5;
    s.seed = 286;
    euclideanLane(s.lanes[0], 0, Role::Accent, kOpenHiConga, 12, 8, 95, 5, 7); // è.ndòmbà
    euclideanLane(s.lanes[1], 1, Role::Accent, kLowConga, 12, 8, 110, 5, 7);   // ngúé
    euclideanLane(s.lanes[2], 2, Role::Ghost, kSideStick, 12, 8, 80, 5, 7);    // dì.kpàkpà
    euclideanLane(s.lanes[3], 3, Role::Accent, kHighAgogo, 12, 8, 100, 7, 0);  // dì.kétɔ̀
    euclideanLane(s.lanes[4], 4, Role::AnchorPulse, kMaracas, 3, 8, 75, 1, 0); // mà.nzénzé
    identityMacros(s);
    return s;
}

// The ngbɔ̀lù drums alone, ex. 90, p. 281: four ternary pulsations, seven
// positions marked, 3 and 9 shared.
GrooveState makeAkaNgboluDrums() {
    GrooveState s{};
    s.activeLaneCount = 2;
    s.seed = 281;
    euclideanLane(s.lanes[0], 0, Role::Ghost, kSideStick, 3, 8, 80, 1, 2);             // dì.kpàkpà
    drawnLane(s.lanes[1], 1, Role::AnchorPulse, kLowConga, 8, 110, "x.x|.x.|..x|.x."); // ngúé
    identityMacros(s);
    return s;
}

// Gàzà (Banda-Dakpa), exx. 83–84, p. 278: two slit drums, two binary
// pulsations of eight values. Arom's only example of a strictly interwoven
// block: no position is shared.
GrooveState makeBandaDakpaGaza() {
    GrooveState s{};
    s.activeLaneCount = 2;
    s.seed = 278;
    drawnLane(s.lanes[0], 0, Role::AnchorPulse, kLowWoodBlock, 16, 105, "......x.|...x..x."); // husband
    euclideanLane(s.lanes[1], 1, Role::Accent, kHiWoodBlock, 16, 16, 95, 3, 8);               // mother
    identityMacros(s);
    return s;
}

// Kpóníngbó (Zande), ex. 89, p. 281: drum and pellet bells over two binary
// pulsations of four values; only position 7 shared. No lane is drawn, so the
// factory referent lock fixes the bells from their own Euclidean recipe.
GrooveState makeZandeKponingbo() {
    GrooveState s{};
    s.activeLaneCount = 2;
    s.seed = 282;
    euclideanLane(s.lanes[0], 0, Role::Accent, kLowConga, 8, 16, 105, 3, 0);       // gúrú drum
    euclideanLane(s.lanes[1], 1, Role::AnchorPulse, kTambourine, 4, 16, 75, 1, 2); // pellet bells
    identityMacros(s);
    return s;
}

// Àgà-tɔ̀rúmɔ̃ (Banda-Linda), ex. 99, p. 285: two unaccented drums in composite
// subdivision, five values to a pulsation (p. 231). At 1/16, Tempo Mult 1.25
// puts five steps in a quarter note. Eight positions struck; 8 shared.
GrooveState makeBandaLindaAgaTorumo() {
    GrooveState s{};
    s.activeLaneCount = 2;
    s.seed = 285;
    drawnLane(s.lanes[0], 0, Role::AnchorPulse, kLowConga, 16, 105, "x.x..|xxx.."); // àkɔ̄.nə̀
    drawnLane(s.lanes[1], 1, Role::Accent, kOpenHiConga, 16, 95, "...xx|..x.x");    // kàmíngà
    for (int lane = 0; lane < s.activeLaneCount; ++lane)
        s.lanes[static_cast<size_t>(lane)].tempoMultiplier = 1.25f;
    identityMacros(s);
    return s;
}

} // namespace poly
