// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// Offline SMF emitter — writes a deterministic Format-1 .mid to disk from the
// same renderPatternToSMF primitive the WebUI Export chip uses. It exists so the
// independent-parser export validator (tests/cubase/validate_smf_export.py) has
// a real engine-produced .mid to check in NORMAL CI, without Cubase: it emits
// the default factory preset (index 0) over 4 bars at 120 BPM — byte-identical
// to the export the "open in Cubase and confirm named tracks" manual UAT covers
// (M032 S01/S02), so the validator asserts the same contract offline.
//
// Usage:
//   poly_smf_emit <out.mid> [laneFilter]
//     out.mid      destination path (required)
//     laneFilter   optional lane index; -1 / omitted = all active lanes,
//                  N in [0,kMaxLanes) = single-lane export (M032 S02).
//
// Deterministic: the same (preset, bars, tempo, timesig, laneFilter) always
// yields identical bytes, so the validator's expectations are stable.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "poly/offline_render.h"
#include "poly/presets.h"
#include "poly/types.h"

namespace {

// The default-patch 4-bar render behind the golden (tests/golden/
// processor_default_4bars.txt) and the S01/S02 named-track UAT.
constexpr int kBars = 4;
constexpr double kTempo = 120.0;
constexpr int kTimeSigNum = 4;
constexpr int kTimeSigDen = 4;

} // namespace

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::fprintf(stderr, "usage: %s <out.mid> [laneFilter]\n", argv[0]);
        return 2;
    }
    const char* outPath = argv[1];
    int laneFilter = -1;
    if (argc == 3) {
        char* end = nullptr;
        const long v = std::strtol(argv[2], &end, 10);
        if (end == argv[2] || *end != '\0') {
            std::fprintf(stderr, "error: laneFilter must be an integer, got '%s'\n", argv[2]);
            return 2;
        }
        laneFilter = static_cast<int>(v);
    }

    poly::SceneState scene{};
    scene.sceneA = poly::makeFactoryPreset(0);
    const std::vector<uint8_t> smf =
        poly::renderPatternToSMF(scene, kBars, kTempo, kTimeSigNum, kTimeSigDen, laneFilter);

    std::FILE* f = std::fopen(outPath, "wb");
    if (f == nullptr) {
        std::fprintf(stderr, "error: could not open %s for writing\n", outPath);
        return 1;
    }
    const size_t written = smf.empty() ? 0 : std::fwrite(smf.data(), 1, smf.size(), f);
    std::fclose(f);
    if (written != smf.size()) {
        std::fprintf(stderr, "error: short write to %s (%zu/%zu bytes)\n", outPath, written, smf.size());
        return 1;
    }
    std::fprintf(stderr, "wrote %zu bytes to %s (preset 0, %d bars, laneFilter=%d)\n", smf.size(), outPath, kBars,
                 laneFilter);
    return 0;
}
