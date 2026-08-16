// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

#include <array>

#include "poly/types.h"

namespace poly {

// Distribute k pulses across n steps as evenly as possible via Bjorklund's
// algorithm (matching site/src/components/EuclideanDiagram.astro).
// Rotation shifts the pattern right by `rotation` positions (wrapping).
// Fills the first n elements of `out`; remaining elements are false.
void euclidean(int k, int n, int rotation, std::array<bool, kMaxSteps>& out);

// M068 S03: rotation delta that migrates a rotation authored against the
// retired Bresenham generator (step i pulses iff `(i*k) mod n < k`) onto the
// Bjorklund generator, preserving the played pattern. Returns the smallest d in
// [0, n) for which `euclidean(k, n, d, ...)` equals the legacy pattern, i.e.
// `bjorklund(k, n) >> d == legacyBresenham(k, n)`. Used only by the saved-state
// migration path (states written before kBjorklundGeneratorStateVersion).
// Returns 0 when k/n are degenerate, rotation-invariant (k >= n), or — as a
// safe fallback — when no exact shift matches.
[[nodiscard]] int euclideanMigrationDelta(int k, int n);

} // namespace poly
