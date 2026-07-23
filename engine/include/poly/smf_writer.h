#pragma once

#include <cstddef>
#include <cstdint>
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

size_t writeVLQ(uint32_t value, uint8_t* out);

} // namespace poly
