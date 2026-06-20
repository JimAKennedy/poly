#include "poly/engine.h"
#include "poly/rng.h"
#include "poly/types.h"
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

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

std::vector<poly::NoteEvent> renderLane(const poly::LaneConfig& cfg,
                                         double ppqEnd = 4.0,
                                         uint64_t seed = 42,
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
    std::sort(events.begin(), events.end(),
              [](const auto& a, const auto& b) {
                  return a.ppqPosition < b.ppqPosition;
              });
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
        EXPECT_DOUBLE_EQ(events[i].ppqPosition, i * sPpq)
            << "Step " << i << " should be unshifted";
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
        EXPECT_NEAR(events[i].ppqPosition, expected, 1e-9)
            << "Step " << i;
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
        EXPECT_NEAR(events[i].ppqPosition, expected, 1e-9)
            << "Step " << i;
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

    auto events = renderLane(cfg, 4.0, 42, tempo);

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
    auto run1 = renderLane(cfg, 4.0, 42);
    auto run2 = renderLane(cfg, 4.0, 999);

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

    auto events = renderLane(cfg, 4.0, 42, tempo);
    ASSERT_EQ(events.size(), 8u);

    const double sPpq = 0.5;
    const double swingOffset = 0.5f * sPpq * (1.0 / 3.0);

    // Build nominal positions for even and odd steps
    std::vector<double> nominals;
    for (int i = 0; i < 8; ++i) {
        double n = i * sPpq;
        if (i % 2 == 1) n += swingOffset;
        nominals.push_back(n);
    }

    for (const auto& e : events) {
        double minDist = 1e9;
        for (double n : nominals) {
            minDist = std::min(minDist, std::abs(e.ppqPosition - n));
        }
        EXPECT_LE(minDist, jitterPpq + 1e-9)
            << "Note at " << e.ppqPosition << " exceeds jitter bound from any nominal";
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
