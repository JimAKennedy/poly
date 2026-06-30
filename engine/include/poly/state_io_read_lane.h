#pragma once

#include <cstddef>
#include <cstdint>

#include "poly/types.h"

namespace poly {

template <typename ReadFn> [[nodiscard]] bool readLaneConfig(ReadFn&& read, LaneConfig& lane, int32_t version) {
    if (!read(&lane.id, sizeof(lane.id)))
        return false;
    uint8_t role = 0;
    if (!read(&role, sizeof(role)))
        return false;
    lane.role = static_cast<Role>(role);
    if (!read(&lane.midiNote, sizeof(lane.midiNote)))
        return false;
    if (!read(&lane.cycle.steps, sizeof(lane.cycle.steps)))
        return false;
    if (!read(&lane.cycle.subdivision, sizeof(lane.cycle.subdivision)))
        return false;
    if (!read(&lane.hitCount, sizeof(lane.hitCount)))
        return false;
    if (!read(&lane.rotation, sizeof(lane.rotation)))
        return false;
    if (!read(&lane.probability, sizeof(lane.probability)))
        return false;
    if (!read(&lane.baseVelocity, sizeof(lane.baseVelocity)))
        return false;

    if (version >= 12) {
        for (int s = 0; s < kMaxSteps; ++s) {
            if (!read(&lane.accents.steps[static_cast<size_t>(s)], sizeof(float)))
                return false;
        }
    } else {
        uint64_t accentBits = 0;
        if (!read(&accentBits, sizeof(accentBits)))
            return false;
        for (int s = 0; s < kMaxSteps; ++s) {
            lane.accents.steps[static_cast<size_t>(s)] = (accentBits & (uint64_t{1} << s)) != 0 ? 1.0f : 0.0f;
        }
    }

    if (!read(&lane.emphasisProb, sizeof(lane.emphasisProb)))
        return false;
    if (!read(&lane.ghostFloor, sizeof(lane.ghostFloor)))
        return false;
    if (!read(&lane.velocitySpread, sizeof(lane.velocitySpread)))
        return false;
    if (!read(&lane.humanizeMs, sizeof(lane.humanizeMs)))
        return false;
    if (!read(&lane.swingAmount, sizeof(lane.swingAmount)))
        return false;
    if (!read(&lane.noteDuration, sizeof(lane.noteDuration)))
        return false;
    uint8_t active = 0;
    if (!read(&active, sizeof(active)))
        return false;
    lane.active = (active != 0);
    if (!read(&lane.envelopeCount, sizeof(lane.envelopeCount)))
        return false;

    for (int e = 0; e < kMaxEnvelopesPerLane; ++e) {
        auto& ea = lane.envelopes[static_cast<size_t>(e)];
        if (!readEnvelope(read, ea.envelope, version))
            return false;
        uint8_t envActive = 0;
        if (!read(&envActive, sizeof(envActive)))
            return false;
        ea.active = (envActive != 0);
    }

    if (version >= 4) {
        uint64_t anchorBits = 0;
        if (!read(&anchorBits, sizeof(anchorBits)))
            return false;
        for (int s = 0; s < kMaxSteps; ++s) {
            lane.constraints.anchorSteps.steps[static_cast<size_t>(s)] =
                (anchorBits & (uint64_t{1} << s)) != 0 ? 1.0f : 0.0f;
        }
        uint8_t bbp = 0;
        if (!read(&bbp, sizeof(bbp)))
            return false;
        lane.constraints.backbeatProtect = (bbp != 0);
        if (!read(&lane.constraints.densityMin, sizeof(lane.constraints.densityMin)))
            return false;
        if (!read(&lane.constraints.densityMax, sizeof(lane.constraints.densityMax)))
            return false;
    }

    if (version >= 5) {
        if (!read(&lane.phraseLength, sizeof(lane.phraseLength)))
            return false;
        if (!read(&lane.phraseGap, sizeof(lane.phraseGap)))
            return false;
        if (!read(&lane.phraseOffset, sizeof(lane.phraseOffset)))
            return false;
    }

    if (version >= 6) {
        if (!read(&lane.mutationRate, sizeof(lane.mutationRate)))
            return false;
    }
    if (version >= 7) {
        if (!read(&lane.driftRate, sizeof(lane.driftRate)))
            return false;
    }
    if (version >= 8) {
        if (!read(&lane.timingOffsetMs, sizeof(lane.timingOffsetMs)))
            return false;
    }
    if (version >= 9) {
        if (!read(&lane.kotekanSourceLane, sizeof(lane.kotekanSourceLane)))
            return false;
    }

    if (version >= 10) {
        if (!read(&lane.cellCount, sizeof(lane.cellCount)))
            return false;
        for (int s = 0; s < kMaxSteps; ++s) {
            if (!read(&lane.cellSizes[static_cast<size_t>(s)], sizeof(int)))
                return false;
        }
        uint8_t tl = 0;
        if (!read(&tl, sizeof(tl)))
            return false;
        lane.timeline = (tl != 0);
        if (!read(&lane.fixedPatternLength, sizeof(lane.fixedPatternLength)))
            return false;
        uint64_t fpBits = 0;
        if (!read(&fpBits, sizeof(fpBits)))
            return false;
        for (int s = 0; s < kMaxSteps; ++s) {
            lane.fixedPattern[static_cast<size_t>(s)] = (fpBits & (uint64_t{1} << s)) != 0;
        }
        for (int s = 0; s < kMaxSteps; ++s) {
            if (!read(&lane.microTimingMs[static_cast<size_t>(s)], sizeof(float)))
                return false;
        }
    }

    if (version >= 14) {
        if (!read(&lane.tempoMultiplier, sizeof(lane.tempoMultiplier)))
            return false;
    }
    if (version >= 15) {
        if (!read(&lane.midiChannel, sizeof(lane.midiChannel)))
            return false;
    }

    return true;
}

} // namespace poly
