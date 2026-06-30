#pragma once

#include <cstddef>
#include <cstdint>

#include "poly/types.h"

namespace poly {

template <typename WriteFn>
[[nodiscard]] bool writeLaneConfig(WriteFn&& write, const LaneConfig& lane, int32_t bodyVersion) {
    if (!write(&lane.id, sizeof(lane.id)))
        return false;
    auto role = static_cast<uint8_t>(lane.role);
    if (!write(&role, sizeof(role)))
        return false;
    if (!write(&lane.midiNote, sizeof(lane.midiNote)))
        return false;
    if (!write(&lane.cycle.steps, sizeof(lane.cycle.steps)))
        return false;
    if (!write(&lane.cycle.subdivision, sizeof(lane.cycle.subdivision)))
        return false;
    if (!write(&lane.hitCount, sizeof(lane.hitCount)))
        return false;
    if (!write(&lane.rotation, sizeof(lane.rotation)))
        return false;
    if (!write(&lane.probability, sizeof(lane.probability)))
        return false;
    if (!write(&lane.baseVelocity, sizeof(lane.baseVelocity)))
        return false;

    if (bodyVersion >= 12) {
        for (int s = 0; s < kMaxSteps; ++s) {
            if (!write(&lane.accents.steps[static_cast<size_t>(s)], sizeof(float)))
                return false;
        }
    } else {
        uint64_t accentBits = 0;
        for (int s = 0; s < kMaxSteps; ++s) {
            if (lane.accents.steps[static_cast<size_t>(s)] > 0.0f)
                accentBits |= (uint64_t{1} << s);
        }
        if (!write(&accentBits, sizeof(accentBits)))
            return false;
    }

    if (!write(&lane.emphasisProb, sizeof(lane.emphasisProb)))
        return false;
    if (!write(&lane.ghostFloor, sizeof(lane.ghostFloor)))
        return false;
    if (!write(&lane.velocitySpread, sizeof(lane.velocitySpread)))
        return false;
    if (!write(&lane.humanizeMs, sizeof(lane.humanizeMs)))
        return false;
    if (!write(&lane.swingAmount, sizeof(lane.swingAmount)))
        return false;
    if (!write(&lane.noteDuration, sizeof(lane.noteDuration)))
        return false;
    uint8_t active = lane.active ? 1 : 0;
    if (!write(&active, sizeof(active)))
        return false;
    if (!write(&lane.envelopeCount, sizeof(lane.envelopeCount)))
        return false;

    for (int e = 0; e < kMaxEnvelopesPerLane; ++e) {
        const auto& ea = lane.envelopes[static_cast<size_t>(e)];
        if (!writeEnvelope(write, ea.envelope))
            return false;
        uint8_t envActive = ea.active ? 1 : 0;
        if (!write(&envActive, sizeof(envActive)))
            return false;
    }

    if (bodyVersion >= 4) {
        uint64_t anchorBits = 0;
        for (int s = 0; s < kMaxSteps; ++s) {
            if (lane.constraints.anchorSteps.steps[static_cast<size_t>(s)] > 0.0f)
                anchorBits |= (uint64_t{1} << s);
        }
        if (!write(&anchorBits, sizeof(anchorBits)))
            return false;
        uint8_t bbp = lane.constraints.backbeatProtect ? 1 : 0;
        if (!write(&bbp, sizeof(bbp)))
            return false;
        if (!write(&lane.constraints.densityMin, sizeof(lane.constraints.densityMin)))
            return false;
        if (!write(&lane.constraints.densityMax, sizeof(lane.constraints.densityMax)))
            return false;
    }

    if (bodyVersion >= 5) {
        if (!write(&lane.phraseLength, sizeof(lane.phraseLength)))
            return false;
        if (!write(&lane.phraseGap, sizeof(lane.phraseGap)))
            return false;
        if (!write(&lane.phraseOffset, sizeof(lane.phraseOffset)))
            return false;
    }

    if (bodyVersion >= 6) {
        if (!write(&lane.mutationRate, sizeof(lane.mutationRate)))
            return false;
    }
    if (bodyVersion >= 7) {
        if (!write(&lane.driftRate, sizeof(lane.driftRate)))
            return false;
    }
    if (bodyVersion >= 8) {
        if (!write(&lane.timingOffsetMs, sizeof(lane.timingOffsetMs)))
            return false;
    }
    if (bodyVersion >= 9) {
        if (!write(&lane.kotekanSourceLane, sizeof(lane.kotekanSourceLane)))
            return false;
    }

    if (bodyVersion >= 10) {
        if (!write(&lane.cellCount, sizeof(lane.cellCount)))
            return false;
        for (int s = 0; s < kMaxSteps; ++s) {
            if (!write(&lane.cellSizes[static_cast<size_t>(s)], sizeof(int)))
                return false;
        }
        uint8_t tl = lane.timeline ? 1 : 0;
        if (!write(&tl, sizeof(tl)))
            return false;
        if (!write(&lane.fixedPatternLength, sizeof(lane.fixedPatternLength)))
            return false;
        uint64_t fpBits = 0;
        for (int s = 0; s < kMaxSteps; ++s) {
            if (lane.fixedPattern[static_cast<size_t>(s)])
                fpBits |= (uint64_t{1} << s);
        }
        if (!write(&fpBits, sizeof(fpBits)))
            return false;
        for (int s = 0; s < kMaxSteps; ++s) {
            if (!write(&lane.microTimingMs[static_cast<size_t>(s)], sizeof(float)))
                return false;
        }
    }

    if (bodyVersion >= 14) {
        if (!write(&lane.tempoMultiplier, sizeof(lane.tempoMultiplier)))
            return false;
    }
    if (bodyVersion >= 15) {
        if (!write(&lane.midiChannel, sizeof(lane.midiChannel)))
            return false;
    }

    return true;
}

} // namespace poly
