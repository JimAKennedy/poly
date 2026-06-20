#include "poly/euclidean.h"

namespace poly {

void euclidean(int k, int n, int rotation, std::array<bool, kMaxSteps>& out) {
    out.fill(false);

    if (n <= 0 || k <= 0) return;
    if (n > kMaxSteps) n = kMaxSteps;
    if (k >= n) {
        for (int i = 0; i < n; ++i) out[i] = true;
        return;
    }

    // Bresenham-style distribution: position i has a hit when (i*k) mod n < k.
    // Apply rotation by shifting indices.
    for (int i = 0; i < n; ++i) {
        int src = ((i - rotation) % n + n) % n;
        out[i] = (src * k % n) < k;
    }
}

} // namespace poly
