// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#include <vector>

#include <gtest/gtest.h>

#include "poly/midi_capture.h"

using namespace poly;

static NoteEvent makeEvent(double ppq, int16_t pitch, float vel, int16_t ch) {
    return {ppq, pitch, vel, 0.25, ch};
}

// Field-exact equality of an extracted window: the S08 frozen-snapshot guarantee
// is that the exported SMF is identical event-for-event. NoteEvent has padding
// bytes, so a raw memcmp is unreliable — compare every serialized field instead.
static bool eventsEqual(const NoteEvent* a, const NoteEvent* b, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (a[i].ppqPosition != b[i].ppqPosition || a[i].pitch != b[i].pitch || a[i].velocity != b[i].velocity ||
            a[i].duration != b[i].duration || a[i].channel != b[i].channel || a[i].laneIndex != b[i].laneIndex) {
            return false;
        }
    }
    return true;
}

TEST(MidiCaptureBuffer, PushAndCount) {
    MidiCaptureBuffer buf;
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.count(), 0u);

    buf.push(makeEvent(0.0, 36, 0.8f, 0));
    EXPECT_FALSE(buf.empty());
    EXPECT_EQ(buf.count(), 1u);

    buf.push(makeEvent(1.0, 38, 0.6f, 1));
    EXPECT_EQ(buf.count(), 2u);
}

TEST(MidiCaptureBuffer, Clear) {
    MidiCaptureBuffer buf;
    for (int i = 0; i < 10; ++i)
        buf.push(makeEvent(static_cast<double>(i), 36, 0.8f, 0));

    EXPECT_EQ(buf.count(), 10u);
    buf.clear();
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.count(), 0u);
}

TEST(MidiCaptureBuffer, WrapAround) {
    MidiCaptureBuffer buf;
    for (size_t i = 0; i < MidiCaptureBuffer::kCapacity + 100; ++i) {
        buf.push(makeEvent(static_cast<double>(i) * 0.25, 36, 0.8f, 0));
    }

    EXPECT_EQ(buf.count(), MidiCaptureBuffer::kCapacity);
    double oldest = (100) * 0.25;
    NoteEvent out[MidiCaptureBuffer::kCapacity];
    size_t n = buf.copyRaw(out, MidiCaptureBuffer::kCapacity);
    EXPECT_EQ(n, MidiCaptureBuffer::kCapacity);
    EXPECT_NEAR(out[0].ppqPosition, oldest, 0.001);
}

TEST(MidiCaptureBuffer, CopyRaw) {
    MidiCaptureBuffer buf;
    buf.push(makeEvent(0.0, 36, 0.8f, 0));
    buf.push(makeEvent(0.5, 38, 0.6f, 1));
    buf.push(makeEvent(1.0, 42, 0.9f, 2));

    NoteEvent out[10];
    size_t n = buf.copyRaw(out, 10);
    EXPECT_EQ(n, 3u);
    EXPECT_EQ(out[0].pitch, 36);
    EXPECT_EQ(out[1].pitch, 38);
    EXPECT_EQ(out[2].pitch, 42);
}

TEST(MidiCaptureBuffer, ExtractRange) {
    MidiCaptureBuffer buf;
    for (int i = 0; i < 16; ++i)
        buf.push(makeEvent(static_cast<double>(i), static_cast<int16_t>(36 + i), 0.8f, 0));

    NoteEvent out[20];
    size_t n = buf.extract(4.0, 8.0, out, 20);
    EXPECT_EQ(n, 4u);
    EXPECT_EQ(out[0].pitch, 40);
    EXPECT_EQ(out[3].pitch, 43);
}

TEST(MidiCaptureBuffer, ExtractLastBars) {
    MidiCaptureBuffer buf;
    for (int i = 0; i < 64; ++i)
        buf.push(makeEvent(static_cast<double>(i) * 0.5, 36, 0.8f, 0));

    size_t total = buf.count();
    EXPECT_EQ(total, 64u);

    NoteEvent out[200];
    size_t n = buf.extractLastBars(2, out, 200);
    double newestPpq = buf.newestPpq();
    double fromPpq = newestPpq - 2 * MidiCaptureBuffer::kBeatsPerBar;
    for (size_t i = 0; i < n; ++i) {
        EXPECT_GE(out[i].ppqPosition, fromPpq);
    }
    EXPECT_GT(n, 0u);
}

TEST(MidiCaptureBuffer, ExtractEmptyBuffer) {
    MidiCaptureBuffer buf;
    NoteEvent out[10];
    EXPECT_EQ(buf.extract(0.0, 10.0, out, 10), 0u);
    EXPECT_EQ(buf.extractLastBars(4, out, 10), 0u);
    EXPECT_EQ(buf.copyRaw(out, 10), 0u);
}

TEST(MidiCaptureBuffer, NewestPpq) {
    MidiCaptureBuffer buf;
    EXPECT_DOUBLE_EQ(buf.newestPpq(), 0.0);

    buf.push(makeEvent(5.0, 36, 0.8f, 0));
    EXPECT_DOUBLE_EQ(buf.newestPpq(), 5.0);

    buf.push(makeEvent(10.0, 38, 0.6f, 1));
    EXPECT_DOUBLE_EQ(buf.newestPpq(), 10.0);
}

// S08 frozen-window guarantee: reaching `complete` must yield a STABLE SMF
// regardless of continued playback. The freeze primitive is extract() over an
// absolute-PPQ window. Prove that extracting the same window before and after
// further push()es beyond that window returns byte-identical results, so the
// processor can snapshot once and rely on it.
TEST(MidiCaptureBuffer, ExtractByteStableAfterFurtherPushes) {
    MidiCaptureBuffer buf;
    // Window [0, 8): bars 1..8 worth of notes (one per beat).
    for (int i = 0; i < 8; ++i)
        buf.push(makeEvent(static_cast<double>(i), static_cast<int16_t>(36 + i), 0.7f, 0));

    NoteEvent before[64];
    size_t nBefore = buf.extract(0.0, 8.0, before, 64);
    EXPECT_EQ(nBefore, 8u);

    // Playback continues: many more notes land strictly beyond the window.
    for (int i = 0; i < 500; ++i)
        buf.push(makeEvent(8.0 + static_cast<double>(i), static_cast<int16_t>(50), 0.9f, 3));

    NoteEvent after[64];
    size_t nAfter = buf.extract(0.0, 8.0, after, 64);
    EXPECT_EQ(nAfter, nBefore);
    EXPECT_TRUE(eventsEqual(before, after, nAfter));
}

// The exclusive upper bound must never leak later pushes into a frozen window:
// a note landing exactly on toPpq (or beyond) is excluded, so re-extracting the
// same window after those pushes stays byte-identical.
TEST(MidiCaptureBuffer, ExtractStableExcludesUpperBoundary) {
    MidiCaptureBuffer buf;
    for (int i = 0; i < 8; ++i)
        buf.push(makeEvent(static_cast<double>(i), static_cast<int16_t>(36 + i), 0.7f, 0));

    NoteEvent before[64];
    size_t nBefore = buf.extract(0.0, 8.0, before, 64);

    // A note exactly on the exclusive upper bound and beyond it.
    buf.push(makeEvent(8.0, 60, 0.5f, 1));
    buf.push(makeEvent(9.0, 61, 0.5f, 1));

    NoteEvent after[64];
    size_t nAfter = buf.extract(0.0, 8.0, after, 64);
    EXPECT_EQ(nAfter, nBefore);
    EXPECT_TRUE(eventsEqual(before, after, nAfter));
}

// Stability holds only while the window's events remain resident in the ring.
// Documenting the eviction boundary: once push()es wrap past the window events,
// extract() no longer returns them — the processor must snapshot before that.
TEST(MidiCaptureBuffer, ExtractWindowEvictedAfterWrap) {
    MidiCaptureBuffer buf;
    for (int i = 0; i < 8; ++i)
        buf.push(makeEvent(static_cast<double>(i), static_cast<int16_t>(36 + i), 0.7f, 0));

    NoteEvent before[64];
    size_t nBefore = buf.extract(0.0, 8.0, before, 64);
    EXPECT_EQ(nBefore, 8u);

    // Overflow the ring so the original window events are overwritten.
    for (size_t i = 0; i < MidiCaptureBuffer::kCapacity + 8; ++i)
        buf.push(makeEvent(100.0 + static_cast<double>(i), 50, 0.9f, 3));

    NoteEvent after[64];
    size_t nAfter = buf.extract(0.0, 8.0, after, 64);
    EXPECT_EQ(nAfter, 0u);
}

// M032 S02 T01: The milestone requires the capture ceiling to hold at least a
// 32-bar dense pattern without event loss. The worst realistic case is 8 lanes
// (kMaxLanes) each firing a note on every sixteenth (subdivision clamps to 16):
// 32 bars * 16 steps/bar * 8 lanes = 4096 note events. Push all of them, then
// prove none were evicted and extractLastBars(32) returns every one. Before
// kCapacity was raised (2048) the ring dropped the first half of the window.
TEST(MidiCaptureBuffer, DenseThirtyTwoBarPatternNoEventLoss) {
    MidiCaptureBuffer buf;

    constexpr int kBars = 32;
    constexpr int kLanes = 8;                                                   // kMaxLanes
    constexpr int kStepsPerBar = 16;                                            // sixteenths = max subdivision
    constexpr double kStepPpq = MidiCaptureBuffer::kBeatsPerBar / kStepsPerBar; // 0.25 PPQ
    constexpr int kExpected = kBars * kStepsPerBar * kLanes;                    // 4096

    int pushed = 0;
    for (int bar = 0; bar < kBars; ++bar) {
        for (int step = 0; step < kStepsPerBar; ++step) {
            double ppq = static_cast<double>(bar * kStepsPerBar + step) * kStepPpq;
            for (int lane = 0; lane < kLanes; ++lane) {
                NoteEvent e = makeEvent(ppq, static_cast<int16_t>(36 + lane), 0.8f, static_cast<int16_t>(lane));
                e.laneIndex = static_cast<int16_t>(lane);
                buf.push(e);
                ++pushed;
            }
        }
    }

    EXPECT_EQ(pushed, kExpected);
    EXPECT_LE(static_cast<size_t>(kExpected), MidiCaptureBuffer::kCapacity);
    // No eviction: every pushed event is still resident in the ring.
    EXPECT_EQ(buf.count(), static_cast<size_t>(kExpected));

    // extractLastBars(32) over the whole pattern must return all 4096 events.
    std::vector<NoteEvent> out(MidiCaptureBuffer::kCapacity);
    size_t n = buf.extractLastBars(kBars, out.data(), out.size());
    EXPECT_EQ(n, static_cast<size_t>(kExpected));
}

// Backs the chapter-16 claim that at typical groove densities (four to six
// active lanes) the buffer holds far more history than any capture length will
// ever request. "Any capture length" is bounded: the Capture Length parameter
// (plugin/source/controller_base.cpp, kCaptureLength) is a 1-32 bar stepped
// control, so 32 bars is the largest window that can be asked for.
//
// The claim is proven at the pessimistic end of "typical": every lane firing on
// every sixteenth, which is the densest a lane can be (subdivision clamps to
// 16) and denser than the "moderate Density" the prose describes. If the
// headroom holds there, it holds for every real patch at that lane count.
TEST(MidiCaptureBuffer, TypicalDensityRetainsBeyondMaxCaptureLength) {
    constexpr int kMaxCaptureBars = 32; // kCaptureLength parameter ceiling
    constexpr int kStepsPerBar = 16;    // sixteenths = max subdivision
    constexpr double kStepPpq = MidiCaptureBuffer::kBeatsPerBar / kStepsPerBar;

    // "Four to six active lanes" — the typical range the chapter names.
    for (int lanes = 4; lanes <= 6; ++lanes) {
        MidiCaptureBuffer buf;

        const int perBar = lanes * kStepsPerBar;
        const int expected = kMaxCaptureBars * perBar;

        for (int bar = 0; bar < kMaxCaptureBars; ++bar) {
            for (int step = 0; step < kStepsPerBar; ++step) {
                double ppq = static_cast<double>(bar * kStepsPerBar + step) * kStepPpq;
                for (int lane = 0; lane < lanes; ++lane) {
                    NoteEvent e = makeEvent(ppq, static_cast<int16_t>(36 + lane), 0.8f, static_cast<int16_t>(lane));
                    e.laneIndex = static_cast<int16_t>(lane);
                    buf.push(e);
                }
            }
        }

        // Nothing was evicted: the longest requestable window fits outright.
        EXPECT_EQ(buf.count(), static_cast<size_t>(expected)) << "lanes=" << lanes;

        std::vector<NoteEvent> out(MidiCaptureBuffer::kCapacity);
        size_t n = buf.extractLastBars(kMaxCaptureBars, out.data(), out.size());
        EXPECT_EQ(n, static_cast<size_t>(expected)) << "lanes=" << lanes;

        // "Far more history than any capture length will ever request": the
        // ring's own capacity spans several times the 32-bar ceiling, so the
        // margin is headroom rather than an exact fit.
        const int barsHeld = static_cast<int>(MidiCaptureBuffer::kCapacity) / perBar;
        EXPECT_GE(barsHeld, 2 * kMaxCaptureBars) << "lanes=" << lanes << " barsHeld=" << barsHeld;
    }
}

TEST(MidiCaptureBuffer, ExtractSortedOutput) {
    MidiCaptureBuffer buf;
    buf.push(makeEvent(2.0, 38, 0.6f, 1));
    buf.push(makeEvent(1.0, 36, 0.8f, 0));
    buf.push(makeEvent(3.0, 42, 0.9f, 2));

    NoteEvent out[10];
    size_t n = buf.extract(0.0, 10.0, out, 10);
    EXPECT_EQ(n, 3u);
    EXPECT_DOUBLE_EQ(out[0].ppqPosition, 1.0);
    EXPECT_DOUBLE_EQ(out[1].ppqPosition, 2.0);
    EXPECT_DOUBLE_EQ(out[2].ppqPosition, 3.0);
}
