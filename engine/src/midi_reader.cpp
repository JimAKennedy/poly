// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#include "poly/midi_reader.h"

#include <algorithm>

namespace poly {

namespace {

// A bounds-checked forward cursor over the SMF byte span. Every read validates
// against `end` first, so a truncated or hostile file can never provoke an
// out-of-bounds read — the reader just reports failure via `ok`. This is what
// backs parseSMF's no-throw / valid-flag contract (Decision D037).
struct Cursor {
    const uint8_t* p;
    const uint8_t* end;

    size_t remaining() const { return static_cast<size_t>(end - p); }

    bool has(size_t n) const { return remaining() >= n; }

    uint8_t u8() { return *p++; }

    uint16_t be16() {
        uint16_t v = static_cast<uint16_t>((static_cast<uint16_t>(p[0]) << 8) | p[1]);
        p += 2;
        return v;
    }

    uint32_t be32() {
        uint32_t v = (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
                     (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
        p += 4;
        return v;
    }
};

// Decode a variable-length quantity, advancing `c`. Sets ok=false (and stops) if
// the span ends mid-VLQ or the value would need more than 4 bytes (malformed).
uint32_t readVLQ(Cursor& c, const uint8_t* trackEnd, bool& ok) {
    uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
        if (c.p >= trackEnd) {
            ok = false;
            return 0;
        }
        uint8_t b = c.u8();
        value = (value << 7) | (b & 0x7Fu);
        if (!(b & 0x80u))
            return value;
    }
    // A fifth continuation byte means a malformed / >28-bit VLQ.
    ok = false;
    return 0;
}

// Number of data bytes carried by a channel voice message with the given status
// high nibble. Program change (0xC0) and channel pressure (0xD0) carry one; all
// other channel messages carry two.
int channelDataBytes(uint8_t statusHi) {
    return (statusHi == 0xC0 || statusHi == 0xD0) ? 1 : 2;
}

} // namespace

MidiParseResult parseSMF(const uint8_t* data, size_t size) {
    MidiParseResult result;
    if (data == nullptr)
        return result;

    Cursor c{data, data + size};

    // --- MThd header: 'MThd' <len:be32> <format:be16> <ntrks:be16> <div:be16>
    if (!c.has(14))
        return result;
    if (c.p[0] != 'M' || c.p[1] != 'T' || c.p[2] != 'h' || c.p[3] != 'd')
        return result;
    c.p += 4;
    uint32_t headerLen = c.be32();
    // Standard header body is 6 bytes; tolerate a larger declared length by
    // skipping the extra bytes (some files pad), but the 6 fields we need must
    // be present.
    if (headerLen < 6 || !c.has(headerLen))
        return result;
    const uint8_t* headerBodyStart = c.p;
    /*format*/ c.be16();
    /*ntrks*/ c.be16();
    uint16_t division = c.be16();
    c.p = headerBodyStart + headerLen; // skip any padding past the 6 known bytes

    // SMPTE division (high bit set) encodes frames/sub-frames, not ticks per
    // quarter — unsupported by the reverse-Euclid import path. A zero division
    // is degenerate. Either way there is no metrical grid to fit against.
    if ((division & 0x8000u) != 0 || division == 0)
        return result;
    const double ticksPerQuarter = static_cast<double>(division);

    std::vector<double> onsetTicks;
    double maxTick = 0.0;
    bool tempoFound = false;
    double tempoBpm = 120.0;
    bool sawTrack = false;

    // --- Walk every MTrk chunk. Tracks are independent tick timelines that all
    // start at 0 (both format 0 and 1); we merge their note-ons into one stream.
    while (c.has(8)) {
        bool isTrack = (c.p[0] == 'M' && c.p[1] == 'T' && c.p[2] == 'r' && c.p[3] == 'k');
        c.p += 4;
        uint32_t chunkLen = c.be32();
        if (!c.has(chunkLen)) {
            // Truncated final chunk: clamp so we never read past `end`, then
            // parse what is present.
            chunkLen = static_cast<uint32_t>(c.remaining());
        }
        const uint8_t* trackStart = c.p;
        const uint8_t* trackEnd = c.p + chunkLen;

        // Unknown chunk type (not 'MTrk') — skip its body per the SMF spec.
        if (!isTrack) {
            c.p = trackEnd;
            continue;
        }
        sawTrack = true;

        uint32_t absTick = 0;
        uint8_t runningStatus = 0;
        bool trackOk = true;

        while (c.p < trackEnd) {
            uint32_t delta = readVLQ(c, trackEnd, trackOk);
            if (!trackOk)
                break;
            absTick += delta;

            if (c.p >= trackEnd) {
                trackOk = false;
                break;
            }
            uint8_t byte = *c.p;
            uint8_t status;
            if (byte & 0x80u) {
                status = byte;
                c.p++;
                // Running status is armed by channel voice messages only; it is
                // cleared by system/meta events below.
                if (status < 0xF0)
                    runningStatus = status;
            } else {
                // Running status: reuse the previous channel status; `byte` is
                // the first data byte.
                if (runningStatus == 0) {
                    trackOk = false;
                    break;
                }
                status = runningStatus;
            }

            if (status == 0xFF) {
                // Meta event: <type:u8> <len:vlq> <bytes>
                runningStatus = 0;
                if (c.p >= trackEnd) {
                    trackOk = false;
                    break;
                }
                uint8_t metaType = c.u8();
                uint32_t metaLen = readVLQ(c, trackEnd, trackOk);
                if (!trackOk)
                    break;
                if (static_cast<size_t>(trackEnd - c.p) < metaLen) {
                    trackOk = false;
                    break;
                }
                if (metaType == 0x51 && metaLen == 3 && !tempoFound) {
                    uint32_t usPerQuarter = (static_cast<uint32_t>(c.p[0]) << 16) |
                                            (static_cast<uint32_t>(c.p[1]) << 8) | static_cast<uint32_t>(c.p[2]);
                    if (usPerQuarter > 0) {
                        tempoBpm = 60000000.0 / static_cast<double>(usPerQuarter);
                        tempoFound = true;
                    }
                }
                c.p += metaLen;
            } else if (status == 0xF0 || status == 0xF7) {
                // Sysex (and escaped sysex): <len:vlq> <bytes>. Clears running
                // status.
                runningStatus = 0;
                uint32_t sysexLen = readVLQ(c, trackEnd, trackOk);
                if (!trackOk)
                    break;
                if (static_cast<size_t>(trackEnd - c.p) < sysexLen) {
                    trackOk = false;
                    break;
                }
                c.p += sysexLen;
            } else {
                // Channel voice message.
                uint8_t hi = status & 0xF0u;
                int dataBytes = channelDataBytes(hi);

                // The first data byte was `byte` when running status kicked in;
                // otherwise it still sits at c.p. Read the needed data bytes
                // bounds-checked either way.
                uint8_t d1 = 0;
                uint8_t d2 = 0;
                if ((byte & 0x80u) == 0) {
                    // Running status already consumed the status view; `byte` is
                    // d1 and is still at c.p.
                    if (c.p >= trackEnd) {
                        trackOk = false;
                        break;
                    }
                    d1 = c.u8();
                    if (dataBytes == 2) {
                        if (c.p >= trackEnd) {
                            trackOk = false;
                            break;
                        }
                        d2 = c.u8();
                    }
                } else {
                    if (static_cast<size_t>(trackEnd - c.p) < static_cast<size_t>(dataBytes)) {
                        trackOk = false;
                        break;
                    }
                    d1 = c.u8();
                    if (dataBytes == 2)
                        d2 = c.u8();
                }

                // Note-on with non-zero velocity is a real onset. Velocity 0 is a
                // note-off per convention and carries no onset.
                if (hi == 0x90 && d2 > 0)
                    onsetTicks.push_back(static_cast<double>(absTick));
            }
        }

        if (static_cast<double>(absTick) > maxTick)
            maxTick = static_cast<double>(absTick);

        // Advance to the declared chunk boundary regardless of where event
        // parsing stopped, so a malformed track cannot desync chunk framing.
        c.p = trackEnd;
        (void)trackStart;
    }

    if (!sawTrack)
        return result;

    std::sort(onsetTicks.begin(), onsetTicks.end());

    result.valid = true;
    result.onsetsPpq.reserve(onsetTicks.size());
    for (double t : onsetTicks)
        result.onsetsPpq.push_back(t / ticksPerQuarter);
    result.loopLengthPpq = maxTick / ticksPerQuarter;
    result.tempoBpm = tempoBpm;
    return result;
}

MidiParseResult parseSMF(const std::vector<uint8_t>& data) {
    return parseSMF(data.data(), data.size());
}

// --- Shared parse -> fit -> apply helper (M035 S02 T02) --------------------

void applyFitToLane(const FitResult& fit, LaneConfig& lane) {
    if (!fit.valid)
        return;

    lane.cycle.steps = fit.steps;
    lane.cycle.subdivision = fit.subdivision;
    lane.hitCount = fit.hitCount;
    lane.rotation = fit.rotation;

    // Residual capture: when the maximally-even Euclidean skeleton cannot
    // reproduce the source occupancy, drive the lane from the fitted grid's
    // actual per-step pattern (timeline mode) so extra/missing hits survive.
    // A clean Euclidean fit clears timeline mode so the engine generates the
    // pattern from hitCount/steps/rotation as usual.
    if (fit.patternMismatch > 0 && fit.fixedPatternLength > 0) {
        lane.timeline = true;
        lane.fixedPattern = fit.fixedPattern;
        lane.fixedPatternLength = fit.fixedPatternLength;
    } else {
        lane.timeline = false;
        lane.fixedPatternLength = 0;
    }

    // Micro-timing residual is zero for on-grid rhythms, so this is safe to copy
    // unconditionally — it only carries nuance the grid could not express.
    lane.microTimingMs = fit.microTimingMs;
}

bool importMidiToLane(const uint8_t* data, size_t size, LaneConfig& lane) {
    const MidiParseResult parsed = parseSMF(data, size);
    if (!parsed.valid)
        return false;
    const FitResult fit = fitEuclidean(parsed.onsetsPpq, parsed.loopLengthPpq, parsed.tempoBpm);
    if (!fit.valid)
        return false;
    applyFitToLane(fit, lane);
    return true;
}

bool importMidiToLane(const std::vector<uint8_t>& data, LaneConfig& lane) {
    return importMidiToLane(data.data(), data.size(), lane);
}

// --- Atomic per-lane import snapshot / revert (M035 S03 T01) ----------------

LaneSnapshot captureLaneSnapshot(const LaneConfig& lane) {
    LaneSnapshot snap;
    snap.config = lane;
    snap.valid = true;
    return snap;
}

bool revertLaneFromSnapshot(LaneConfig& lane, LaneSnapshot& snap) {
    if (!snap.valid)
        return false;
    lane = snap.config;
    snap.valid = false;
    return true;
}

} // namespace poly
