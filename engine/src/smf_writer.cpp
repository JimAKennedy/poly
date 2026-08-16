// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#include "poly/smf_writer.h"

#include <algorithm>
#include <cmath>
#include <set>

namespace poly {

namespace {

void writeBE16(std::vector<uint8_t>& buf, uint16_t val) {
    buf.push_back(static_cast<uint8_t>(val >> 8));
    buf.push_back(static_cast<uint8_t>(val));
}

void writeBE32(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back(static_cast<uint8_t>(val >> 24));
    buf.push_back(static_cast<uint8_t>(val >> 16));
    buf.push_back(static_cast<uint8_t>(val >> 8));
    buf.push_back(static_cast<uint8_t>(val));
}

void writeVLQToVec(std::vector<uint8_t>& buf, uint32_t value) {
    uint8_t temp[4];
    size_t n = writeVLQ(value, temp);
    for (size_t i = 0; i < n; ++i)
        buf.push_back(temp[i]);
}

// --- Shared Format-1 helpers (writeMultiTrackSMF only; writeSMF is left
// byte-for-byte untouched for the Format-0 parity harness). ---

struct MidiMsg {
    uint32_t tick;
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
};

// Clamp pathological tempo values the same way writeSMF does, then append the
// FF 51 tempo meta at delta 0.
void appendTempoMeta(std::vector<uint8_t>& track, double tempo) {
    const double safeTempo = (std::isfinite(tempo) && tempo >= kSmfMinTempo) ? tempo : kSmfMinTempo;
    uint32_t usPerQuarter = static_cast<uint32_t>(std::round(60000000.0 / safeTempo));
    writeVLQToVec(track, 0);
    track.push_back(0xFF);
    track.push_back(0x51);
    track.push_back(0x03);
    track.push_back(static_cast<uint8_t>(usPerQuarter >> 16));
    track.push_back(static_cast<uint8_t>(usPerQuarter >> 8));
    track.push_back(static_cast<uint8_t>(usPerQuarter));
}

// Append the FF 03 track-name meta at delta 0. The name length is VLQ-encoded
// per the SMF spec (names longer than 127 bytes are unheard-of here but encode
// correctly regardless).
void appendTrackName(std::vector<uint8_t>& track, const std::string& name) {
    writeVLQToVec(track, 0);
    track.push_back(0xFF);
    track.push_back(0x03);
    writeVLQToVec(track, static_cast<uint32_t>(name.size()));
    for (char c : name)
        track.push_back(static_cast<uint8_t>(c));
}

// Append the FF 2F end-of-track meta at delta 0.
void appendEndOfTrack(std::vector<uint8_t>& track) {
    writeVLQToVec(track, 0);
    track.push_back(0xFF);
    track.push_back(0x2F);
    track.push_back(0x00);
}

// Build the sorted note-on/note-off stream for a set of events and append it to
// track as delta-time-encoded channel messages. Mirrors writeSMF's ordering:
// ascending tick, and note-off (0x80) before note-on (0x90) at the same tick.
void appendNoteMessages(std::vector<uint8_t>& track, const NoteEvent* events, const size_t* indices, size_t indexCount,
                        double ppqOffset) {
    std::vector<MidiMsg> msgs;
    msgs.reserve(indexCount * 2);

    for (size_t k = 0; k < indexCount; ++k) {
        const NoteEvent& e = events[indices[k]];
        double ppq = e.ppqPosition - ppqOffset;
        if (ppq < 0.0)
            ppq = 0.0;
        uint32_t onTick = static_cast<uint32_t>(std::round(ppq * kSmfTicksPerQuarter));
        uint8_t vel = static_cast<uint8_t>(std::clamp(static_cast<int>(std::round(e.velocity * 127.0f)), 0, 127));
        uint8_t ch = static_cast<uint8_t>(std::clamp(static_cast<int>(e.channel), 0, 15));
        uint8_t pitch = static_cast<uint8_t>(std::clamp(static_cast<int>(e.pitch), 0, 127));

        msgs.push_back({onTick, static_cast<uint8_t>(0x90 | ch), pitch, vel});

        uint32_t offTick = static_cast<uint32_t>(std::round((ppq + e.duration) * kSmfTicksPerQuarter));
        msgs.push_back({offTick, static_cast<uint8_t>(0x80 | ch), pitch, 0});
    }

    std::sort(msgs.begin(), msgs.end(), [](const MidiMsg& a, const MidiMsg& b) {
        if (a.tick != b.tick)
            return a.tick < b.tick;
        return (a.status & 0xF0) < (b.status & 0xF0);
    });

    uint32_t prevTick = 0;
    for (const auto& msg : msgs) {
        uint32_t delta = msg.tick - prevTick;
        writeVLQToVec(track, delta);
        track.push_back(msg.status);
        track.push_back(msg.data1);
        track.push_back(msg.data2);
        prevTick = msg.tick;
    }
}

// Wrap a completed track body in an MTrk chunk header and append it to data.
void appendTrackChunk(std::vector<uint8_t>& data, const std::vector<uint8_t>& track) {
    data.push_back('M');
    data.push_back('T');
    data.push_back('r');
    data.push_back('k');
    writeBE32(data, static_cast<uint32_t>(track.size()));
    data.insert(data.end(), track.begin(), track.end());
}

} // namespace

size_t writeVLQ(uint32_t value, uint8_t* out) {
    // A 4-byte VLQ encodes at most 28 bits (0x0FFFFFFF). A value >= 2^28 needs a
    // 5th byte, which would push temp[n++] past the fixed temp[4] stack buffer
    // below — a latent overflow. No delta this writer emits reaches that ceiling
    // (256 bars 4/4 = 491520 ticks << 2^28), but clamp defensively so the
    // primitive can never overrun for any future/larger caller. Clamping to the
    // max representable delta is the safe degradation: the JS/TS ports mirror
    // this exact clamp (0x0fffffff) so all three implementations agree.
    constexpr uint32_t kMaxVLQ = 0x0FFFFFFF; // 2^28 - 1
    if (value > kMaxVLQ)
        value = kMaxVLQ;

    if (value < 0x80) {
        out[0] = static_cast<uint8_t>(value);
        return 1;
    }

    uint8_t temp[4];
    size_t n = 0;
    temp[n++] = static_cast<uint8_t>(value & 0x7F);
    value >>= 7;
    while (value > 0) {
        temp[n++] = static_cast<uint8_t>((value & 0x7F) | 0x80);
        value >>= 7;
    }

    for (size_t i = 0; i < n; ++i)
        out[i] = temp[n - 1 - i];

    return n;
}

std::vector<uint8_t> writeSMF(const NoteEvent* events, size_t count, double tempo, double ppqOffset) {
    std::vector<uint8_t> data;
    data.reserve(14 + count * 12 + 32);

    data.push_back('M');
    data.push_back('T');
    data.push_back('h');
    data.push_back('d');
    writeBE32(data, 6);
    writeBE16(data, 0);
    writeBE16(data, 1);
    writeBE16(data, static_cast<uint16_t>(kSmfTicksPerQuarter));

    std::vector<uint8_t> track;
    track.reserve(count * 12 + 32);

    // M049 S03 (E3): clamp pathological tempo values so the usPerQuarter
    // computation never produces inf/NaN (undefined uint32_t cast) or a value
    // that overflows the 3-byte tempo meta encoding. See kSmfMinTempo in the
    // header for the rationale.
    const double safeTempo = (std::isfinite(tempo) && tempo >= kSmfMinTempo) ? tempo : kSmfMinTempo;
    uint32_t usPerQuarter = static_cast<uint32_t>(std::round(60000000.0 / safeTempo));
    writeVLQToVec(track, 0);
    track.push_back(0xFF);
    track.push_back(0x51);
    track.push_back(0x03);
    track.push_back(static_cast<uint8_t>(usPerQuarter >> 16));
    track.push_back(static_cast<uint8_t>(usPerQuarter >> 8));
    track.push_back(static_cast<uint8_t>(usPerQuarter));

    struct MidiMsg {
        uint32_t tick;
        uint8_t status;
        uint8_t data1;
        uint8_t data2;
    };

    std::vector<MidiMsg> msgs;
    msgs.reserve(count * 2);

    for (size_t i = 0; i < count; ++i) {
        double ppq = events[i].ppqPosition - ppqOffset;
        if (ppq < 0.0)
            ppq = 0.0;
        uint32_t onTick = static_cast<uint32_t>(std::round(ppq * kSmfTicksPerQuarter));
        uint8_t vel =
            static_cast<uint8_t>(std::clamp(static_cast<int>(std::round(events[i].velocity * 127.0f)), 0, 127));
        uint8_t ch = static_cast<uint8_t>(std::clamp(static_cast<int>(events[i].channel), 0, 15));
        uint8_t pitch = static_cast<uint8_t>(std::clamp(static_cast<int>(events[i].pitch), 0, 127));

        msgs.push_back({onTick, static_cast<uint8_t>(0x90 | ch), pitch, vel});

        uint32_t offTick = static_cast<uint32_t>(std::round((ppq + events[i].duration) * kSmfTicksPerQuarter));
        msgs.push_back({offTick, static_cast<uint8_t>(0x80 | ch), pitch, 0});
    }

    std::sort(msgs.begin(), msgs.end(), [](const MidiMsg& a, const MidiMsg& b) {
        if (a.tick != b.tick)
            return a.tick < b.tick;
        return (a.status & 0xF0) < (b.status & 0xF0);
    });

    uint32_t prevTick = 0;
    for (const auto& msg : msgs) {
        uint32_t delta = msg.tick - prevTick;
        writeVLQToVec(track, delta);
        track.push_back(msg.status);
        track.push_back(msg.data1);
        track.push_back(msg.data2);
        prevTick = msg.tick;
    }

    writeVLQToVec(track, 0);
    track.push_back(0xFF);
    track.push_back(0x2F);
    track.push_back(0x00);

    data.push_back('M');
    data.push_back('T');
    data.push_back('r');
    data.push_back('k');
    writeBE32(data, static_cast<uint32_t>(track.size()));
    data.insert(data.end(), track.begin(), track.end());

    return data;
}

std::vector<uint8_t> writeMultiTrackSMF(const NoteEvent* events, size_t count, double tempo, double ppqOffset,
                                        const std::function<std::string(int laneIndex)>& nameForLane) {
    // Distinct laneIndex values, ascending, so track order is deterministic and
    // independent of event arrival order. std::set gives sorted-unique for free.
    std::set<int> laneSet;
    for (size_t i = 0; i < count; ++i)
        laneSet.insert(static_cast<int>(events[i].laneIndex));
    std::vector<int> lanes(laneSet.begin(), laneSet.end());

    std::vector<uint8_t> data;
    // Rough reserve: MThd(14) + conductor(~15) + per-lane header/name/EOT + notes.
    data.reserve(14 + 32 + lanes.size() * 48 + count * 8);

    // MThd: format=1, ntrks = conductor + one per active lane.
    data.push_back('M');
    data.push_back('T');
    data.push_back('h');
    data.push_back('d');
    writeBE32(data, 6);
    writeBE16(data, 1);
    writeBE16(data, static_cast<uint16_t>(1 + lanes.size()));
    writeBE16(data, static_cast<uint16_t>(kSmfTicksPerQuarter));

    // Track 0: conductor — tempo meta + end-of-track only (no notes). Keeping
    // tempo in track 0 is the Format-1 convention DAWs expect.
    {
        std::vector<uint8_t> conductor;
        appendTempoMeta(conductor, tempo);
        appendEndOfTrack(conductor);
        appendTrackChunk(data, conductor);
    }

    // One MTrk per lane: FF 03 name, then that lane's notes, then EOT.
    std::vector<size_t> laneIndices;
    for (int lane : lanes) {
        laneIndices.clear();
        for (size_t i = 0; i < count; ++i)
            if (static_cast<int>(events[i].laneIndex) == lane)
                laneIndices.push_back(i);

        std::string name;
        if (nameForLane)
            name = nameForLane(lane);
        if (name.empty())
            name = "Lane " + std::to_string(lane + 1);

        std::vector<uint8_t> track;
        track.reserve(name.size() + laneIndices.size() * 8 + 16);
        appendTrackName(track, name);
        appendNoteMessages(track, events, laneIndices.data(), laneIndices.size(), ppqOffset);
        appendEndOfTrack(track);
        appendTrackChunk(data, track);
    }

    return data;
}

} // namespace poly
