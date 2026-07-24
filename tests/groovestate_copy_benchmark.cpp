// M049 S09 (E10): benchmark the per-block GrooveState copy pipeline the
// plugin runs inside process(). Answers the question the 2026-07-16 review
// left open: is the "3-4 copies of a 13.6 KB struct per audio block" cost
// worth engineering around, or is it in the noise?
//
// Pipeline replicated from plugin/source/processor.cpp process():
//     GrooveState base = sceneState_.sceneA;                    // copy 1
//     GrooveState macroOut = resolveMacros(base);                // copy 2 (return)
//     GrooveState resolved = resolveConstraints(base, macroOut); // copy 3 (return)
//     engine.renderRange(tc, resolved, out);                     // const ref, no copy
//
// Morph adds a 4th copy via interpolateGrooveState.
//
// Not an assertion-heavy test — the SUCCESS criterion is that the numbers
// print. The decision is recorded in .gsd/DECISIONS.md and checked in.
// A trivial ceiling assertion at the end guards against a 10× regression.

#include <chrono>
#include <cstdio>

#include <gtest/gtest.h>

#include "poly/constraint.h"
#include "poly/macro.h"
#include "poly/presets.h"
#include "poly/scene.h"
#include "poly/types.h"

namespace {

using Clock = std::chrono::steady_clock;
using ns = std::chrono::nanoseconds;

constexpr int kIters = 100000;

// Realistic audio-period constants (44.1 kHz).
constexpr double kSampleRateHz = 44100.0;
constexpr double kUsPerBlock128 = 128.0 * 1e6 / kSampleRateHz;   // ~2903 us
constexpr double kUsPerBlock512 = 512.0 * 1e6 / kSampleRateHz;   // ~11610 us
constexpr double kUsPerBlock1024 = 1024.0 * 1e6 / kSampleRateHz; // ~23220 us

poly::GrooveState makeRealisticState() {
    // Use a shipping factory preset so lane fields, envelope counts, and
    // constraints are representative — not a zeroed skeleton.
    return poly::makeAfrobeat12_8();
}

// Volatile sink to keep the optimizer honest — without it clang happily
// elides the whole pipeline as dead code and the benchmark reports 0 ns.
volatile uint64_t g_sink = 0;

void consume(const poly::GrooveState& gs) {
    g_sink ^= static_cast<uint64_t>(gs.lanes[0].baseVelocity);
    g_sink ^= static_cast<uint64_t>(gs.macros.complexity * 1000.0f);
    g_sink ^= static_cast<uint64_t>(gs.activeLaneCount);
}

} // namespace

TEST(GrooveStateCopyBenchmark, ReportsFactSizes) {
    std::fprintf(stderr, "[bench] sizeof(GrooveState) = %zu bytes\n", sizeof(poly::GrooveState));
    std::fprintf(stderr, "[bench] sizeof(LaneConfig)  = %zu bytes\n", sizeof(poly::LaneConfig));
    std::fprintf(stderr, "[bench] sizeof(SceneState)  = %zu bytes\n", sizeof(poly::SceneState));
    EXPECT_EQ(sizeof(poly::GrooveState), 13584u) << "GrooveState size changed — update DECISIONS.md perf entry";
}

TEST(GrooveStateCopyBenchmark, ThreeCopyPipelineFitsBlockBudget) {
    auto scene = poly::SceneState{};
    scene.sceneA = makeRealisticState();
    scene.sceneB = makeRealisticState();

    auto t0 = Clock::now();
    for (int i = 0; i < kIters; ++i) {
        poly::GrooveState base = scene.sceneA;                                 // copy 1
        poly::GrooveState macroOut = poly::resolveMacros(base);                // copy 2
        poly::GrooveState resolved = poly::resolveConstraints(base, macroOut); // copy 3
        consume(resolved);
    }
    auto t1 = Clock::now();

    double totalNs = std::chrono::duration_cast<ns>(t1 - t0).count();
    double perBlockUs = (totalNs / kIters) / 1000.0;

    std::fprintf(stderr, "[bench] 3-copy pipeline: %.2f us/block (%d iters, %.1f ms total)\n", perBlockUs, kIters,
                 totalNs / 1e6);
    std::fprintf(stderr, "[bench]   %.2f%% of 128-sample block period (%.2f us)\n", 100.0 * perBlockUs / kUsPerBlock128,
                 kUsPerBlock128);
    std::fprintf(stderr, "[bench]   %.2f%% of 512-sample block period (%.2f us)\n", 100.0 * perBlockUs / kUsPerBlock512,
                 kUsPerBlock512);
    std::fprintf(stderr, "[bench]   %.2f%% of 1024-sample block period (%.2f us)\n",
                 100.0 * perBlockUs / kUsPerBlock1024, kUsPerBlock1024);

    // Ceiling assertion: guards against a >200× regression. Recorded baseline
    // (macOS M-series, Release build, M049 S09 landing): ~0.5 us/block. This
    // ceiling is deliberately loose so slower CI runners (Ubuntu, Windows,
    // Debug builds) don't trip it — the number to watch is the perBlockUs
    // print, not this bound.
    EXPECT_LT(perBlockUs, 100.0) << "3-copy pipeline exceeded 100us — reopen the copy-elimination question";
}

TEST(GrooveStateCopyBenchmark, MorphPathAddsOneMoreCopy) {
    auto scene = poly::SceneState{};
    scene.sceneA = makeRealisticState();
    scene.sceneB = makeRealisticState();
    scene.morphAmount = 0.5f;

    auto t0 = Clock::now();
    for (int i = 0; i < kIters; ++i) {
        poly::GrooveState base = poly::interpolateGrooveState(scene.sceneA, scene.sceneB, scene.morphAmount);
        poly::GrooveState macroOut = poly::resolveMacros(base);
        poly::GrooveState resolved = poly::resolveConstraints(base, macroOut);
        consume(resolved);
    }
    auto t1 = Clock::now();

    double totalNs = std::chrono::duration_cast<ns>(t1 - t0).count();
    double perBlockUs = (totalNs / kIters) / 1000.0;

    std::fprintf(stderr, "[bench] Morph 4-copy pipeline: %.2f us/block (%d iters)\n", perBlockUs, kIters);
    std::fprintf(stderr, "[bench]   %.2f%% of 128-sample block period\n", 100.0 * perBlockUs / kUsPerBlock128);

    // Morph path is inherently more work (interpolate does per-field lerp);
    // ceiling looser to reflect that.
    EXPECT_LT(perBlockUs, 200.0) << "Morph 4-copy pipeline exceeded 200us — reopen the copy-elimination question";
}

TEST(GrooveStateCopyBenchmark, RawStructCopyBaseline) {
    // Isolate the raw memcpy-style struct assignment from the resolve* work,
    // so the decision doc can attribute cost between copy vs compute.
    auto src = makeRealisticState();

    auto t0 = Clock::now();
    for (int i = 0; i < kIters; ++i) {
        poly::GrooveState dst = src;
        consume(dst);
    }
    auto t1 = Clock::now();

    double totalNs = std::chrono::duration_cast<ns>(t1 - t0).count();
    double perCopyNs = totalNs / kIters;

    std::fprintf(stderr, "[bench] raw single struct copy: %.0f ns/copy (%d iters)\n", perCopyNs, kIters);
    std::fprintf(stderr, "[bench]   theoretical throughput at that cost: %.1f GB/s\n",
                 (static_cast<double>(sizeof(poly::GrooveState)) / perCopyNs));
}
