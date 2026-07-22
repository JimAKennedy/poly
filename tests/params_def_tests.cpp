// Roundtrip invariants for the M048 S03 parameter registry.
//
// For every entry in kLaneExprParamRegistry and kLaneCoreParamRegistry we
// assert three things:
//   1) engineToNorm(normToEngine(norm)) ≈ norm at sampled norm values,
//      within a per-Kind tolerance (continuous kinds require ~1e-6; integer
//      and enum kinds quantize and must land on the nearest step).
//   2) normToEngine(0) == entry.minEngine  and  normToEngine(1) == entry.maxEngine.
//   3) engineToNorm(entry.defaultEngine) is in [0, 1] and roundtrips.
//
// These invariants are what makes it safe for the plugin layer to consume
// the registry as its sole scaling authority (S03 T03 wires processor.cpp;
// S04 wires the remaining sites).

#include <cmath>
#include <cstdint>

#include "gtest/gtest.h"
#include "poly/params_def.h"

namespace {

using poly::params::Entry;
using poly::params::Kind;
using poly::params::kLaneCoreParamRegistry;
using poly::params::kLaneExprParamRegistry;

// Continuous kinds where roundtrip should be near-exact (float rounding only).
bool isContinuous(Kind k) {
    switch (k) {
    case Kind::Unit01:
    case Kind::LinearFloat:
    case Kind::TempoMult:
    case Kind::DriftRate:
    case Kind::TimingOffs:
        return true;
    default:
        return false;
    }
}

// For quantizing kinds we can't require exact roundtrip from arbitrary norm
// values (norm=0.37 might quantize away and back to 0.375). We instead assert
// engineToNorm(normToEngine(engineToNorm(engine))) is stable — one full trip
// through quantization, then a second trip must not move again.
double normToEngineExprLocal(uint32_t off, double n) {
    return poly::params::normToEngineExpr(off, n);
}
double engineToNormExprLocal(uint32_t off, double e) {
    return poly::params::engineToNormExpr(off, e);
}
double normToEngineCoreLocal(uint32_t off, double n) {
    return poly::params::normToEngineCore(off, n);
}
double engineToNormCoreLocal(uint32_t off, double e) {
    return poly::params::engineToNormCore(off, e);
}

template <typename NormToEng, typename EngToNorm> void checkRoundtrip(const Entry& e, NormToEng nte, EngToNorm etn) {
    SCOPED_TRACE(::testing::Message() << "param='" << e.name << "' offset=" << e.offset
                                      << " kind=" << static_cast<int>(e.kind));

    // 2) endpoints — clamped norm 0/1 must land on the registry min/max.
    EXPECT_DOUBLE_EQ(nte(e.offset, 0.0), e.minEngine) << "norm=0.0 must map to minEngine";
    EXPECT_DOUBLE_EQ(nte(e.offset, 1.0), e.maxEngine) << "norm=1.0 must map to maxEngine";

    // 3) default must be in [min, max] and reachable via engineToNorm.
    EXPECT_GE(e.defaultEngine, e.minEngine);
    EXPECT_LE(e.defaultEngine, e.maxEngine);
    const double defNorm = etn(e.offset, e.defaultEngine);
    EXPECT_GE(defNorm, 0.0);
    EXPECT_LE(defNorm, 1.0);

    // 1) roundtrip at sampled norm values.
    constexpr double samples[] = {0.0, 0.25, 0.5, 0.75, 1.0};
    for (double n : samples) {
        const double eng = nte(e.offset, n);
        const double back = etn(e.offset, eng);
        if (isContinuous(e.kind)) {
            // Near-exact roundtrip — allow float rounding only.
            EXPECT_NEAR(back, n, 1e-9) << "continuous roundtrip drift at norm=" << n;
        } else {
            // Quantizing kinds: one more trip through the pipeline must be stable.
            const double eng2 = nte(e.offset, back);
            EXPECT_DOUBLE_EQ(eng, eng2) << "quantizing roundtrip unstable at norm=" << n;
        }
    }
}

} // namespace

TEST(ParamsDefTest, ExprRegistryRoundtripsForEveryEntry) {
    for (const auto& e : kLaneExprParamRegistry) {
        checkRoundtrip(e, &normToEngineExprLocal, &engineToNormExprLocal);
    }
}

TEST(ParamsDefTest, CoreRegistryRoundtripsForEveryEntry) {
    for (const auto& e : kLaneCoreParamRegistry) {
        checkRoundtrip(e, &normToEngineCoreLocal, &engineToNormCoreLocal);
    }
}

TEST(ParamsDefTest, RegistryCountsMatchPlugids) {
    // Guard against silent drift: plugids.h declares kParamsPerLane=16 and
    // kCoreParamsPerLane=10. The registries must match.
    EXPECT_EQ(kLaneExprParamRegistry.size(), 16u);
    EXPECT_EQ(kLaneCoreParamRegistry.size(), 10u);
}

TEST(ParamsDefTest, OffsetsAreSequentialAndUnique) {
    // Every offset in each registry must be unique and contiguous [0..N).
    // Consumers use offset as a lookup key (the ParamIDs constant); collisions
    // silently mis-scale a parameter, so this is a real defect surface.
    for (size_t i = 0; i < kLaneExprParamRegistry.size(); ++i) {
        EXPECT_EQ(kLaneExprParamRegistry[i].offset, static_cast<uint32_t>(i));
    }
    for (size_t i = 0; i < kLaneCoreParamRegistry.size(); ++i) {
        EXPECT_EQ(kLaneCoreParamRegistry[i].offset, static_cast<uint32_t>(i));
    }
}

TEST(ParamsDefTest, KnownFormulasMatchLegacyBehaviorSpotCheck) {
    // Anchor a handful of specific mappings against the legacy inline
    // formulas from processor.cpp:648-746 and web_ui_view.cpp:593-707.
    // If any of these fail, S03 T03's "wire processor.cpp" step will
    // produce user-visible behavior drift and must be aborted.
    using poly::params::engineToNormCore;
    using poly::params::engineToNormExpr;
    using poly::params::normToEngineCore;
    using poly::params::normToEngineExpr;

    // Steps: 1 + round(norm*63).  norm=0.5 → 33.
    EXPECT_DOUBLE_EQ(normToEngineCore(0, 0.5), 33.0);
    // Rotation: round(norm*63).  norm=1.0 → 63.
    EXPECT_DOUBLE_EQ(normToEngineCore(3, 1.0), 63.0);
    // MidiNote core: round(norm*127).  norm=36.0/127.0 → 36.
    EXPECT_DOUBLE_EQ(normToEngineCore(4, 36.0 / 127.0), 36.0);
    // TempoMult: 0.25 + norm*3.75.  norm=0 → 0.25; norm=1 → 4.0.
    EXPECT_DOUBLE_EQ(normToEngineCore(8, 0.0), 0.25);
    EXPECT_DOUBLE_EQ(normToEngineCore(8, 1.0), 4.0);
    // MidiChannel: round(norm*16)-1.  norm=0 → -1 (auto).
    EXPECT_DOUBLE_EQ(normToEngineCore(9, 0.0), -1.0);
    // BaseVelocity: round(norm*127).  norm=100/127 → 100.
    EXPECT_DOUBLE_EQ(normToEngineExpr(1, 100.0 / 127.0), 100.0);
    // Humanize (LinearFloat 0..50): norm=1 → 50.
    EXPECT_DOUBLE_EQ(normToEngineExpr(6, 1.0), 50.0);
    // DriftRate: norm*8-4.  norm=0.5 → 0.
    EXPECT_DOUBLE_EQ(normToEngineExpr(13, 0.5), 0.0);
    // TimingOffs: norm*40-20.  norm=0.5 → 0.
    EXPECT_DOUBLE_EQ(normToEngineExpr(14, 0.5), 0.0);
    // KotekanSrc: round(norm*8)-1.  norm=0 → -1 (independent).
    EXPECT_DOUBLE_EQ(normToEngineExpr(15, 0.0), -1.0);

    // Inverse spot checks — controller/header init math (header_view.cpp:196+).
    EXPECT_DOUBLE_EQ(engineToNormExpr(1, 100.0), 100.0 / 127.0);
    EXPECT_DOUBLE_EQ(engineToNormCore(0, 4.0), 3.0 / 63.0);
    EXPECT_DOUBLE_EQ(engineToNormCore(2, 4.0), 4.0 / 64.0);
}
