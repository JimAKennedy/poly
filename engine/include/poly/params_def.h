#pragma once

// Single-source-of-truth registry for Poly's lane-scoped parameters.
//
// The 4-way scaling duplication the M048 review flagged (processor.cpp,
// web_ui_view.cpp, header_view.cpp, controller_base.cpp) collapses through
// the two dispatch functions in this header. Each parameter appears exactly
// once — as a row in kLaneCoreParamRegistry or kLaneExprParamRegistry — with
// its scaling kind, engine range, and default. Consumers call
// normToEngineCore / normToEngineExpr (VST3 normalized double -> engine value)
// or their engineToNorm inverses.
//
// This header lives in engine/ and MUST NOT include VST3 SDK types. Parameter
// IDs here are the raw `offset` fields; the plugin layer wraps them into
// Steinberg::Vst::ParamID via ParamIDs::laneParam / laneCoreParam. Keeping
// this engine-side lets the emitter tool (S07 bridge schema) and engine
// tests (T02 roundtrip) consume the registry without a plugin build.

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace poly {
namespace params {

// Kinds that describe how a normalized [0,1] value maps to an engine value.
// Registry entries pick one; the dispatch functions branch on it.
enum class Kind : uint8_t {
    Unit01,      // engine = norm (identity, min=0, max=1)
    LinearFloat, // engine = min + norm * (max - min)
    Byte7,       // engine = round(norm * 127); 8-bit clamped
    Ranged1_64,  // engine = 1 + round(norm * 63) — steps
    Ranged0_63,  // engine = round(norm * 63) — rotation
    Ranged0_64,  // engine = round(norm * 64) — cellCount, fixedPatternLen, hitCount, phraseLength (float scaled)
    Bool,        // engine = (norm > 0.5) ? 1 : 0
    Subdivision, // engine picks from {1,2,4,8,16} by round(norm*4)
    TempoMult,   // engine = 0.25 + norm * 3.75
    DriftRate,   // engine = norm * 8 - 4
    TimingOffs,  // engine = norm * 40 - 20
    MidiChannel, // engine = round(norm * 16) - 1  (yields -1..15)
    KotekanSrc,  // engine = round(norm * 8) - 1   (yields -1..7)
};

struct Entry {
    uint32_t offset; // param offset within its family (matches ParamIDs::kProbability etc)
    const char* name;
    Kind kind;
    double minEngine; // informational; the dispatch functions encode this per-kind
    double maxEngine;
    double defaultEngine;
};

// Lane expression params (16), addressed via ParamIDs::laneParam(lane, offset).
// Order matches plugids.h ParamIDs::kProbability..kKotekanSource so the offset
// field IS the ParamIDs constant.
static constexpr std::array<Entry, 16> kLaneExprParamRegistry = {{
    {0, "Probability", Kind::Unit01, 0.0, 1.0, 1.0},
    {1, "Base Velocity", Kind::Byte7, 0.0, 127.0, 100.0},
    {2, "Emphasis", Kind::Unit01, 0.0, 1.0, 0.5},
    {3, "Ghost Floor", Kind::Byte7, 0.0, 127.0, 30.0},
    {4, "Spread", Kind::Unit01, 0.0, 1.0, 0.05},
    {5, "Swing", Kind::Unit01, 0.0, 1.0, 0.0},
    {6, "Humanize", Kind::LinearFloat, 0.0, 50.0, 0.0},
    {7, "Duration", Kind::LinearFloat, 0.0, 4.0, 0.0},
    {8, "Active", Kind::Bool, 0.0, 1.0, 1.0},
    {9, "Phrase Len", Kind::LinearFloat, 0.0, 64.0, 0.0},
    {10, "Phrase Gap", Kind::LinearFloat, 0.0, 64.0, 0.0},
    {11, "Phrase Offset", Kind::LinearFloat, 0.0, 64.0, 0.0},
    {12, "Mutation", Kind::Unit01, 0.0, 1.0, 0.0},
    {13, "Drift Rate", Kind::DriftRate, -4.0, 4.0, 0.0},
    {14, "Timing Offset", Kind::TimingOffs, -20.0, 20.0, 0.0},
    {15, "Kotekan Source", Kind::KotekanSrc, -1.0, 7.0, -1.0},
}};

// Lane core params (10), addressed via ParamIDs::laneCoreParam(lane, offset).
// Order matches plugids.h ParamIDs::kCoreSteps..kCoreMidiChannel.
static constexpr std::array<Entry, 10> kLaneCoreParamRegistry = {{
    {0, "Steps", Kind::Ranged1_64, 1.0, 64.0, 4.0},
    {1, "Subdivision", Kind::Subdivision, 1.0, 16.0, 4.0},
    {2, "Hits", Kind::Ranged0_64, 0.0, 64.0, 4.0},
    {3, "Rotation", Kind::Ranged0_63, 0.0, 63.0, 0.0},
    {4, "MIDI Note", Kind::Byte7, 0.0, 127.0, 36.0},
    {5, "Cell Count", Kind::Ranged0_64, 0.0, 64.0, 0.0},
    {6, "Timeline", Kind::Bool, 0.0, 1.0, 0.0},
    {7, "Pattern Length", Kind::Ranged0_64, 0.0, 64.0, 0.0},
    {8, "Tempo Mult", Kind::TempoMult, 0.25, 4.0, 1.0},
    {9, "MIDI Channel", Kind::MidiChannel, -1.0, 15.0, -1.0},
}};

namespace detail {

// Subdivision choices exposed as a stepped enum through Kind::Subdivision.
// index = round(norm * 4), engine value = kSubdivChoices[index].
static constexpr std::array<int, 5> kSubdivChoices = {1, 2, 4, 8, 16};

inline double clamp01(double v) {
    return v < 0.0 ? 0.0 : (v > 1.0 ? 1.0 : v);
}

inline double dispatchNormToEngine(Kind k, double norm) {
    const double n = clamp01(norm);
    switch (k) {
    case Kind::Unit01:
        return n;
    case Kind::LinearFloat:
        return n; // caller multiplies by (max-min); handled below
    case Kind::Byte7:
        return std::round(n * 127.0);
    case Kind::Ranged1_64:
        return 1.0 + std::round(n * 63.0);
    case Kind::Ranged0_63:
        return std::round(n * 63.0);
    case Kind::Ranged0_64:
        return std::round(n * 64.0);
    case Kind::Bool:
        return (n > 0.5) ? 1.0 : 0.0;
    case Kind::Subdivision: {
        int idx = static_cast<int>(std::round(n * 4.0));
        if (idx < 0)
            idx = 0;
        if (idx > 4)
            idx = 4;
        return static_cast<double>(kSubdivChoices[static_cast<size_t>(idx)]);
    }
    case Kind::TempoMult:
        return 0.25 + n * 3.75;
    case Kind::DriftRate:
        return n * 8.0 - 4.0;
    case Kind::TimingOffs:
        return n * 40.0 - 20.0;
    case Kind::MidiChannel:
        return std::round(n * 16.0) - 1.0;
    case Kind::KotekanSrc:
        return std::round(n * 8.0) - 1.0;
    }
    return n;
}

inline double dispatchEngineToNorm(Kind k, double engine) {
    switch (k) {
    case Kind::Unit01:
        return clamp01(engine);
    case Kind::LinearFloat:
        return engine; // caller divides by (max-min); handled below
    case Kind::Byte7:
        return clamp01(engine / 127.0);
    case Kind::Ranged1_64:
        return clamp01((engine - 1.0) / 63.0);
    case Kind::Ranged0_63:
        return clamp01(engine / 63.0);
    case Kind::Ranged0_64:
        return clamp01(engine / 64.0);
    case Kind::Bool:
        return (engine > 0.5) ? 1.0 : 0.0;
    case Kind::Subdivision: {
        // Reverse the choices table. Unknown subdivisions round to nearest.
        int best = 0;
        double bestDist = 1e30;
        for (int i = 0; i < 5; ++i) {
            double d = std::abs(engine - static_cast<double>(kSubdivChoices[static_cast<size_t>(i)]));
            if (d < bestDist) {
                bestDist = d;
                best = i;
            }
        }
        return static_cast<double>(best) / 4.0;
    }
    case Kind::TempoMult:
        return clamp01((engine - 0.25) / 3.75);
    case Kind::DriftRate:
        return clamp01((engine + 4.0) / 8.0);
    case Kind::TimingOffs:
        return clamp01((engine + 20.0) / 40.0);
    case Kind::MidiChannel:
        return clamp01((engine + 1.0) / 16.0);
    case Kind::KotekanSrc:
        return clamp01((engine + 1.0) / 8.0);
    }
    return clamp01(engine);
}

// LinearFloat helpers know the entry's min/max; dispatch dispatches into these
// via family-specific normToEngineExpr / normToEngineCore for readability.
inline double normToLinearFloat(const Entry& e, double norm) {
    return e.minEngine + clamp01(norm) * (e.maxEngine - e.minEngine);
}
inline double linearFloatToNorm(const Entry& e, double engine) {
    if (e.maxEngine == e.minEngine)
        return 0.0;
    return clamp01((engine - e.minEngine) / (e.maxEngine - e.minEngine));
}

} // namespace detail

// Family-specific dispatchers. Callers pass the offset (the ParamIDs constant)
// and get back the engine value. Unknown offsets return 0.0 (silent no-op —
// same defensive behavior as the legacy switch's `default: break`).
inline double normToEngineExpr(uint32_t offset, double norm) {
    for (const auto& e : kLaneExprParamRegistry) {
        if (e.offset == offset) {
            if (e.kind == Kind::LinearFloat)
                return detail::normToLinearFloat(e, norm);
            return detail::dispatchNormToEngine(e.kind, norm);
        }
    }
    return 0.0;
}

inline double engineToNormExpr(uint32_t offset, double engine) {
    for (const auto& e : kLaneExprParamRegistry) {
        if (e.offset == offset) {
            if (e.kind == Kind::LinearFloat)
                return detail::linearFloatToNorm(e, engine);
            return detail::dispatchEngineToNorm(e.kind, engine);
        }
    }
    return 0.0;
}

inline double normToEngineCore(uint32_t offset, double norm) {
    for (const auto& e : kLaneCoreParamRegistry) {
        if (e.offset == offset) {
            if (e.kind == Kind::LinearFloat)
                return detail::normToLinearFloat(e, norm);
            return detail::dispatchNormToEngine(e.kind, norm);
        }
    }
    return 0.0;
}

inline double engineToNormCore(uint32_t offset, double engine) {
    for (const auto& e : kLaneCoreParamRegistry) {
        if (e.offset == offset) {
            if (e.kind == Kind::LinearFloat)
                return detail::linearFloatToNorm(e, engine);
            return detail::dispatchEngineToNorm(e.kind, engine);
        }
    }
    return 0.0;
}

} // namespace params
} // namespace poly
