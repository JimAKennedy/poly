#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "poly/types.h"

namespace poly {

static constexpr int kSmfTicksPerQuarter = 480;

// region:writeSMF
std::vector<uint8_t> writeSMF(const NoteEvent* events, size_t count, double tempo, double ppqOffset = 0.0);
// endregion:writeSMF

size_t writeVLQ(uint32_t value, uint8_t* out);

} // namespace poly
