#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/euclidean.h"
#include "poly/types.h"

namespace {

std::string patternStr(const std::array<bool, poly::kMaxSteps>& p, int n) {
    std::string s;
    for (int i = 0; i < n; ++i)
        s += p[i] ? '1' : '0';
    return s;
}

int countHits(const std::array<bool, poly::kMaxSteps>& p, int n) {
    int c = 0;
    for (int i = 0; i < n; ++i)
        c += p[i] ? 1 : 0;
    return c;
}

} // namespace

TEST(Euclidean, FourOnTheFloor) {
    std::array<bool, poly::kMaxSteps> p{};
    poly::euclidean(4, 4, 0, p);
    EXPECT_EQ(patternStr(p, 4), "1111");
}

TEST(Euclidean, SinglePulse) {
    std::array<bool, poly::kMaxSteps> p{};
    poly::euclidean(1, 4, 0, p);
    EXPECT_EQ(countHits(p, 4), 1);
    EXPECT_TRUE(p[0]);
}

TEST(Euclidean, Tresillo_3_8) {
    std::array<bool, poly::kMaxSteps> p{};
    poly::euclidean(3, 8, 0, p);
    EXPECT_EQ(countHits(p, 8), 3);
    EXPECT_EQ(patternStr(p, 8), "10010010");
}

TEST(Euclidean, Cinquillo_5_8) {
    std::array<bool, poly::kMaxSteps> p{};
    poly::euclidean(5, 8, 0, p);
    EXPECT_EQ(countHits(p, 8), 5);
}

TEST(Euclidean, ThreeInSixteen) {
    std::array<bool, poly::kMaxSteps> p{};
    poly::euclidean(3, 16, 0, p);
    EXPECT_EQ(countHits(p, 16), 3);
    EXPECT_TRUE(p[0]);
}

TEST(Euclidean, ZeroPulses) {
    std::array<bool, poly::kMaxSteps> p{};
    p[0] = true;
    poly::euclidean(0, 8, 0, p);
    EXPECT_EQ(countHits(p, 8), 0);
}

TEST(Euclidean, MorePulsesThanSteps) {
    std::array<bool, poly::kMaxSteps> p{};
    poly::euclidean(10, 4, 0, p);
    EXPECT_EQ(patternStr(p, 4), "1111");
}

TEST(Euclidean, RotationShiftsPattern) {
    std::array<bool, poly::kMaxSteps> base{};
    poly::euclidean(3, 8, 0, base);

    std::array<bool, poly::kMaxSteps> rot1{};
    poly::euclidean(3, 8, 1, rot1);

    EXPECT_EQ(countHits(rot1, 8), 3);
    EXPECT_NE(patternStr(base, 8), patternStr(rot1, 8));

    // Rotation by n should give the original
    std::array<bool, poly::kMaxSteps> full{};
    poly::euclidean(3, 8, 8, full);
    EXPECT_EQ(patternStr(full, 8), patternStr(base, 8));
}

TEST(Euclidean, RotationPreservesHitCount) {
    for (int rot = 0; rot < 8; ++rot) {
        std::array<bool, poly::kMaxSteps> p{};
        poly::euclidean(3, 8, rot, p);
        EXPECT_EQ(countHits(p, 8), 3) << "rotation=" << rot;
    }
}

TEST(Euclidean, MaximallyEvenSpacing) {
    // E(3,8) should have spacings that differ by at most 1
    std::array<bool, poly::kMaxSteps> p{};
    poly::euclidean(3, 8, 0, p);

    std::vector<int> gaps;
    int lastHit = -1;
    for (int i = 0; i < 8; ++i) {
        if (p[i]) {
            if (lastHit >= 0)
                gaps.push_back(i - lastHit);
            lastHit = i;
        }
    }
    // Wrap-around gap
    if (lastHit >= 0) {
        int first = -1;
        for (int i = 0; i < 8; ++i) {
            if (p[i]) {
                first = i;
                break;
            }
        }
        gaps.push_back(8 - lastHit + first);
    }

    int minGap = *std::min_element(gaps.begin(), gaps.end());
    int maxGap = *std::max_element(gaps.begin(), gaps.end());
    EXPECT_LE(maxGap - minGap, 1);
}

// --- Additive / Aksak cell tests ---

namespace {

poly::LaneConfig makeAdditiveConfig(std::initializer_list<int> cells, int subdivision, int hits) {
    poly::LaneConfig cfg{};
    cfg.id = 0;
    cfg.midiNote = 36;
    cfg.cycle.subdivision = subdivision;
    cfg.hitCount = hits;
    cfg.baseVelocity = 100;
    cfg.probability = 1.0f;
    cfg.active = true;
    cfg.cellCount = static_cast<int>(cells.size());
    cfg.cycle.steps = cfg.cellCount;
    int i = 0;
    for (int c : cells)
        cfg.cellSizes[i++] = c;
    return cfg;
}

std::vector<double> renderPpqPositions(const poly::LaneConfig& cfg, double ppqStart, double ppqEnd,
                                       double blockPpq = 0.25) {
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.lanes[0] = cfg;
    state.seed = 42;

    poly::Engine engine;
    poly::NoteEventBuffer buf;
    std::vector<double> positions;
    double ppq = ppqStart;

    while (ppq < ppqEnd) {
        double end = std::min(ppq + blockPpq, ppqEnd);
        poly::TransportContext tc{};
        tc.ppqStart = ppq;
        tc.ppqEnd = end;
        tc.tempo = 120.0;
        tc.playing = true;
        engine.renderRange(tc, state, buf);
        for (size_t j = 0; j < buf.count; ++j)
            positions.push_back(buf.events[j].ppqPosition);
        ppq = end;
    }
    std::sort(positions.begin(), positions.end());
    return positions;
}

} // namespace

TEST(AdditiveCells, SevenEightAksakPpqPositions) {
    auto cfg = makeAdditiveConfig({2, 2, 3}, 8, 3);
    auto pos = renderPpqPositions(cfg, 0.0, 3.5);
    double basePpq = 4.0 / 8.0;
    double cyclePpq = (2 + 2 + 3) * basePpq;
    ASSERT_GE(pos.size(), 3u);
    EXPECT_NEAR(pos[0], 0.0, 1e-9);
    EXPECT_NEAR(pos[1], 2.0 * basePpq, 1e-9);
    EXPECT_NEAR(pos[2], 4.0 * basePpq, 1e-9);
    if (pos.size() >= 6) {
        EXPECT_NEAR(pos[3], cyclePpq, 1e-9);
        EXPECT_NEAR(pos[4], cyclePpq + 2.0 * basePpq, 1e-9);
        EXPECT_NEAR(pos[5], cyclePpq + 4.0 * basePpq, 1e-9);
    }
}

TEST(AdditiveCells, NineEightAksakWorks) {
    auto cfg = makeAdditiveConfig({2, 2, 2, 3}, 8, 4);
    auto pos = renderPpqPositions(cfg, 0.0, 4.5);
    double basePpq = 0.5;
    ASSERT_GE(pos.size(), 4u);
    EXPECT_NEAR(pos[0], 0.0, 1e-9);
    EXPECT_NEAR(pos[1], 2.0 * basePpq, 1e-9);
    EXPECT_NEAR(pos[2], 4.0 * basePpq, 1e-9);
    EXPECT_NEAR(pos[3], 6.0 * basePpq, 1e-9);
}

TEST(AdditiveCells, EuclideanOverAdditiveCells) {
    auto cfg = makeAdditiveConfig({2, 2, 3}, 8, 2);
    auto pos = renderPpqPositions(cfg, 0.0, 7.0);
    EXPECT_EQ(pos.size(), 4u);
}

TEST(AdditiveCells, BlockSizeIndependence) {
    auto cfg = makeAdditiveConfig({2, 2, 3}, 8, 3);
    auto posSmall = renderPpqPositions(cfg, 0.0, 7.0, 0.125);
    auto posLarge = renderPpqPositions(cfg, 0.0, 7.0, 2.0);
    ASSERT_EQ(posSmall.size(), posLarge.size());
    for (size_t i = 0; i < posSmall.size(); ++i) {
        EXPECT_NEAR(posSmall[i], posLarge[i], 1e-9) << "i=" << i;
    }
}

TEST(AdditiveCells, AdditiveWithPhraseGating) {
    auto cfg = makeAdditiveConfig({2, 2, 3}, 8, 3);
    cfg.phraseLength = 3.0f;
    cfg.phraseGap = 1.0f;
    auto pos = renderPpqPositions(cfg, 0.0, 8.0);
    for (double p : pos) {
        double phrasePos = std::fmod(p, 4.0);
        EXPECT_LT(phrasePos, 3.0 + 1e-9) << "ppq=" << p << " should be within phrase window";
    }
}

TEST(AdditiveCells, AdditiveWithDrift) {
    auto cfg = makeAdditiveConfig({2, 2, 3}, 8, 3);
    cfg.driftRate = 1.0f;
    auto posNoDrift = renderPpqPositions(makeAdditiveConfig({2, 2, 3}, 8, 3), 0.0, 7.0);
    auto posDrift = renderPpqPositions(cfg, 0.0, 7.0);
    EXPECT_EQ(posNoDrift.size(), posDrift.size());
}

TEST(AdditiveCells, ComputeAdditiveCellsHelper) {
    auto cfg = makeAdditiveConfig({2, 2, 3}, 8, 3);
    auto info = poly::computeAdditiveCells(cfg);
    EXPECT_EQ(info.count, 3);
    double basePpq = 0.5;
    EXPECT_NEAR(info.cumPpq[0], 0.0, 1e-9);
    EXPECT_NEAR(info.cumPpq[1], 2.0 * basePpq, 1e-9);
    EXPECT_NEAR(info.cumPpq[2], 4.0 * basePpq, 1e-9);
    EXPECT_NEAR(info.totalPpq, 7.0 * basePpq, 1e-9);
}

TEST(AdditiveCells, ZeroCellCountFallsBack) {
    poly::LaneConfig cfg{};
    cfg.cellCount = 0;
    auto info = poly::computeAdditiveCells(cfg);
    EXPECT_EQ(info.count, 0);
    EXPECT_NEAR(info.totalPpq, 0.0, 1e-9);
}

// --- Timeline mode tests ---

static poly::LaneConfig makeTimelineConfig(std::initializer_list<bool> steps, int subdivision = 4) {
    poly::LaneConfig cfg{};
    cfg.id = 0;
    cfg.active = true;
    cfg.timeline = true;
    cfg.cycle.steps = static_cast<int>(steps.size());
    cfg.cycle.subdivision = subdivision;
    cfg.fixedPatternLength = static_cast<int>(steps.size());
    cfg.probability = 1.0f;
    cfg.baseVelocity = 100;
    int i = 0;
    for (bool s : steps)
        cfg.fixedPattern[i++] = s;
    return cfg;
}

TEST(Timeline, UsesFixedPatternNotEuclidean) {
    auto cfg = makeTimelineConfig({true, false, false, true, false, false, true, false});
    cfg.hitCount = 8;
    cfg.rotation = 0;
    auto pos = renderPpqPositions(cfg, 0.0, 8.0);
    EXPECT_EQ(pos.size(), 3u);
    EXPECT_NEAR(pos[0], 0.0, 1e-9);
    EXPECT_NEAR(pos[1], 3.0, 1e-9);
    EXPECT_NEAR(pos[2], 6.0, 1e-9);
}

TEST(Timeline, IgnoresHitCountAndRotation) {
    auto cfg = makeTimelineConfig({true, true, false, false});
    cfg.hitCount = 4;
    cfg.rotation = 2;
    auto pos = renderPpqPositions(cfg, 0.0, 4.0);
    EXPECT_EQ(pos.size(), 2u);
    EXPECT_NEAR(pos[0], 0.0, 1e-9);
    EXPECT_NEAR(pos[1], 1.0, 1e-9);
}

TEST(Timeline, EnvelopesStillApply) {
    auto cfg = makeTimelineConfig({true, true, true, true});
    cfg.envelopeCount = 1;
    cfg.envelopes[0].active = true;
    cfg.envelopes[0].envelope.target = poly::EnvTarget::Velocity;
    cfg.envelopes[0].envelope.periodBars = 1.0f;
    cfg.envelopes[0].envelope.shape = poly::Shape::Ramp;
    cfg.envelopes[0].envelope.depth = 1.0f;

    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.lanes[0] = cfg;
    state.seed = 42;

    poly::TransportContext tc{};
    tc.playing = true;
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.sampleRate = 44100.0;

    poly::NoteEventBuffer buf;
    poly::Engine engine;
    engine.renderRange(tc, state, buf);

    EXPECT_EQ(buf.count, 4u);
    EXPECT_NE(buf.events[0].velocity, buf.events[3].velocity);
}

TEST(Timeline, PhraseGatingStillWorks) {
    auto cfg = makeTimelineConfig({true, true, true, true, true, true, true, true});
    cfg.phraseLength = 2.0f;
    cfg.phraseGap = 2.0f;
    auto pos = renderPpqPositions(cfg, 0.0, 8.0);
    for (double p : pos) {
        double phrasePos = std::fmod(p, 4.0);
        EXPECT_LT(phrasePos, 2.0 + 1e-9) << "ppq=" << p;
    }
}

TEST(Timeline, BellPatternGolden) {
    auto cfg = makeTimelineConfig({true, false, true, true, false, true, false, true, true, false, true, false});
    cfg.cycle.steps = 12;
    cfg.cycle.subdivision = 8;
    cfg.fixedPatternLength = 12;
    auto pos = renderPpqPositions(cfg, 0.0, 6.0);
    EXPECT_EQ(pos.size(), 7u);
    double sPpq = 0.5;
    EXPECT_NEAR(pos[0], 0.0 * sPpq, 1e-9);
    EXPECT_NEAR(pos[1], 2.0 * sPpq, 1e-9);
    EXPECT_NEAR(pos[2], 3.0 * sPpq, 1e-9);
    EXPECT_NEAR(pos[3], 5.0 * sPpq, 1e-9);
    EXPECT_NEAR(pos[4], 7.0 * sPpq, 1e-9);
    EXPECT_NEAR(pos[5], 8.0 * sPpq, 1e-9);
    EXPECT_NEAR(pos[6], 10.0 * sPpq, 1e-9);
}
