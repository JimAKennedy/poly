#include "poly/smf_writer.h"

#include <algorithm>
#include <cmath>

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

} // namespace

size_t writeVLQ(uint32_t value, uint8_t* out) {
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

    uint32_t usPerQuarter = static_cast<uint32_t>(std::round(60000000.0 / tempo));
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

} // namespace poly
