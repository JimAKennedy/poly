#include "poly/euclidean.h"
#include <gtest/gtest.h>
#include <string>

namespace {

std::string patternStr(const std::array<bool, poly::kMaxSteps>& p, int n) {
    std::string s;
    for (int i = 0; i < n; ++i) s += p[i] ? '1' : '0';
    return s;
}

int countHits(const std::array<bool, poly::kMaxSteps>& p, int n) {
    int c = 0;
    for (int i = 0; i < n; ++i) c += p[i] ? 1 : 0;
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
            if (lastHit >= 0) gaps.push_back(i - lastHit);
            lastHit = i;
        }
    }
    // Wrap-around gap
    if (lastHit >= 0) {
        int first = -1;
        for (int i = 0; i < 8; ++i) { if (p[i]) { first = i; break; } }
        gaps.push_back(8 - lastHit + first);
    }

    int minGap = *std::min_element(gaps.begin(), gaps.end());
    int maxGap = *std::max_element(gaps.begin(), gaps.end());
    EXPECT_LE(maxGap - minGap, 1);
}
