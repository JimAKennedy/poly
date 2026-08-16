// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "poly/types.h"

namespace poly {

static constexpr int kSmfTicksPerQuarter = 480;

// M049 S03 (E3): tempo clamp floor. Applied to writeSMF()'s tempo argument when
// the caller passes 0, negative, or non-finite (transport pauses, uninitialized
// export state). Matches the guard shape in bridge.cpp's ppqToSampleOffset but
// clamps rather than early-returns so the export path always yields a valid
// SMF blob. 20 BPM is the slowest tempo any major DAW/notation tool treats as
// valid; below that the tempo meta encoding still fits in uint32_t but the
// output is musically nonsensical.
static constexpr double kSmfMinTempo = 20.0;

// region:writeSMF
std::vector<uint8_t> writeSMF(const NoteEvent* events, size_t count, double tempo, double ppqOffset = 0.0);
// endregion:writeSMF

// region:writeMultiTrackSMF
// Format-1 multi-track serializer. Emits MThd with format=1 and
// ntrks = 1 + (number of distinct NoteEvent::laneIndex values among events).
// Track 0 is a conductor track carrying only the FF 51 tempo meta + FF 2F
// end-of-track. Each subsequent MTrk holds exactly one lane's note events,
// ordered by ascending laneIndex, and is preceded by an FF 03 track-name meta.
//
// nameForLane maps a laneIndex to a human-readable track name (e.g. the offline
// export path passes laneName(midiNote) → "Kick"/"Snare"/...). When it is empty
// (the legacy capture path, which has no LaneConfig) each track falls back to
// "Lane N" where N = laneIndex + 1.
//
// tempo clamp (kSmfMinTempo), ppqOffset trimming, VLQ delta encoding, and the
// note-off-before-note-on-at-same-tick ordering all match writeSMF; only the
// chunk layout differs. writeSMF is left untouched for the Format-0 parity
// harness — this is a separate function.
std::vector<uint8_t> writeMultiTrackSMF(const NoteEvent* events, size_t count, double tempo, double ppqOffset = 0.0,
                                        const std::function<std::string(int laneIndex)>& nameForLane = {});
// endregion:writeMultiTrackSMF

size_t writeVLQ(uint32_t value, uint8_t* out);

} // namespace poly
