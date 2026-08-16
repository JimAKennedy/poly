// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "poly/lane_name.h"
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

TEST(VLQ, FourBytes) {
    uint8_t out[4];
    // Smallest 4-byte value: 2^21 = 0x200000.
    EXPECT_EQ(writeVLQ(0x200000, out), 4u);
    EXPECT_EQ(out[0], 0x81);
    EXPECT_EQ(out[1], 0x80);
    EXPECT_EQ(out[2], 0x80);
    EXPECT_EQ(out[3], 0x00);

    // Largest representable 4-byte VLQ: 2^28 - 1 = 0x0FFFFFFF -> FF FF FF 7F.
    EXPECT_EQ(writeVLQ(0x0FFFFFFF, out), 4u);
    EXPECT_EQ(out[0], 0xFF);
    EXPECT_EQ(out[1], 0xFF);
    EXPECT_EQ(out[2], 0xFF);
    EXPECT_EQ(out[3], 0x7F);
}

TEST(VLQ, ClampAtCeiling) {
    // Values >= 2^28 cannot fit a 4-byte VLQ; writeVLQ clamps to 0x0FFFFFFF
    // rather than overrunning its temp[4] buffer. The encoding must match the
    // ceiling exactly (FF FF FF 7F) and never exceed 4 bytes.
    uint8_t out[4];
    const uint32_t expected[4] = {0xFF, 0xFF, 0xFF, 0x7F};

    // Exactly 2^28 — the first value that would need a 5th byte unclamped.
    EXPECT_EQ(writeVLQ(0x10000000u, out), 4u);
    for (int i = 0; i < 4; ++i)
        EXPECT_EQ(out[i], expected[i]);

    // Well above the ceiling clamps identically.
    EXPECT_EQ(writeVLQ(0x12345678u, out), 4u);
    for (int i = 0; i < 4; ++i)
        EXPECT_EQ(out[i], expected[i]);

    // UINT32_MAX is the worst case; still clamps to the ceiling in 4 bytes.
    EXPECT_EQ(writeVLQ(0xFFFFFFFFu, out), 4u);
    for (int i = 0; i < 4; ++i)
        EXPECT_EQ(out[i], expected[i]);
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

// --- Format-1 writeMultiTrackSMF (T02) ---
//
// A Format-1 file is a sequence of MTrk chunks after the MThd header. These
// helpers walk that structure so the tests can make concrete byte-offset
// assertions: wrong ntrks, missing track-name meta, or tempo meta landing in
// the wrong track all surface as specific failures.
namespace {

struct MTrkChunk {
    size_t bodyStart; // offset of the first byte after the MTrk length field
    uint32_t len;     // declared body length
};

// Split a Format-1 blob into its MTrk chunks. Asserts each chunk carries the
// 'MTrk' magic so a malformed layout fails loudly rather than silently.
std::vector<MTrkChunk> parseTracks(const std::vector<uint8_t>& d) {
    std::vector<MTrkChunk> out;
    size_t pos = 14; // past MThd (8 header + 6 body)
    while (pos + 8 <= d.size()) {
        EXPECT_EQ(d[pos + 0], 'M');
        EXPECT_EQ(d[pos + 1], 'T');
        EXPECT_EQ(d[pos + 2], 'r');
        EXPECT_EQ(d[pos + 3], 'k');
        uint32_t len = readBE32(&d[pos + 4]);
        out.push_back({pos + 8, len});
        pos += 8 + len;
    }
    return out;
}

// Walk a single MTrk body event-by-event (delta VLQ + event), returning true if
// a meta event of metaType appears. The writer emits an explicit status for
// every message (no running status) and no sysex, so a plain walker suffices.
// This respects event boundaries, so it never false-matches meta bytes that
// happen to occur inside note or delta data.
bool trackHasMeta(const std::vector<uint8_t>& d, const MTrkChunk& c, uint8_t metaType) {
    size_t p = c.bodyStart;
    const size_t end = c.bodyStart + c.len;
    while (p < end) {
        while (p < end && (d[p] & 0x80))
            ++p; // skip delta VLQ continuation bytes
        ++p;     // final delta byte
        if (p >= end)
            break;
        uint8_t status = d[p++];
        if (status == 0xFF) {
            uint8_t type = d[p++];
            uint32_t l = 0;
            while (p < end) {
                uint8_t b = d[p++];
                l = (l << 7) | (b & 0x7F);
                if (!(b & 0x80))
                    break;
            }
            if (type == metaType)
                return true;
            p += l;
        } else {
            uint8_t hi = status & 0xF0;
            p += (hi == 0xC0 || hi == 0xD0) ? 1u : 2u;
        }
    }
    return false;
}

// Count note-on (0x9n, non-zero velocity would matter but we count status only)
// events in a track body, respecting event framing.
int trackNoteOnCount(const std::vector<uint8_t>& d, const MTrkChunk& c) {
    int n = 0;
    size_t p = c.bodyStart;
    const size_t end = c.bodyStart + c.len;
    while (p < end) {
        while (p < end && (d[p] & 0x80))
            ++p;
        ++p;
        if (p >= end)
            break;
        uint8_t status = d[p++];
        if (status == 0xFF) {
            ++p; // meta type
            uint32_t l = 0;
            while (p < end) {
                uint8_t b = d[p++];
                l = (l << 7) | (b & 0x7F);
                if (!(b & 0x80))
                    break;
            }
            p += l;
        } else {
            uint8_t hi = status & 0xF0;
            if (hi == 0x90)
                ++n;
            p += (hi == 0xC0 || hi == 0xD0) ? 1u : 2u;
        }
    }
    return n;
}

// Read the FF 03 track-name string. Our writer always places it first (delta 0),
// so assert that framing directly and decode the VLQ-lengthed payload.
std::string trackName(const std::vector<uint8_t>& d, const MTrkChunk& c) {
    size_t p = c.bodyStart;
    const size_t end = c.bodyStart + c.len;
    if (p + 3 > end)
        return {};
    EXPECT_EQ(d[p + 0], 0x00); // delta 0
    EXPECT_EQ(d[p + 1], 0xFF);
    EXPECT_EQ(d[p + 2], 0x03);
    p += 3;
    uint32_t l = 0;
    while (p < end) {
        uint8_t b = d[p++];
        l = (l << 7) | (b & 0x7F);
        if (!(b & 0x80))
            break;
    }
    std::string s;
    for (uint32_t i = 0; i < l && p < end; ++i)
        s.push_back(static_cast<char>(d[p++]));
    return s;
}

} // namespace

TEST(SmfWriterMultiTrack, HeaderIsFormat1WithConductorPlusLanes) {
    // Two distinct laneIndex values -> conductor + 2 lane tracks = ntrks 3.
    NoteEvent events[] = {
        {0.0, 36, 0.8f, 0.25, 9, 0},
        {0.5, 38, 0.6f, 0.25, 9, 1},
        {1.0, 36, 0.9f, 0.25, 9, 0},
    };
    auto data = writeMultiTrackSMF(events, 3, 120.0);

    ASSERT_GE(data.size(), 14u);
    EXPECT_EQ(data[0], 'M');
    EXPECT_EQ(data[1], 'T');
    EXPECT_EQ(data[2], 'h');
    EXPECT_EQ(data[3], 'd');
    EXPECT_EQ(readBE32(&data[4]), 6u);
    EXPECT_EQ(readBE16(&data[8]), 1u);  // format 1
    EXPECT_EQ(readBE16(&data[10]), 3u); // conductor + 2 lanes
    EXPECT_EQ(readBE16(&data[12]), static_cast<uint16_t>(kSmfTicksPerQuarter));

    auto tracks = parseTracks(data);
    ASSERT_EQ(tracks.size(), 3u);
}

TEST(SmfWriterMultiTrack, ConductorTrackHoldsTempoAndNoNotes) {
    NoteEvent events[] = {
        {0.0, 36, 0.8f, 0.25, 9, 0},
        {0.5, 38, 0.6f, 0.25, 9, 1},
    };
    auto data = writeMultiTrackSMF(events, 2, 120.0);
    auto tracks = parseTracks(data);
    ASSERT_EQ(tracks.size(), 3u);

    // Track 0: tempo meta present, zero notes.
    EXPECT_TRUE(trackHasMeta(data, tracks[0], 0x51));
    EXPECT_EQ(trackNoteOnCount(data, tracks[0]), 0);

    // Concrete byte-offset assertion: conductor body starts with delta 0 + tempo.
    const size_t b = tracks[0].bodyStart;
    EXPECT_EQ(data[b + 0], 0x00);
    EXPECT_EQ(data[b + 1], 0xFF);
    EXPECT_EQ(data[b + 2], 0x51);
    EXPECT_EQ(data[b + 3], 0x03);
    const uint32_t usPerQuarter = (static_cast<uint32_t>(data[b + 4]) << 16) |
                                  (static_cast<uint32_t>(data[b + 5]) << 8) | static_cast<uint32_t>(data[b + 6]);
    EXPECT_EQ(usPerQuarter, 500000u);
}

TEST(SmfWriterMultiTrack, TempoMetaOnlyInConductorTrack) {
    NoteEvent events[] = {
        {0.0, 36, 0.8f, 0.25, 9, 0},
        {0.5, 38, 0.6f, 0.25, 9, 1},
    };
    auto data = writeMultiTrackSMF(events, 2, 120.0);
    auto tracks = parseTracks(data);
    ASSERT_EQ(tracks.size(), 3u);

    EXPECT_TRUE(trackHasMeta(data, tracks[0], 0x51));
    EXPECT_FALSE(trackHasMeta(data, tracks[1], 0x51));
    EXPECT_FALSE(trackHasMeta(data, tracks[2], 0x51));
}

TEST(SmfWriterMultiTrack, LaneTracksAreNamedAndCarryTheirOwnNotes) {
    NoteEvent events[] = {
        {0.0, 36, 0.8f, 0.25, 9, 0}, // Kick, lane 0
        {1.0, 36, 0.8f, 0.25, 9, 0}, // Kick, lane 0
        {0.5, 38, 0.6f, 0.25, 9, 1}, // Snare, lane 1
    };
    auto nameForLane = [&](int lane) -> std::string { return laneName(events[lane == 0 ? 0 : 2].pitch); };
    auto data = writeMultiTrackSMF(events, 3, 120.0, 0.0, nameForLane);
    auto tracks = parseTracks(data);
    ASSERT_EQ(tracks.size(), 3u);

    // Lane tracks carry an FF 03 name meta.
    EXPECT_TRUE(trackHasMeta(data, tracks[1], 0x03));
    EXPECT_TRUE(trackHasMeta(data, tracks[2], 0x03));
    EXPECT_EQ(trackName(data, tracks[1]), "Kick");
    EXPECT_EQ(trackName(data, tracks[2]), "Snare");

    // Notes are partitioned by lane: 2 note-ons in Kick, 1 in Snare.
    EXPECT_EQ(trackNoteOnCount(data, tracks[1]), 2);
    EXPECT_EQ(trackNoteOnCount(data, tracks[2]), 1);
}

TEST(SmfWriterMultiTrack, FallsBackToLaneNWhenNoNamerProvided) {
    NoteEvent events[] = {
        {0.0, 36, 0.8f, 0.25, 9, 0},
        {0.5, 38, 0.6f, 0.25, 9, 2}, // sparse laneIndex
    };
    auto data = writeMultiTrackSMF(events, 2, 120.0); // no namer
    auto tracks = parseTracks(data);
    ASSERT_EQ(tracks.size(), 3u);

    EXPECT_EQ(trackName(data, tracks[1]), "Lane 1"); // laneIndex 0 -> "Lane 1"
    EXPECT_EQ(trackName(data, tracks[2]), "Lane 3"); // laneIndex 2 -> "Lane 3"
}

TEST(SmfWriterMultiTrack, EmptyInputYieldsConductorOnlyWithValidTempo) {
    auto data = writeMultiTrackSMF(nullptr, 0, 120.0);

    ASSERT_GE(data.size(), 14u);
    EXPECT_EQ(readBE16(&data[8]), 1u);  // format 1
    EXPECT_EQ(readBE16(&data[10]), 1u); // conductor only

    auto tracks = parseTracks(data);
    ASSERT_EQ(tracks.size(), 1u);
    EXPECT_TRUE(trackHasMeta(data, tracks[0], 0x51));
    EXPECT_TRUE(trackHasMeta(data, tracks[0], 0x2F)); // end-of-track
}

TEST(SmfWriterMultiTrack, TempoClampAppliesToConductor) {
    NoteEvent ev = {0.0, 36, 0.8f, 0.25, 9, 0};
    auto data = writeMultiTrackSMF(&ev, 1, 0.0); // pathological tempo
    auto tracks = parseTracks(data);
    ASSERT_GE(tracks.size(), 1u);

    const size_t b = tracks[0].bodyStart;
    ASSERT_GE(data.size(), b + 7u);
    const uint32_t usPerQuarter = (static_cast<uint32_t>(data[b + 4]) << 16) |
                                  (static_cast<uint32_t>(data[b + 5]) << 8) | static_cast<uint32_t>(data[b + 6]);
    EXPECT_EQ(usPerQuarter, static_cast<uint32_t>(60000000.0 / kSmfMinTempo));
}

TEST(SmfWriterMultiTrack, EveryTrackEndsWithEndOfTrack) {
    NoteEvent events[] = {
        {0.0, 36, 0.8f, 0.25, 9, 0},
        {0.5, 38, 0.6f, 0.25, 9, 1},
    };
    auto data = writeMultiTrackSMF(events, 2, 120.0);
    auto tracks = parseTracks(data);
    ASSERT_EQ(tracks.size(), 3u);
    for (const auto& t : tracks) {
        // Last three bytes of each track body are FF 2F 00.
        const size_t e = t.bodyStart + t.len;
        ASSERT_GE(e, t.bodyStart + 3u);
        EXPECT_EQ(data[e - 3], 0xFF);
        EXPECT_EQ(data[e - 2], 0x2F);
        EXPECT_EQ(data[e - 1], 0x00);
        EXPECT_TRUE(trackHasMeta(data, t, 0x2F));
    }
}

TEST(SmfWriterMultiTrack, NoteOffBeforeNoteOnAtSameTickWithinLane) {
    // Two same-pitch hits on one lane; the second's note-on shares a tick with
    // the first's note-off. Ordering must place the off first.
    NoteEvent events[] = {
        {0.0, 60, 0.8f, 0.5, 0, 0},
        {0.5, 60, 0.8f, 0.5, 0, 0},
    };
    auto data = writeMultiTrackSMF(events, 2, 120.0);
    auto tracks = parseTracks(data);
    ASSERT_EQ(tracks.size(), 2u); // conductor + 1 lane

    // Walk the lane track for the off-then-on-at-delta-0 pattern.
    const MTrkChunk& lane = tracks[1];
    bool noteOffFirst = false;
    for (size_t i = lane.bodyStart; i + 5 < lane.bodyStart + lane.len; ++i) {
        if ((data[i] & 0xF0) == 0x80 && data[i + 1] == 60) {
            if (data[i + 3] == 0x00 && (data[i + 4] & 0xF0) == 0x90 && data[i + 5] == 60)
                noteOffFirst = true;
            break;
        }
    }
    EXPECT_TRUE(noteOffFirst);
}

TEST(SmfWriterMultiTrack, PpqOffsetTrimsLeadingSilence) {
    NoteEvent ev = {8.0, 36, 0.8f, 0.5, 9, 0};
    auto withOffset = writeMultiTrackSMF(&ev, 1, 120.0, 8.0);

    NoteEvent evAtZero = {0.0, 36, 0.8f, 0.5, 9, 0};
    auto fromZero = writeMultiTrackSMF(&evAtZero, 1, 120.0, 0.0);

    EXPECT_EQ(withOffset, fromZero);
}
