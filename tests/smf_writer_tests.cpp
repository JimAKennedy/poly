#include <cmath>
#include <cstdint>

#include <gtest/gtest.h>

#include "poly/smf_writer.h"

using namespace poly;

static uint16_t readBE16(const uint8_t* p) {
    return static_cast<uint16_t>((p[0] << 8) | p[1]);
}

static uint32_t readBE32(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
}

TEST(VLQ, SingleByte) {
    uint8_t out[4];
    EXPECT_EQ(writeVLQ(0, out), 1u);
    EXPECT_EQ(out[0], 0x00);

    EXPECT_EQ(writeVLQ(0x7F, out), 1u);
    EXPECT_EQ(out[0], 0x7F);
}

TEST(VLQ, TwoBytes) {
    uint8_t out[4];
    EXPECT_EQ(writeVLQ(0x80, out), 2u);
    EXPECT_EQ(out[0], 0x81);
    EXPECT_EQ(out[1], 0x00);

    EXPECT_EQ(writeVLQ(0x3FFF, out), 2u);
    EXPECT_EQ(out[0], 0xFF);
    EXPECT_EQ(out[1], 0x7F);
}

TEST(VLQ, ThreeBytes) {
    uint8_t out[4];
    EXPECT_EQ(writeVLQ(0x4000, out), 3u);
    EXPECT_EQ(out[0], 0x81);
    EXPECT_EQ(out[1], 0x80);
    EXPECT_EQ(out[2], 0x00);
}

TEST(SmfWriter, HeaderChunk) {
    NoteEvent ev = {0.0, 60, 0.8f, 0.5, 0};
    auto data = writeSMF(&ev, 1, 120.0);

    ASSERT_GE(data.size(), 14u);
    EXPECT_EQ(data[0], 'M');
    EXPECT_EQ(data[1], 'T');
    EXPECT_EQ(data[2], 'h');
    EXPECT_EQ(data[3], 'd');
    EXPECT_EQ(readBE32(&data[4]), 6u);
    EXPECT_EQ(readBE16(&data[8]), 0u);
    EXPECT_EQ(readBE16(&data[10]), 1u);
    EXPECT_EQ(readBE16(&data[12]), static_cast<uint16_t>(kSmfTicksPerQuarter));
}

TEST(SmfWriter, TrackChunk) {
    NoteEvent ev = {0.0, 60, 0.8f, 0.5, 0};
    auto data = writeSMF(&ev, 1, 120.0);

    ASSERT_GE(data.size(), 22u);
    EXPECT_EQ(data[14], 'M');
    EXPECT_EQ(data[15], 'T');
    EXPECT_EQ(data[16], 'r');
    EXPECT_EQ(data[17], 'k');

    uint32_t trackLen = readBE32(&data[18]);
    EXPECT_EQ(data.size(), 22u + trackLen);
}

TEST(SmfWriter, TempoMetaEvent) {
    NoteEvent ev = {0.0, 60, 0.8f, 0.5, 0};
    auto data = writeSMF(&ev, 1, 120.0);

    ASSERT_GE(data.size(), 29u);
    EXPECT_EQ(data[22], 0x00);
    EXPECT_EQ(data[23], 0xFF);
    EXPECT_EQ(data[24], 0x51);
    EXPECT_EQ(data[25], 0x03);

    uint32_t usPerQuarter = (static_cast<uint32_t>(data[26]) << 16) | (static_cast<uint32_t>(data[27]) << 8) |
                            static_cast<uint32_t>(data[28]);
    EXPECT_EQ(usPerQuarter, 500000u);
}

TEST(SmfWriter, NoteEvents) {
    NoteEvent ev = {0.0, 60, 1.0f, 0.5, 0};
    auto data = writeSMF(&ev, 1, 120.0);

    bool foundNoteOn = false;
    bool foundNoteOff = false;
    for (size_t i = 22; i + 2 < data.size(); ++i) {
        if (data[i] == 0x90 && data[i + 1] == 60) {
            foundNoteOn = true;
            EXPECT_EQ(data[i + 2], 127);
        }
        if (data[i] == 0x80 && data[i + 1] == 60) {
            foundNoteOff = true;
        }
    }
    EXPECT_TRUE(foundNoteOn);
    EXPECT_TRUE(foundNoteOff);
}

TEST(SmfWriter, EndOfTrack) {
    NoteEvent ev = {0.0, 60, 0.8f, 0.5, 0};
    auto data = writeSMF(&ev, 1, 120.0);

    size_t n = data.size();
    ASSERT_GE(n, 3u);
    EXPECT_EQ(data[n - 3], 0xFF);
    EXPECT_EQ(data[n - 2], 0x2F);
    EXPECT_EQ(data[n - 1], 0x00);
}

TEST(SmfWriter, PpqOffset) {
    NoteEvent ev = {8.0, 60, 0.8f, 0.5, 0};
    auto withOffset = writeSMF(&ev, 1, 120.0, 8.0);
    auto withoutOffset = writeSMF(&ev, 1, 120.0, 0.0);

    EXPECT_NE(withOffset, withoutOffset);

    NoteEvent evAtZero = {0.0, 60, 0.8f, 0.5, 0};
    auto fromZero = writeSMF(&evAtZero, 1, 120.0, 0.0);
    EXPECT_EQ(withOffset, fromZero);
}

TEST(SmfWriter, MultipleNotes) {
    NoteEvent events[] = {
        {0.0, 36, 0.8f, 0.25, 9},
        {0.5, 38, 0.6f, 0.25, 9},
        {1.0, 42, 0.9f, 0.25, 9},
    };
    auto data = writeSMF(events, 3, 120.0);

    int noteOnCount = 0;
    int noteOffCount = 0;
    for (size_t i = 22; i + 2 < data.size(); ++i) {
        uint8_t status = data[i] & 0xF0;
        if (status == 0x90)
            ++noteOnCount;
        if (status == 0x80)
            ++noteOffCount;
    }
    EXPECT_EQ(noteOnCount, 3);
    EXPECT_EQ(noteOffCount, 3);
}

TEST(SmfWriter, EmptyInput) {
    auto data = writeSMF(nullptr, 0, 120.0);

    ASSERT_GE(data.size(), 14u);
    EXPECT_EQ(data[0], 'M');

    size_t n = data.size();
    EXPECT_EQ(data[n - 3], 0xFF);
    EXPECT_EQ(data[n - 2], 0x2F);
    EXPECT_EQ(data[n - 1], 0x00);
}

TEST(SmfWriter, NoteOffBeforeNoteOnAtSameTick) {
    NoteEvent events[] = {
        {0.0, 60, 0.8f, 0.5, 0},
        {0.5, 60, 0.8f, 0.5, 0},
    };
    auto data = writeSMF(events, 2, 120.0);

    // Note off = 3 bytes (status, pitch, vel), then delta=0 = 1 byte, then note on status
    bool noteOffFirst = false;
    for (size_t i = 22; i + 5 < data.size(); ++i) {
        if ((data[i] & 0xF0) == 0x80 && data[i + 1] == 60) {
            if (i + 5 < data.size() && data[i + 3] == 0x00 && (data[i + 4] & 0xF0) == 0x90 && data[i + 5] == 60) {
                noteOffFirst = true;
            }
            break;
        }
    }
    EXPECT_TRUE(noteOffFirst);
}

// M049 S03 (E3) regression: writeSMF() previously divided by tempo directly.
// tempo=0 → 60000000/0 = inf → std::round(inf) → uint32_t cast UB. Bridge
// guards ppqToSampleOffset the same way. Fix: clamp to kSmfMinTempo (20 BPM)
// before the division so the export path never emits UB regardless of caller
// pathology (transport paused, uninitialized export state).
namespace {
uint32_t extractTempoMeta(const std::vector<uint8_t>& data) {
    return (static_cast<uint32_t>(data[26]) << 16) | (static_cast<uint32_t>(data[27]) << 8) |
           static_cast<uint32_t>(data[28]);
}
} // namespace

TEST(SmfWriter, TempoZeroClampsToMinimum) {
    NoteEvent ev = {0.0, 60, 0.8f, 0.5, 0};
    auto data = writeSMF(&ev, 1, 0.0);

    ASSERT_GE(data.size(), 29u);
    const uint32_t expected = static_cast<uint32_t>(60000000.0 / kSmfMinTempo);
    EXPECT_EQ(extractTempoMeta(data), expected);

    int noteOnCount = 0;
    for (size_t i = 22; i + 2 < data.size(); ++i)
        if ((data[i] & 0xF0) == 0x90)
            ++noteOnCount;
    EXPECT_EQ(noteOnCount, 1);
}

TEST(SmfWriter, TempoNegativeClampsToMinimum) {
    NoteEvent ev = {0.0, 60, 0.8f, 0.5, 0};
    auto data = writeSMF(&ev, 1, -120.0);

    ASSERT_GE(data.size(), 29u);
    const uint32_t expected = static_cast<uint32_t>(60000000.0 / kSmfMinTempo);
    EXPECT_EQ(extractTempoMeta(data), expected);
}

TEST(SmfWriter, TempoNaNClampsToMinimum) {
    NoteEvent ev = {0.0, 60, 0.8f, 0.5, 0};
    auto data = writeSMF(&ev, 1, std::nan(""));

    ASSERT_GE(data.size(), 29u);
    const uint32_t expected = static_cast<uint32_t>(60000000.0 / kSmfMinTempo);
    EXPECT_EQ(extractTempoMeta(data), expected);
}

TEST(SmfWriter, TempoBelowMinimumClampsToMinimum) {
    // Sub-20-BPM values are technically finite and positive but musically
    // nonsensical and can push usPerQuarter above the 3-byte encoding range.
    NoteEvent ev = {0.0, 60, 0.8f, 0.5, 0};
    auto data = writeSMF(&ev, 1, 5.0);

    ASSERT_GE(data.size(), 29u);
    const uint32_t expected = static_cast<uint32_t>(60000000.0 / kSmfMinTempo);
    EXPECT_EQ(extractTempoMeta(data), expected);
}
