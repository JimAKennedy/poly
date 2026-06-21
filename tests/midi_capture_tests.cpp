#include <gtest/gtest.h>

#include "poly/midi_capture.h"

using namespace poly;

static NoteEvent makeEvent(double ppq, int16_t pitch, float vel, int16_t ch) {
    return {ppq, pitch, vel, 0.25, ch};
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
