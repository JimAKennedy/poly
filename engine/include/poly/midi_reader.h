// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "poly/fitter.h" // FitResult (shared apply helper, M035 S02 T02)
#include "poly/types.h"  // LaneConfig (shared apply helper, M035 S02 T02)

namespace poly {

// --- Standard MIDI File reader (reverse-Euclid import path, M035 S02 T01) ---
//
// The authoritative, general-purpose SMF *reader* for the offline import path:
// it turns the raw bytes of a dropped .mid file into the note-on onset PPQ
// vector poly::fitEuclidean() consumes (fitter.h), closing the read side of the
// MIDI file boundary engine/src/smf_writer.cpp opened on the write side.
//
// It parses SMF format 0 and 1, decodes VLQ delta times, honours running
// status, reads the first FF 51 tempo meta, and merges every track's note-on
// stream into one onset timeline (the import path fits a single lane at a time,
// and a dropped loop is that lane). Metrical division only (ticks-per-quarter);
// SMPTE division files are reported invalid rather than silently misread.
//
// Like the fitter this module is deliberately NON-RT — it runs on the file-drop
// import path, never inside renderRange()/process() — so std::vector is fine. It
// has zero VST3/audio-thread dependencies (engine isolation, CLAUDE.md).
//
// No-throw contract: parseSMF never throws and never reads out of bounds. Any
// structural problem (too small, bad 'MThd' magic, SMPTE division, truncated
// chunk) yields a result with valid == false; malformed input is a data
// condition the caller handles, not an exception (Decision D037).

// region:midi-parse-result
struct MidiParseResult {
    // False for input that is not a usable metrical SMF (bad header, SMPTE
    // division, no readable structure). The caller should ignore onsetsPpq /
    // loopLengthPpq and leave the target lane untouched when this is false.
    bool valid = false;

    // Note-on onset positions in PPQ (quarter-note units), measured from the
    // start of the file (tick 0), merged across all tracks and sorted ascending.
    // A note-on with velocity 0 is treated as a note-off and contributes no
    // onset, matching the SMF running-status convention. May be empty for a
    // well-formed file that carries no notes.
    std::vector<double> onsetsPpq;

    // Total length of the parsed content in PPQ: the largest absolute tick
    // reached by any track (typically the final note-off / end-of-track),
    // converted to quarter-note units. This is the loopLengthPpq the fitter
    // needs to resolve trailing-rest cycle-length ambiguity.
    double loopLengthPpq = 0.0;

    // Tempo in BPM from the first FF 51 tempo meta encountered, or 120 when the
    // file carries no tempo meta. Passed to fitEuclidean only to express
    // micro-timing residuals in ms.
    double tempoBpm = 120.0;
};
// endregion:midi-parse-result

// region:parseSMF
// Parse a Standard MIDI File held in [data, data+size). Never throws; on any
// structural failure returns a result with valid == false (see MidiParseResult).
[[nodiscard]] MidiParseResult parseSMF(const uint8_t* data, size_t size);

// Convenience overload for the common owned-buffer case (dropped file bytes).
[[nodiscard]] MidiParseResult parseSMF(const std::vector<uint8_t>& data);
// endregion:parseSMF

// region:apply-fit-to-lane
// Apply a fitted Euclidean result to a lane in place (M035 S02 T02). Writes the
// recovered cycle length, subdivision, hit count and rotation. When the fit
// found per-step occupancy the maximally-even skeleton cannot express
// (patternMismatch > 0), the lane is switched to timeline mode carrying the
// actual fixedPattern so a re-render approximates the source rather than the
// nearest textbook Euclid; otherwise timeline mode is cleared. The per-step
// micro-timing residual is always copied (zero on-grid). A degenerate fit
// (valid == false) leaves the lane untouched. Pure and non-RT — offline import
// path only.
void applyFitToLane(const FitResult& fit, LaneConfig& lane);

// Atomic parse -> fit -> apply for the offline MIDI import path (M035 S02 T02).
// Parses the dropped SMF bytes, fits Euclidean lane parameters to the recovered
// onsets, and applies them to `lane`. Returns true iff the bytes were a usable
// metrical SMF AND the fit was valid; on false the lane is left untouched. This
// is the single import operation both UI surfaces call — the poly_import_midi
// wasm export (web preview) and the plugin's fitMidi bridge action — so they
// agree on exactly what a drop does. Never throws (parseSMF's no-throw contract,
// Decision D037).
[[nodiscard]] bool importMidiToLane(const uint8_t* data, size_t size, LaneConfig& lane);

// Convenience overload for the common owned-buffer case (dropped file bytes).
[[nodiscard]] bool importMidiToLane(const std::vector<uint8_t>& data, LaneConfig& lane);
// endregion:apply-fit-to-lane

// region:import-snapshot
// Atomic pre-import snapshot of a single lane, backing the revertImport bridge
// action (M035 S03 T01, Decision D039). Holds a full copy of the LaneConfig as
// it was immediately before an import overwrote it, plus a validity flag. The
// snapshot lives entirely engine-side and is keyed by lane index at the call
// site (the wasm Context, the plugin scene), so the bridge contract stays
// {lane}-only — no LaneConfig is ever serialized across the plugin/JS boundary.
struct LaneSnapshot {
    LaneConfig config{};
    bool valid = false;
};

// Copy `lane` into a snapshot the caller stores keyed by lane index. Call this
// immediately before importMidiToLane/applyFitToLane overwrites the lane, then
// retain the snapshot only when the import actually succeeds so a rejected drop
// (non-MIDI, degenerate fit) leaves no stale snapshot to revert into (S03
// must-have 6). Pure and non-RT — offline import path only.
[[nodiscard]] LaneSnapshot captureLaneSnapshot(const LaneConfig& lane);

// Restore `lane` from `snap` if the snapshot is valid, then invalidate `snap`
// so a subsequent revert is a safe no-op. Returns true iff a restore actually
// happened. An empty (invalid) snapshot changes nothing and returns false —
// mirroring the S02 unknown-message-drop contract, the caller logs a warning
// and no-ops rather than crashing. Pure and non-RT.
bool revertLaneFromSnapshot(LaneConfig& lane, LaneSnapshot& snap);
// endregion:import-snapshot

} // namespace poly
