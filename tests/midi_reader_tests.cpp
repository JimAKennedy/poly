// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M035 S02 T01: poly::parseSMF reader. Test suite name is MidiReader so
// `ctest -R MidiReader` selects exactly these cases. Covers the write->read->fit
// round trip (the authoritative import path), running status, tempo meta,
// velocity-0-as-note-off, format-1 multi-track merging, and the no-throw /
// valid-flag contract for malformed input.
#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <vector>

#include <gtest/gtest.h>

#include "poly/euclidean.h"
#include "poly/fitter.h"
#include "poly/midi_reader.h"
#include "poly/smf_writer.h"
#include "poly/types.h"
#include "poly/wasm_api.h"

namespace {

// Onset PPQ positions a pure Euclidean rhythm E(k,n) at the given subdivision
// and rotation produces over one cycle starting at PPQ 0 — mirrors the helper in
// fitter_tests.cpp so a round-tripped fit is directly comparable.
std::vector<double> euclideanOnsets(int k, int n, int rotation, int subdivision) {
    std::array<bool, poly::kMaxSteps> pattern{};
    poly::euclidean(k, n, rotation, pattern);
    const double g = 4.0 / static_cast<double>(subdivision);
    std::vector<double> onsets;
    for (int i = 0; i < n; ++i) {
        if (pattern[static_cast<size_t>(i)])
            onsets.push_back(static_cast<double>(i) * g);
    }
    return onsets;
}

double loopLength(int n, int subdivision) {
    return static_cast<double>(n) * (4.0 / static_cast<double>(subdivision));
}

// Build NoteEvents for one full cycle of E(k,n). Each note lasts one step,
// EXCEPT the last-in-time note, which is sustained to the exact cycle boundary
// (n*g) so the written file's final note-off encodes the full loop length. This
// is how an exported loop conveys trailing rests the onsets alone cannot (the
// fitter's loopLengthPpq contract) — a note ringing to loop end.
std::vector<poly::NoteEvent> euclideanEvents(int k, int n, int rotation, int subdivision) {
    const double g = 4.0 / static_cast<double>(subdivision);
    const auto onsets = euclideanOnsets(k, n, rotation, subdivision);
    std::vector<poly::NoteEvent> events;
    double maxOnset = 0.0;
    for (double o : onsets)
        maxOnset = std::max(maxOnset, o);
    for (double o : onsets) {
        poly::NoteEvent ev;
        ev.ppqPosition = o;
        ev.pitch = 36;
        ev.velocity = 0.8f;
        ev.duration = (o == maxOnset) ? (loopLength(n, subdivision) - o) : g;
        ev.channel = 0;
        ev.laneIndex = 0;
        events.push_back(ev);
    }
    return events;
}

// Assert a full write->parse->fit round trip recovers the source Euclidean
// triple exactly. Spellings used are aperiodic (gcd(k,n)==1) so rotation is
// unique, matching fitter_tests.cpp.
void expectRoundTrip(int k, int n, int rotation, int subdivision) {
    const auto events = euclideanEvents(k, n, rotation, subdivision);
    const auto bytes = poly::writeSMF(events.data(), events.size(), 120.0);

    const poly::MidiParseResult parsed = poly::parseSMF(bytes);
    ASSERT_TRUE(parsed.valid) << "k=" << k << " n=" << n << " r=" << rotation << " sub=" << subdivision;
    ASSERT_EQ(parsed.onsetsPpq.size(), static_cast<size_t>(k));
    EXPECT_DOUBLE_EQ(parsed.loopLengthPpq, loopLength(n, subdivision));

    const poly::FitResult fit = poly::fitEuclidean(parsed.onsetsPpq, parsed.loopLengthPpq);
    EXPECT_TRUE(fit.valid);
    EXPECT_EQ(fit.steps, n) << "k=" << k << " n=" << n << " r=" << rotation << " sub=" << subdivision;
    EXPECT_EQ(fit.subdivision, subdivision) << "k=" << k << " n=" << n << " r=" << rotation;
    EXPECT_EQ(fit.hitCount, k) << "k=" << k << " n=" << n << " r=" << rotation;
    EXPECT_EQ(fit.rotation, rotation) << "k=" << k << " n=" << n << " r=" << rotation;
    EXPECT_EQ(fit.patternMismatch, 0);
    // Not EXPECT_DOUBLE_EQ 0: routing onsets through the integer SMF tick grid
    // and back introduces a sub-quantum rounding residual (e.g. the 4/12
    // subdivision is not binary-exact), so the least-squares error is ~1e-31,
    // not bit-zero. Params still recover exactly.
    EXPECT_NEAR(fit.onsetError, 0.0, 1e-9);
}

// --- Hand-built SMF assembly (for cases writeSMF cannot emit: running status,
// custom division, deliberate truncation). ---

void appendU16BE(std::vector<uint8_t>& b, uint16_t v) {
    b.push_back(static_cast<uint8_t>(v >> 8));
    b.push_back(static_cast<uint8_t>(v));
}

void appendU32BE(std::vector<uint8_t>& b, uint32_t v) {
    b.push_back(static_cast<uint8_t>(v >> 24));
    b.push_back(static_cast<uint8_t>(v >> 16));
    b.push_back(static_cast<uint8_t>(v >> 8));
    b.push_back(static_cast<uint8_t>(v));
}

// Wrap a raw track-event body in MThd(format 0, 1 track, given division) + MTrk.
std::vector<uint8_t> makeSMF(uint16_t division, const std::vector<uint8_t>& trackBody) {
    std::vector<uint8_t> f;
    f.push_back('M');
    f.push_back('T');
    f.push_back('h');
    f.push_back('d');
    appendU32BE(f, 6);
    appendU16BE(f, 0); // format 0
    appendU16BE(f, 1); // ntrks
    appendU16BE(f, division);
    f.push_back('M');
    f.push_back('T');
    f.push_back('r');
    f.push_back('k');
    appendU32BE(f, static_cast<uint32_t>(trackBody.size()));
    f.insert(f.end(), trackBody.begin(), trackBody.end());
    return f;
}

} // namespace

// --- Round trip across canonical spellings, subdivisions, rotations ---

// Spellings the fitter provably recovers uniquely (see fitter_tests.cpp) —
// keeping to that set isolates this test to the READER: any failure here means
// parseSMF returned wrong onsets/loopLength, not a fitter ambiguity.
TEST(MidiReader, RoundTripRecoversEuclideanParameters) {
    expectRoundTrip(3, 8, 0, 8);   // tresillo
    expectRoundTrip(5, 8, 0, 8);   // cinquillo
    expectRoundTrip(5, 16, 0, 16); // son clave at sixteenths
    expectRoundTrip(7, 12, 0, 12); // 12-step, 7 hits
}

TEST(MidiReader, RoundTripRecoversRotation) {
    expectRoundTrip(3, 8, 1, 8);
    expectRoundTrip(3, 8, 2, 8);
    expectRoundTrip(5, 16, 3, 16);
    expectRoundTrip(7, 12, 5, 12);
}

// --- Running status: a channel message that omits the status byte reuses the
// previous one. Poly's writer never emits this, so it is hand-built. ---

TEST(MidiReader, HonorsRunningStatus) {
    std::vector<uint8_t> body = {
        0x00, 0x90, 0x3C, 0x40, // delta 0: note-on C, vel 64 (arms running status)
        0x60, 0x3E, 0x40,       // delta 96: note-on D, vel 64 (running status — no 0x90)
        0x00, 0xFF, 0x2F, 0x00  // end of track
    };
    const poly::MidiParseResult r = poly::parseSMF(makeSMF(480, body));
    ASSERT_TRUE(r.valid);
    ASSERT_EQ(r.onsetsPpq.size(), 2u);
    EXPECT_DOUBLE_EQ(r.onsetsPpq[0], 0.0);
    EXPECT_DOUBLE_EQ(r.onsetsPpq[1], 96.0 / 480.0);
}

// --- Note-on with velocity 0 is a note-off (running-status convention): no
// onset recorded. ---

TEST(MidiReader, VelocityZeroNoteOnIsNoteOff) {
    std::vector<uint8_t> body = {
        0x00, 0x90, 0x3C, 0x40, // note-on C
        0x30, 0x3C, 0x00,       // running status: C vel 0 -> note-off
        0x00, 0xFF, 0x2F, 0x00  // end of track
    };
    const poly::MidiParseResult r = poly::parseSMF(makeSMF(480, body));
    ASSERT_TRUE(r.valid);
    EXPECT_EQ(r.onsetsPpq.size(), 1u);
}

// --- Tempo meta drives tempoBpm; absent tempo defaults to 120. ---

TEST(MidiReader, ReadsTempoMeta) {
    // 150 BPM = 400000 us/quarter exactly.
    std::vector<uint8_t> body = {0x00, 0xFF, 0x51, 0x03, 0x06, 0x1A, 0x80, // FF 51 03 = 0x061A80 = 400000
                                 0x00, 0x90, 0x3C, 0x40,                   // a note so the file has content
                                 0x00, 0xFF, 0x2F, 0x00};
    const poly::MidiParseResult r = poly::parseSMF(makeSMF(480, body));
    ASSERT_TRUE(r.valid);
    EXPECT_DOUBLE_EQ(r.tempoBpm, 150.0);
}

TEST(MidiReader, DefaultsTempoWhenAbsent) {
    const auto events = euclideanEvents(3, 8, 0, 8);
    const auto bytes = poly::writeSMF(events.data(), events.size(), 90.0);
    const poly::MidiParseResult r = poly::parseSMF(bytes);
    ASSERT_TRUE(r.valid);
    // writeSMF DOES emit a tempo meta, so 90 BPM comes back here.
    EXPECT_NEAR(r.tempoBpm, 90.0, 0.01);
}

// --- Format-1 multi-track: note-ons merge across tracks into one sorted
// timeline. ---

TEST(MidiReader, MergesFormat1Tracks) {
    // Two lanes, distinct laneIndex -> writeMultiTrackSMF emits format 1 with a
    // conductor + one MTrk per lane.
    std::vector<poly::NoteEvent> events;
    for (int i = 0; i < 2; ++i) {
        poly::NoteEvent a;
        a.ppqPosition = static_cast<double>(i);
        a.pitch = 36;
        a.velocity = 0.8f;
        a.duration = 0.5;
        a.laneIndex = 0;
        events.push_back(a);
        poly::NoteEvent b;
        b.ppqPosition = static_cast<double>(i) + 0.5;
        b.pitch = 38;
        b.velocity = 0.8f;
        b.duration = 0.5;
        b.laneIndex = 1;
        events.push_back(b);
    }
    const auto bytes = poly::writeMultiTrackSMF(events.data(), events.size(), 120.0);
    const poly::MidiParseResult r = poly::parseSMF(bytes);
    ASSERT_TRUE(r.valid);
    ASSERT_EQ(r.onsetsPpq.size(), 4u);
    // Merged + sorted ascending: 0.0, 0.5, 1.0, 1.5.
    EXPECT_DOUBLE_EQ(r.onsetsPpq[0], 0.0);
    EXPECT_DOUBLE_EQ(r.onsetsPpq[1], 0.5);
    EXPECT_DOUBLE_EQ(r.onsetsPpq[2], 1.0);
    EXPECT_DOUBLE_EQ(r.onsetsPpq[3], 1.5);
}

// --- No-throw / valid-flag contract for malformed input ---

TEST(MidiReader, RejectsEmptyInput) {
    const poly::MidiParseResult r = poly::parseSMF(std::vector<uint8_t>{});
    EXPECT_FALSE(r.valid);
    EXPECT_TRUE(r.onsetsPpq.empty());
}

TEST(MidiReader, RejectsNonMidiBytes) {
    // A non-MIDI drop (e.g. a text/PNG file) — the key negative case for the UI.
    std::vector<uint8_t> notMidi = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '!'};
    const poly::MidiParseResult r = poly::parseSMF(notMidi);
    EXPECT_FALSE(r.valid);
    EXPECT_TRUE(r.onsetsPpq.empty());
}

TEST(MidiReader, RejectsTruncatedHeader) {
    std::vector<uint8_t> shortHeader = {'M', 'T', 'h', 'd', 0, 0, 0, 6}; // header cut off
    const poly::MidiParseResult r = poly::parseSMF(shortHeader);
    EXPECT_FALSE(r.valid);
}

TEST(MidiReader, RejectsSmpteDivision) {
    // High bit set => SMPTE frame division, unsupported by the metrical import.
    std::vector<uint8_t> body = {0x00, 0x90, 0x3C, 0x40, 0x00, 0xFF, 0x2F, 0x00};
    const poly::MidiParseResult r = poly::parseSMF(makeSMF(0xE728, body)); // -25 fps / 40 subframes
    EXPECT_FALSE(r.valid);
}

TEST(MidiReader, RejectsZeroDivision) {
    std::vector<uint8_t> body = {0x00, 0xFF, 0x2F, 0x00};
    const poly::MidiParseResult r = poly::parseSMF(makeSMF(0, body));
    EXPECT_FALSE(r.valid);
}

TEST(MidiReader, TruncatedTrackDoesNotCrash) {
    // Well-formed header, then a track whose declared length runs past EOF and
    // whose final event is cut mid-message. Must not throw / read OOB; whatever
    // was cleanly parsed before truncation is fine.
    std::vector<uint8_t> f;
    f.push_back('M');
    f.push_back('T');
    f.push_back('h');
    f.push_back('d');
    appendU32BE(f, 6);
    appendU16BE(f, 0);
    appendU16BE(f, 1);
    appendU16BE(f, 480);
    f.push_back('M');
    f.push_back('T');
    f.push_back('r');
    f.push_back('k');
    appendU32BE(f, 1000); // lies: claims 1000 bytes
    // Only a partial event follows.
    f.push_back(0x00);
    f.push_back(0x90);
    f.push_back(0x3C); // missing velocity byte -> truncated
    const poly::MidiParseResult r = poly::parseSMF(f);
    // Header + track magic were valid, so valid==true; the truncated note simply
    // yields no complete onset. The contract is "no crash", which reaching this
    // assertion proves.
    EXPECT_TRUE(r.valid);
    EXPECT_TRUE(r.onsetsPpq.empty());
}

TEST(MidiReader, HandlesGarbageAfterValidHeaderWithoutCrash) {
    std::vector<uint8_t> f;
    f.push_back('M');
    f.push_back('T');
    f.push_back('h');
    f.push_back('d');
    appendU32BE(f, 6);
    appendU16BE(f, 0);
    appendU16BE(f, 1);
    appendU16BE(f, 480);
    // Random non-chunk garbage where a track should be.
    const std::array<uint8_t, 10> garbage = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x11, 0x22};
    for (uint8_t v : garbage)
        f.push_back(v);
    const poly::MidiParseResult r = poly::parseSMF(f);
    // No valid MTrk chunk was seen -> invalid, but no crash.
    EXPECT_FALSE(r.valid);
}

// --- Atomic parse+fit+apply import (M035 S02 T02) ---------------------------
//
// Suite name ImportMidi so `ctest -R ImportMidi` selects exactly these. Covers
// the shared engine helper (importMidiToLane / applyFitToLane) AND the wasm
// C-ABI export (poly_import_midi) both UI surfaces call, plus the negative
// contract: a non-MIDI / degenerate drop returns failure and leaves the lane
// untouched.

// Field ordinals from the anonymous LaneFieldInt enum in wasm_api.cpp — kept in
// sync with that surface so the C-ABI read-back below asserts the right slots.
namespace {
constexpr int kFieldHitCount = 2;
constexpr int kFieldRotation = 3;
constexpr int kFieldSubdivision = 7;
constexpr int kFieldCycleSteps = 8;
} // namespace

TEST(ImportMidi, HelperAppliesEuclideanFitToLane) {
    // Tresillo E(3,8) at eighths -> the helper must recover steps/hits/rotation
    // and (being a clean Euclid) leave the lane in generative (non-timeline)
    // mode.
    const auto events = euclideanEvents(3, 8, 0, 8);
    const auto bytes = poly::writeSMF(events.data(), events.size(), 120.0);

    poly::LaneConfig lane; // defaults: steps=4, hitCount=4, rotation=0
    ASSERT_TRUE(poly::importMidiToLane(bytes, lane));
    EXPECT_EQ(lane.cycle.steps, 8);
    EXPECT_EQ(lane.cycle.subdivision, 8);
    EXPECT_EQ(lane.hitCount, 3);
    EXPECT_EQ(lane.rotation, 0);
    EXPECT_FALSE(lane.timeline);
}

TEST(ImportMidi, HelperRecoversRotation) {
    const auto events = euclideanEvents(5, 16, 3, 16);
    const auto bytes = poly::writeSMF(events.data(), events.size(), 120.0);

    poly::LaneConfig lane;
    ASSERT_TRUE(poly::importMidiToLane(bytes, lane));
    EXPECT_EQ(lane.cycle.steps, 16);
    EXPECT_EQ(lane.hitCount, 5);
    EXPECT_EQ(lane.rotation, 3);
}

TEST(ImportMidi, HelperRejectsNonMidiLeavesLaneUntouched) {
    poly::LaneConfig lane;
    lane.cycle.steps = 7;
    lane.hitCount = 5;
    lane.rotation = 2;
    const poly::LaneConfig before = lane;

    std::vector<uint8_t> notMidi = {'h', 'e', 'l', 'l', 'o'};
    EXPECT_FALSE(poly::importMidiToLane(notMidi, lane));
    EXPECT_EQ(lane.cycle.steps, before.cycle.steps);
    EXPECT_EQ(lane.hitCount, before.hitCount);
    EXPECT_EQ(lane.rotation, before.rotation);
}

TEST(ImportMidi, HelperRejectsDegenerateSingleOnset) {
    // One note -> fewer than two onsets -> fitEuclidean returns valid=false, so
    // the atomic import must fail and leave the lane untouched.
    std::vector<uint8_t> body = {0x00, 0x90, 0x3C, 0x40, 0x00, 0xFF, 0x2F, 0x00};
    const poly::MidiParseResult parsed = poly::parseSMF(makeSMF(480, body));
    ASSERT_TRUE(parsed.valid);
    ASSERT_EQ(parsed.onsetsPpq.size(), 1u);

    poly::LaneConfig lane;
    const poly::LaneConfig before = lane;
    EXPECT_FALSE(poly::importMidiToLane(makeSMF(480, body), lane));
    EXPECT_EQ(lane.cycle.steps, before.cycle.steps);
    EXPECT_EQ(lane.hitCount, before.hitCount);
}

// --- wasm C-ABI atomic path (the export the web preview's fitMidi routes to) --

TEST(ImportMidi, WasmExportMutatesTargetLane) {
    const auto events = euclideanEvents(5, 8, 0, 8); // cinquillo
    const auto bytes = poly::writeSMF(events.data(), events.size(), 120.0);

    PolyContext ctx = poly_create();
    ASSERT_NE(ctx, nullptr);
    const int rc = poly_import_midi(ctx, 0, bytes.data(), static_cast<int>(bytes.size()));
    EXPECT_EQ(rc, 1);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldCycleSteps), 8);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldSubdivision), 8);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldHitCount), 5);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldRotation), 0);
    poly_destroy(ctx);
}

TEST(ImportMidi, WasmExportRejectsNonMidiWithoutMutation) {
    PolyContext ctx = poly_create();
    ASSERT_NE(ctx, nullptr);
    const int stepsBefore = poly_lane_int(ctx, 0, kFieldCycleSteps);
    const int hitsBefore = poly_lane_int(ctx, 0, kFieldHitCount);

    std::vector<uint8_t> notMidi = {'n', 'o', 't', ' ', 'm', 'i', 'd', 'i'};
    const int rc = poly_import_midi(ctx, 0, notMidi.data(), static_cast<int>(notMidi.size()));
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldCycleSteps), stepsBefore);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldHitCount), hitsBefore);
    poly_destroy(ctx);
}

TEST(ImportMidi, WasmExportGuardsBadArguments) {
    const auto events = euclideanEvents(3, 8, 0, 8);
    const auto bytes = poly::writeSMF(events.data(), events.size(), 120.0);

    PolyContext ctx = poly_create();
    ASSERT_NE(ctx, nullptr);
    // Out-of-range lane, null data, and non-positive size all reject with 0 and
    // never dereference out of bounds.
    EXPECT_EQ(poly_import_midi(ctx, -1, bytes.data(), static_cast<int>(bytes.size())), 0);
    EXPECT_EQ(poly_import_midi(ctx, 999, bytes.data(), static_cast<int>(bytes.size())), 0);
    EXPECT_EQ(poly_import_midi(ctx, 0, nullptr, 8), 0);
    EXPECT_EQ(poly_import_midi(ctx, 0, bytes.data(), 0), 0);
    poly_destroy(ctx);
}

// --- Atomic import snapshot / revert (M035 S03 T01) --------------------------
//
// Suite name RevertImport so `ctest -R RevertImport` selects exactly these.
// Covers the pure engine helpers (captureLaneSnapshot / revertLaneFromSnapshot)
// and the wasm C-ABI revert path (poly_revert_import) both UI surfaces call,
// plus the negative contract: an empty snapshot and a rejected drop leave
// nothing to revert into and are safe no-ops.

TEST(RevertImport, HelperRestoresPreImportLaneByteForByte) {
    // A hand-set lane, snapshotted, then overwritten by an import; revert must
    // restore every field the import touched (steps, subdivision, hits,
    // rotation, timeline/fixedPattern, micro-timing).
    poly::LaneConfig lane;
    lane.cycle.steps = 7;
    lane.cycle.subdivision = 12;
    lane.hitCount = 5;
    lane.rotation = 2;
    lane.microTimingMs[0] = 3.5f;
    const poly::LaneConfig before = lane;

    poly::LaneSnapshot snap = poly::captureLaneSnapshot(lane);
    EXPECT_TRUE(snap.valid);

    const auto events = euclideanEvents(5, 16, 3, 16);
    const auto bytes = poly::writeSMF(events.data(), events.size(), 120.0);
    ASSERT_TRUE(poly::importMidiToLane(bytes, lane));
    EXPECT_EQ(lane.cycle.steps, 16); // import landed

    ASSERT_TRUE(poly::revertLaneFromSnapshot(lane, snap));
    EXPECT_EQ(lane.cycle.steps, before.cycle.steps);
    EXPECT_EQ(lane.cycle.subdivision, before.cycle.subdivision);
    EXPECT_EQ(lane.hitCount, before.hitCount);
    EXPECT_EQ(lane.rotation, before.rotation);
    EXPECT_EQ(lane.timeline, before.timeline);
    EXPECT_FLOAT_EQ(lane.microTimingMs[0], before.microTimingMs[0]);
    // Snapshot is consumed: a second revert is a no-op.
    EXPECT_FALSE(snap.valid);
    EXPECT_FALSE(poly::revertLaneFromSnapshot(lane, snap));
}

TEST(RevertImport, HelperEmptySnapshotIsSafeNoOp) {
    poly::LaneConfig lane;
    lane.cycle.steps = 9;
    const poly::LaneConfig before = lane;
    poly::LaneSnapshot empty; // valid == false
    EXPECT_FALSE(poly::revertLaneFromSnapshot(lane, empty));
    EXPECT_EQ(lane.cycle.steps, before.cycle.steps);
}

TEST(RevertImport, WasmExportRoundTripRestoresLane) {
    const auto events = euclideanEvents(5, 8, 0, 8); // cinquillo
    const auto bytes = poly::writeSMF(events.data(), events.size(), 120.0);

    PolyContext ctx = poly_create();
    ASSERT_NE(ctx, nullptr);
    const int stepsBefore = poly_lane_int(ctx, 0, kFieldCycleSteps);
    const int hitsBefore = poly_lane_int(ctx, 0, kFieldHitCount);
    const int rotBefore = poly_lane_int(ctx, 0, kFieldRotation);

    ASSERT_EQ(poly_import_midi(ctx, 0, bytes.data(), static_cast<int>(bytes.size())), 1);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldCycleSteps), 8); // import landed

    EXPECT_EQ(poly_revert_import(ctx, 0), 1);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldCycleSteps), stepsBefore);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldHitCount), hitsBefore);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldRotation), rotBefore);

    // Snapshot consumed: a second revert is a no-op and leaves the lane put.
    EXPECT_EQ(poly_revert_import(ctx, 0), 0);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldCycleSteps), stepsBefore);
    poly_destroy(ctx);
}

TEST(RevertImport, WasmExportNoSnapshotIsNoOp) {
    PolyContext ctx = poly_create();
    ASSERT_NE(ctx, nullptr);
    const int stepsBefore = poly_lane_int(ctx, 0, kFieldCycleSteps);
    // Never imported -> nothing to revert.
    EXPECT_EQ(poly_revert_import(ctx, 0), 0);
    EXPECT_EQ(poly_lane_int(ctx, 0, kFieldCycleSteps), stepsBefore);
    poly_destroy(ctx);
}

TEST(RevertImport, WasmExportRejectedDropLeavesNoSnapshot) {
    // A rejected drop (non-MIDI) must not arm a revert — S03 must-have 6.
    PolyContext ctx = poly_create();
    ASSERT_NE(ctx, nullptr);
    std::vector<uint8_t> notMidi = {'n', 'o', 'p', 'e'};
    EXPECT_EQ(poly_import_midi(ctx, 0, notMidi.data(), static_cast<int>(notMidi.size())), 0);
    EXPECT_EQ(poly_revert_import(ctx, 0), 0); // no stale snapshot
    poly_destroy(ctx);
}

TEST(RevertImport, WasmExportGuardsBadLane) {
    PolyContext ctx = poly_create();
    ASSERT_NE(ctx, nullptr);
    EXPECT_EQ(poly_revert_import(ctx, -1), 0);
    EXPECT_EQ(poly_revert_import(ctx, 999), 0);
    poly_destroy(ctx);
}
