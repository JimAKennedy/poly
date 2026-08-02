#pragma once

#include <array>

#include "poly/types.h"

namespace poly {

// Distribute k pulses across n steps as evenly as possible via Bjorklund's
// algorithm (matching site/src/components/EuclideanDiagram.astro).
// Rotation shifts the pattern right by `rotation` positions (wrapping).
// Fills the first n elements of `out`; remaining elements are false.
void euclidean(int k, int n, int rotation, std::array<bool, kMaxSteps>& out);

} // namespace poly
