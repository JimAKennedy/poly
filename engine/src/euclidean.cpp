#include "poly/euclidean.h"

#include <algorithm>

namespace poly {

// region:bjorklund
// Bjorklund's pairing/elimination algorithm — the maximally-even distribution
// of k pulses across n steps, matching the normative reference in
// site/src/components/EuclideanDiagram.astro so diagrams show what the engine
// plays (enforced by the exhaustive equality test in tests/euclidean_tests.cpp).
//
// RT-safe reformulation: the reference builds an array-of-arrays and repeatedly
// concatenates pattern[i] ++ remainder[i]. Two invariants let us collapse that
// to fixed-size storage with no heap use:
//   1. Every sequence currently in `pattern` is identical to the others, as is
//      every sequence in `remainder`. Concatenation and slicing preserve this,
//      so the whole state is just two bit-sequences (P, R) plus their counts.
//   2. Each sequence is a subsequence of the original n bits, so len(P),
//      len(R) <= n <= kMaxSteps and every buffer is a std::array<bool,kMaxSteps>.
// The final pattern is pCount copies of P followed by rCount copies of R.
void euclidean(int k, int n, int rotation, std::array<bool, kMaxSteps>& out) {
    out.fill(false);

    if (n <= 0 || k <= 0)
        return;
    if (n > kMaxSteps)
        n = kMaxSteps;
    if (k >= n) {
        for (int i = 0; i < n; ++i)
            out[i] = true;
        return;
    }

    // State: pCount copies of sequence seqP[0..lenP) and rCount copies of
    // seqR[0..lenR). Start with k singleton pulses and (n-k) singleton rests.
    std::array<bool, kMaxSteps> seqP{};
    std::array<bool, kMaxSteps> seqR{};
    seqP[0] = true;
    seqR[0] = false;
    int lenP = 1, lenR = 1;
    int pCount = k, rCount = n - k;

    // Pair each pattern group with a remainder group until at most one remainder
    // group is left, exactly as the reference `while (remainder.length > 1)`.
    while (rCount > 1) {
        const int minLen = std::min(pCount, rCount);

        // newP = P ++ R (all `minLen` resulting pattern groups are identical).
        std::array<bool, kMaxSteps> newP{};
        const int newLenP = lenP + lenR;
        for (int i = 0; i < lenP; ++i)
            newP[i] = seqP[i];
        for (int i = 0; i < lenR; ++i)
            newP[lenP + i] = seqR[i];

        // Leftover groups (the longer collection beyond minLen) become the next
        // remainder; mirrors `pattern.length > remainder.length ? pattern : remainder`.
        std::array<bool, kMaxSteps> newR{};
        int newLenR;
        int newRCount;
        if (pCount > rCount) {
            newR = seqP;
            newLenR = lenP;
            newRCount = pCount - minLen;
        } else {
            newR = seqR;
            newLenR = lenR;
            newRCount = rCount - minLen;
        }

        seqP = newP;
        lenP = newLenP;
        pCount = minLen;
        seqR = newR;
        lenR = newLenR;
        rCount = newRCount;
    }

    // Flatten: pCount copies of P, then rCount copies of R. This is the rotation-0
    // base pattern.
    std::array<bool, kMaxSteps> base{};
    int idx = 0;
    for (int c = 0; c < pCount; ++c)
        for (int i = 0; i < lenP; ++i)
            base[idx++] = seqP[i];
    for (int c = 0; c < rCount; ++c)
        for (int i = 0; i < lenR; ++i)
            base[idx++] = seqR[i];

    // Right-shift rotation, matching the reference's slice-based rotation.
    for (int i = 0; i < n; ++i)
        out[i] = base[((i - rotation) % n + n) % n];
}
// endregion:bjorklund

} // namespace poly
