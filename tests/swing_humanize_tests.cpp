#include <algorithm>
#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/macro.h"
#include "poly/rng.h"
#include "poly/types.h"

namespace {

poly::LaneConfig makeBasicLane() {
    poly::LaneConfig cfg{};
    cfg.id = 0;
    cfg.midiNote = 36;
    cfg.cycle = {.steps = 8, .subdivision = 8};
    cfg.hitCount = 8;
    cfg.baseVelocity = 100;
    cfg.probability = 1.0f;
    cfg.velocitySpread = 0.0f;
    cfg.emphasisProb = 0.0f;
    cfg.ghostFloor = 0;
    cfg.active = true;
    return cfg;
}

std::vector<poly::NoteEvent> renderLane(const poly::LaneConfig& cfg, double ppqEnd = 4.0, uint64_t seed = 42,
                                        double tempo = 120.0) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = seed;
    state.lanes[0] = cfg;

    poly::NoteEventBuffer buf;
    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = ppqEnd;
    tc.tempo = tempo;
    tc.playing = true;

    engine.renderRange(tc, state, buf);

    std::vector<poly::NoteEvent> events;
    for (size_t i = 0; i < buf.count; ++i) {
        events.push_back(buf.events[i]);
    }
    std::sort(events.begin(), events.end(), [](const auto& a, const auto& b) { return a.ppqPosition < b.ppqPosition; });
    return events;
}

} // namespace

// --- Swing Tests ---

TEST(SwingHumanize, NoSwingNoOffset) {
    auto cfg = makeBasicLane();
    cfg.swingAmount = 0.0f;
    auto events = renderLane(cfg);

    ASSERT_EQ(events.size(), 8u);
    const double sPpq = 0.5;
    for (size_t i = 0; i < events.size(); ++i) {
        EXPECT_DOUBLE_EQ(events[i].ppqPosition, i * sPpq) << "Step " << i << " should be unshifted";
    }
}

TEST(SwingHumanize, SwingShiftsOddSteps) {
    auto cfg = makeBasicLane();
    cfg.swingAmount = 0.5f;
    auto events = renderLane(cfg);

    ASSERT_EQ(events.size(), 8u);
    const double sPpq = 0.5;
    const double swingOffset = 0.5f * sPpq * (1.0 / 3.0);

    for (size_t i = 0; i < events.size(); ++i) {
        double expected = i * sPpq;
        if (i % 2 == 1) {
            expected += swingOffset;
        }
        EXPECT_NEAR(events[i].ppqPosition, expected, 1e-9) << "Step " << i;
    }
}

TEST(SwingHumanize, FullSwingTripleFeel) {
    auto cfg = makeBasicLane();
    cfg.swingAmount = 1.0f;
    auto events = renderLane(cfg);

    ASSERT_EQ(events.size(), 8u);
    const double sPpq = 0.5;
    const double swingOffset = 1.0f * sPpq * (1.0 / 3.0);

    for (size_t i = 0; i < events.size(); ++i) {
        double expected = i * sPpq;
        if (i % 2 == 1) {
            expected += swingOffset;
        }
        EXPECT_NEAR(events[i].ppqPosition, expected, 1e-9) << "Step " << i;
    }
}

TEST(SwingHumanize, SwingDeterministic) {
    auto cfg = makeBasicLane();
    cfg.swingAmount = 0.6f;
    auto run1 = renderLane(cfg);
    auto run2 = renderLane(cfg);

    ASSERT_EQ(run1.size(), run2.size());
    for (size_t i = 0; i < run1.size(); ++i) {
        EXPECT_DOUBLE_EQ(run1[i].ppqPosition, run2[i].ppqPosition);
    }
}

// --- Humanize Tests ---

TEST(SwingHumanize, HumanizeAddsJitter) {
    auto cfg = makeBasicLane();
    cfg.humanizeMs = 10.0f;
    auto events = renderLane(cfg);

    const double sPpq = 0.5;
    bool anyShifted = false;
    for (size_t i = 0; i < events.size(); ++i) {
        double nominal = i * sPpq;
        if (std::abs(events[i].ppqPosition - nominal) > 1e-9) {
            anyShifted = true;
        }
    }
    EXPECT_TRUE(anyShifted) << "Humanize should shift at least some note positions";
}

TEST(SwingHumanize, HumanizeRespectsBounds) {
    auto cfg = makeBasicLane();
    cfg.humanizeMs = 20.0f;
    double tempo = 120.0;
    double jitterPpq = cfg.humanizeMs * tempo / 60000.0;

    auto events = renderLane(cfg, 3.75, 42, tempo);

    ASSERT_EQ(events.size(), 8u);
    const double sPpq = 0.5;
    for (const auto& e : events) {
        double nearestNominal = std::round(e.ppqPosition / sPpq) * sPpq;
        double offset = e.ppqPosition - nearestNominal;
        EXPECT_LE(std::abs(offset), jitterPpq + 1e-9)
            << "Note at " << e.ppqPosition << " exceeds max jitter from nearest step";
    }
}

TEST(SwingHumanize, HumanizeDeterministic) {
    auto cfg = makeBasicLane();
    cfg.humanizeMs = 15.0f;
    auto run1 = renderLane(cfg);
    auto run2 = renderLane(cfg);

    ASSERT_EQ(run1.size(), run2.size());
    for (size_t i = 0; i < run1.size(); ++i) {
        EXPECT_DOUBLE_EQ(run1[i].ppqPosition, run2[i].ppqPosition);
    }
}

TEST(SwingHumanize, HumanizeZeroMeansNoJitter) {
    auto cfg = makeBasicLane();
    cfg.humanizeMs = 0.0f;
    auto events = renderLane(cfg);

    const double sPpq = 0.5;
    for (size_t i = 0; i < events.size(); ++i) {
        EXPECT_DOUBLE_EQ(events[i].ppqPosition, i * sPpq);
    }
}

TEST(SwingHumanize, HumanizeDifferentSeedDiffers) {
    auto cfg = makeBasicLane();
    cfg.humanizeMs = 10.0f;
    auto run1 = renderLane(cfg, 3.75, 42);
    auto run2 = renderLane(cfg, 3.75, 999);

    bool anyDifferent = false;
    ASSERT_EQ(run1.size(), run2.size());
    for (size_t i = 0; i < run1.size(); ++i) {
        if (std::abs(run1[i].ppqPosition - run2[i].ppqPosition) > 1e-9) {
            anyDifferent = true;
            break;
        }
    }
    EXPECT_TRUE(anyDifferent) << "Different seeds should produce different jitter";
}

// --- Combined Swing + Humanize ---

TEST(SwingHumanize, SwingPlusHumanize) {
    auto cfg = makeBasicLane();
    cfg.swingAmount = 0.5f;
    cfg.humanizeMs = 5.0f;
    double tempo = 120.0;
    double jitterPpq = cfg.humanizeMs * tempo / 60000.0;

    auto events = renderLane(cfg, 3.75, 42, tempo);
    ASSERT_EQ(events.size(), 8u);

    const double sPpq = 0.5;
    const double swingOffset = 0.5f * sPpq * (1.0 / 3.0);

    // Build nominal positions for even and odd steps
    std::vector<double> nominals;
    for (int i = 0; i < 8; ++i) {
        double n = i * sPpq;
        if (i % 2 == 1)
            n += swingOffset;
        nominals.push_back(n);
    }

    for (const auto& e : events) {
        double minDist = 1e9;
        for (double n : nominals) {
            minDist = std::min(minDist, std::abs(e.ppqPosition - n));
        }
        EXPECT_LE(minDist, jitterPpq + 1e-9) << "Note at " << e.ppqPosition << " exceeds jitter bound from any nominal";
    }
}

// --- Note Duration Tests ---

TEST(SwingHumanize, DefaultDurationHalfStep) {
    auto cfg = makeBasicLane();
    cfg.noteDuration = 0.0f;
    auto events = renderLane(cfg);

    const double sPpq = 0.5;
    for (const auto& e : events) {
        EXPECT_DOUBLE_EQ(e.duration, sPpq * 0.5);
    }
}

TEST(SwingHumanize, CustomDuration) {
    auto cfg = makeBasicLane();
    cfg.noteDuration = 0.75f;
    auto events = renderLane(cfg);

    for (const auto& e : events) {
        EXPECT_DOUBLE_EQ(e.duration, 0.75);
    }
}

TEST(SwingHumanize, ShortStaccatoDuration) {
    auto cfg = makeBasicLane();
    cfg.noteDuration = 0.1f;
    auto events = renderLane(cfg);

    for (const auto& e : events) {
        EXPECT_NEAR(e.duration, 0.1, 1e-6);
    }
}

// --- Micro-timing Map Tests ---

TEST(MicroTiming, PerStepOffsetShiftsPpq) {
    auto cfg = makeBasicLane();
    cfg.microTimingMs[1] = 10.0f;
    cfg.microTimingMs[3] = -5.0f;
    double tempo = 120.0;
    auto events = renderLane(cfg, 4.0, 42, tempo);

    ASSERT_EQ(events.size(), 8u);
    const double sPpq = 0.5;
    double offset1 = 10.0 * tempo / 60000.0;
    double offset3 = -5.0 * tempo / 60000.0;

    EXPECT_DOUBLE_EQ(events[0].ppqPosition, 0.0);
    EXPECT_NEAR(events[1].ppqPosition, 1 * sPpq + offset1, 1e-9);
    EXPECT_DOUBLE_EQ(events[2].ppqPosition, 2 * sPpq);
    EXPECT_NEAR(events[3].ppqPosition, 3 * sPpq + offset3, 1e-9);
}

TEST(MicroTiming, ComposeWithSwing) {
    auto cfg = makeBasicLane();
    cfg.swingAmount = 0.5f;
    cfg.microTimingMs[1] = 5.0f;
    double tempo = 120.0;
    auto events = renderLane(cfg, 4.0, 42, tempo);

    ASSERT_EQ(events.size(), 8u);
    const double sPpq = 0.5;
    double swingOff = 0.5f * sPpq * (1.0 / 3.0);
    double microOff = 5.0 * tempo / 60000.0;

    EXPECT_NEAR(events[1].ppqPosition, 1 * sPpq + swingOff + microOff, 1e-9);
    EXPECT_DOUBLE_EQ(events[0].ppqPosition, 0.0);
}

TEST(MicroTiming, ComposeWithHumanize) {
    auto cfg = makeBasicLane();
    cfg.humanizeMs = 5.0f;
    cfg.microTimingMs[0] = 10.0f;
    double tempo = 120.0;
    auto events = renderLane(cfg, 4.0, 42, tempo);

    double microOff = 10.0 * tempo / 60000.0;
    double jitterMax = 5.0 * tempo / 60000.0;
    EXPECT_NEAR(events[0].ppqPosition, microOff, jitterMax + 1e-9);
}

TEST(MicroTiming, ComposeWithAdditiveCells) {
    poly::LaneConfig cfg{};
    cfg.id = 0;
    cfg.active = true;
    cfg.midiNote = 36;
    cfg.cycle = {.steps = 3, .subdivision = 8};
    cfg.hitCount = 3;
    cfg.probability = 1.0f;
    cfg.baseVelocity = 100;
    cfg.cellCount = 3;
    cfg.cellSizes[0] = 2;
    cfg.cellSizes[1] = 2;
    cfg.cellSizes[2] = 3;
    cfg.microTimingMs[0] = 5.0f;
    cfg.microTimingMs[1] = -5.0f;

    double tempo = 120.0;
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;
    state.lanes[0] = cfg;

    poly::NoteEventBuffer buf;
    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 3.5;
    tc.tempo = tempo;
    tc.playing = true;
    engine.renderRange(tc, state, buf);

    ASSERT_EQ(buf.count, 3u);
    double basePpq = 0.5;
    double microOff0 = 5.0 * tempo / 60000.0;
    double microOff1 = -5.0 * tempo / 60000.0;
    EXPECT_NEAR(buf.events[0].ppqPosition, 0.0 + microOff0, 1e-9);
    EXPECT_NEAR(buf.events[1].ppqPosition, 2.0 * basePpq + microOff1, 1e-9);
}

TEST(MicroTiming, AllZeroNoChange) {
    auto cfg = makeBasicLane();
    auto eventsClean = renderLane(cfg);

    auto cfgWithMap = makeBasicLane();
    auto eventsWithMap = renderLane(cfgWithMap);

    ASSERT_EQ(eventsClean.size(), eventsWithMap.size());
    for (size_t i = 0; i < eventsClean.size(); ++i) {
        EXPECT_DOUBLE_EQ(eventsClean[i].ppqPosition, eventsWithMap[i].ppqPosition);
    }
}

TEST(MicroTiming, GingaGrooveGolden) {
    auto cfg = makeBasicLane();
    cfg.cycle = {.steps = 4, .subdivision = 4};
    cfg.hitCount = 4;
    cfg.microTimingMs[0] = 0.0f;
    cfg.microTimingMs[1] = 8.0f;
    cfg.microTimingMs[2] = 0.0f;
    cfg.microTimingMs[3] = -4.0f;
    double tempo = 120.0;
    auto events = renderLane(cfg, 4.0, 42, tempo);

    ASSERT_EQ(events.size(), 4u);
    double microOff1 = 8.0 * tempo / 60000.0;
    double microOff3 = -4.0 * tempo / 60000.0;
    EXPECT_DOUBLE_EQ(events[0].ppqPosition, 0.0);
    EXPECT_NEAR(events[1].ppqPosition, 1.0 + microOff1, 1e-9);
    EXPECT_DOUBLE_EQ(events[2].ppqPosition, 2.0);
    EXPECT_NEAR(events[3].ppqPosition, 3.0 + microOff3, 1e-9);
}

// --- Effect Presence Tests ---
// Verify each control produces a measurable change in output.

static std::vector<poly::NoteEvent> renderWithMacros(const poly::GrooveState& state, double ppqEnd = 4.0,
                                                     double tempo = 120.0) {
    poly::GrooveState resolved = poly::resolveMacros(state);
    poly::Engine engine;
    poly::NoteEventBuffer buf;
    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = ppqEnd;
    tc.tempo = tempo;
    tc.playing = true;
    engine.renderRange(tc, resolved, buf);

    std::vector<poly::NoteEvent> events;
    for (size_t i = 0; i < buf.count; ++i)
        events.push_back(buf.events[i]);
    std::sort(events.begin(), events.end(), [](const auto& a, const auto& b) { return a.ppqPosition < b.ppqPosition; });
    return events;
}

TEST(EffectPresence, SwingProducesMeasurableShift) {
    auto cfg = makeBasicLane();
    cfg.swingAmount = 0.0f;
    auto baseline = renderLane(cfg);

    cfg.swingAmount = 1.0f;
    auto swung = renderLane(cfg);

    ASSERT_EQ(baseline.size(), swung.size());
    double maxShift = 0.0;
    for (size_t i = 0; i < baseline.size(); ++i)
        maxShift = std::max(maxShift, std::abs(swung[i].ppqPosition - baseline[i].ppqPosition));
    EXPECT_GT(maxShift, 0.05) << "Full swing should shift notes by >0.05 PPQ";
}

TEST(EffectPresence, HumanizeProducesMeasurableJitter) {
    auto cfg = makeBasicLane();
    cfg.humanizeMs = 0.0f;
    auto baseline = renderLane(cfg, 3.75);

    cfg.humanizeMs = 30.0f;
    auto humanized = renderLane(cfg, 3.75);

    ASSERT_EQ(baseline.size(), humanized.size());
    double maxShift = 0.0;
    for (size_t i = 0; i < baseline.size(); ++i)
        maxShift = std::max(maxShift, std::abs(humanized[i].ppqPosition - baseline[i].ppqPosition));
    EXPECT_GT(maxShift, 0.01) << "30ms humanize should produce >0.01 PPQ jitter at 120 BPM";
}

TEST(EffectPresence, HumanizeFullRangeWorks) {
    auto cfg = makeBasicLane();
    cfg.humanizeMs = 50.0f;
    double tempo = 120.0;
    double maxJitterPpq = 50.0 * tempo / 60000.0;

    auto events = renderLane(cfg, 4.0, 42, tempo);

    const double sPpq = 0.5;
    bool anyLargeJitter = false;
    for (const auto& e : events) {
        double nearestNominal = std::round(e.ppqPosition / sPpq) * sPpq;
        double offset = std::abs(e.ppqPosition - nearestNominal);
        EXPECT_LE(offset, maxJitterPpq + 1e-9) << "Jitter exceeds 50ms bound";
        if (offset > 0.02)
            anyLargeJitter = true;
    }
    EXPECT_TRUE(anyLargeJitter) << "50ms humanize should produce at least some large jitter values";
}

TEST(EffectPresence, MacroSwingProducesShift) {
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;
    state.lanes[0] = makeBasicLane();
    state.lanes[0].swingAmount = 0.0f;
    state.macros.swing = 0.0f;
    auto baseline = renderWithMacros(state);

    state.macros.swing = 1.0f;
    auto swung = renderWithMacros(state);

    ASSERT_EQ(baseline.size(), swung.size());
    double maxShift = 0.0;
    for (size_t i = 0; i < baseline.size(); ++i)
        maxShift = std::max(maxShift, std::abs(swung[i].ppqPosition - baseline[i].ppqPosition));
    EXPECT_GT(maxShift, 0.05) << "Macro swing at 1.0 should produce >0.05 PPQ shift";
}

TEST(EffectPresence, MacroHumanizeProducesJitter) {
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;
    state.lanes[0] = makeBasicLane();
    state.lanes[0].humanizeMs = 0.0f;
    state.macros.humanize = 0.0f;
    auto baseline = renderWithMacros(state, 3.75);

    state.macros.humanize = 1.0f;
    auto humanized = renderWithMacros(state, 3.75);

    ASSERT_EQ(baseline.size(), humanized.size());
    double maxShift = 0.0;
    for (size_t i = 0; i < baseline.size(); ++i)
        maxShift = std::max(maxShift, std::abs(humanized[i].ppqPosition - baseline[i].ppqPosition));
    EXPECT_GT(maxShift, 0.01) << "Macro humanize at 1.0 should produce >0.01 PPQ jitter at 120 BPM";
}

TEST(EffectPresence, MacroHumanizePreservesPerLaneRange) {
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;
    state.lanes[0] = makeBasicLane();
    state.lanes[0].humanizeMs = 40.0f;
    state.macros.humanize = 0.0f;

    auto resolved = poly::resolveMacros(state);
    EXPECT_FLOAT_EQ(resolved.lanes[0].humanizeMs, 40.0f)
        << "Macro resolution must not clamp per-lane humanize below its set value";
}

TEST(EffectPresence, ProbabilityReducesNoteCount) {
    auto cfg = makeBasicLane();
    cfg.probability = 1.0f;
    auto full = renderLane(cfg);

    cfg.probability = 0.3f;
    auto sparse = renderLane(cfg);

    EXPECT_LT(sparse.size(), full.size()) << "Lower probability should produce fewer notes";
}

TEST(EffectPresence, VelocitySpreadVariesOutput) {
    auto cfg = makeBasicLane();
    cfg.velocitySpread = 0.0f;
    auto uniform = renderLane(cfg);

    cfg.velocitySpread = 0.3f;
    auto spread = renderLane(cfg);

    bool anyDifferent = false;
    ASSERT_EQ(uniform.size(), spread.size());
    for (size_t i = 0; i < uniform.size(); ++i) {
        if (std::abs(uniform[i].velocity - spread[i].velocity) > 1e-6)
            anyDifferent = true;
    }
    EXPECT_TRUE(anyDifferent) << "Velocity spread should produce different velocities";
}

TEST(EffectPresence, NoteDurationChangesOutput) {
    auto cfg = makeBasicLane();
    cfg.noteDuration = 0.0f;
    auto defaultDur = renderLane(cfg);

    cfg.noteDuration = 1.0f;
    auto longDur = renderLane(cfg);

    ASSERT_EQ(defaultDur.size(), longDur.size());
    bool anyDifferent = false;
    for (size_t i = 0; i < defaultDur.size(); ++i) {
        if (std::abs(defaultDur[i].duration - longDur[i].duration) > 1e-6)
            anyDifferent = true;
    }
    EXPECT_TRUE(anyDifferent) << "Note duration setting should change output duration";
}

TEST(EffectPresence, TimingOffsetShiftsAllNotes) {
    auto cfg = makeBasicLane();
    cfg.timingOffsetMs = 0.0f;
    auto baseline = renderLane(cfg);

    cfg.timingOffsetMs = 15.0f;
    auto shifted = renderLane(cfg);

    ASSERT_EQ(baseline.size(), shifted.size());
    for (size_t i = 0; i < baseline.size(); ++i) {
        EXPECT_GT(shifted[i].ppqPosition, baseline[i].ppqPosition) << "Positive timing offset should shift notes later";
    }
}

TEST(EffectPresence, SyncopationProducesMeasurableShift) {
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;
    state.lanes[0] = makeBasicLane();
    state.macros.syncopation = 0.0f;
    auto baseline = renderWithMacros(state);

    state.macros.syncopation = 1.0f;
    auto syncopated = renderWithMacros(state);

    ASSERT_EQ(baseline.size(), syncopated.size());
    double maxShift = 0.0;
    for (size_t i = 0; i < baseline.size(); ++i)
        maxShift = std::max(maxShift, std::abs(syncopated[i].ppqPosition - baseline[i].ppqPosition));
    EXPECT_GT(maxShift, 0.05) << "Full syncopation should shift notes by >0.05 PPQ";
}

TEST(EffectPresence, SyncopationShiftsEvenStepsOnly) {
    auto cfg = makeBasicLane();
    cfg.syncopationOffset = 0.5f;
    auto events = renderLane(cfg);

    ASSERT_EQ(events.size(), 8u);
    const double sPpq = 0.5;
    const double syncopOffset = 0.5f * sPpq * (1.0 / 3.0);

    for (size_t i = 0; i < events.size(); ++i) {
        double expected = i * sPpq;
        if (i % 2 == 0)
            expected += syncopOffset;
        EXPECT_NEAR(events[i].ppqPosition, expected, 1e-9) << "Step " << i;
    }
}

TEST(EffectPresence, SyncopationAndSwingCompose) {
    auto cfg = makeBasicLane();
    cfg.syncopationOffset = 0.5f;
    cfg.swingAmount = 0.5f;
    auto events = renderLane(cfg);

    ASSERT_EQ(events.size(), 8u);
    const double sPpq = 0.5;
    const double offset = 0.5f * sPpq * (1.0 / 3.0);

    for (size_t i = 0; i < events.size(); ++i) {
        double expected = i * sPpq + offset;
        EXPECT_NEAR(events[i].ppqPosition, expected, 1e-9)
            << "Step " << i << ": both swing (odd) and syncopation (even) should shift by same amount";
    }
}

TEST(EffectPresence, SyncopationDeterministic) {
    auto cfg = makeBasicLane();
    cfg.syncopationOffset = 0.7f;
    auto run1 = renderLane(cfg);
    auto run2 = renderLane(cfg);

    ASSERT_EQ(run1.size(), run2.size());
    for (size_t i = 0; i < run1.size(); ++i) {
        EXPECT_DOUBLE_EQ(run1[i].ppqPosition, run2[i].ppqPosition);
    }
}

TEST(EffectPresence, DriftRateChangesPatternOverTime) {
    auto cfg = makeBasicLane();
    cfg.hitCount = 3;
    cfg.driftRate = 0.0f;
    auto stable = renderLane(cfg, 8.0);

    cfg.driftRate = 2.0f;
    auto drifted = renderLane(cfg, 8.0);

    bool anyPositionDiff = false;
    size_t minSize = std::min(stable.size(), drifted.size());
    for (size_t i = 0; i < minSize; ++i) {
        if (std::abs(stable[i].ppqPosition - drifted[i].ppqPosition) > 1e-9) {
            anyPositionDiff = true;
            break;
        }
    }
    EXPECT_TRUE(anyPositionDiff || stable.size() != drifted.size()) << "Drift should change which steps fire over time";
}
