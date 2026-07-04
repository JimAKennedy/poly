#pragma once

// Maps bridge paramId strings (webui/bridge-schema.md) to VST ParamIDs so
// the JS side never hardcodes numeric IDs. Table-driven: add a row to
// kLaneCoreFields or kLaneExprFields to wire a new lane parameter.

#include <cstdio>
#include <cstring>
#include <optional>

#include "../plugids.h"
#include "poly/scene.h"

namespace poly::webui {

namespace detail {

struct FieldMapping {
    const char* field;
    int offset;
};

inline constexpr FieldMapping kLaneExprFields[] = {
    {"probability", ParamIDs::kProbability},
    {"velocity", ParamIDs::kBaseVelocity},
    {"emphasisProb", ParamIDs::kEmphasisProb},
    {"ghostFloor", ParamIDs::kGhostFloor},
    {"spread", ParamIDs::kVelocitySpread},
    {"swing", ParamIDs::kSwingAmount},
    {"humanize", ParamIDs::kHumanizeMs},
    {"duration", ParamIDs::kNoteDuration},
    {"active", ParamIDs::kActive},
    {"phraseLength", ParamIDs::kPhraseLength},
    {"phraseGap", ParamIDs::kPhraseGap},
    {"phraseOffset", ParamIDs::kPhraseOffset},
    {"mutationRate", ParamIDs::kMutationRate},
    {"driftRate", ParamIDs::kDriftRate},
    {"timingOffset", ParamIDs::kTimingOffset},
    {"kotekanSource", ParamIDs::kKotekanSource},
};

inline constexpr FieldMapping kLaneCoreFields[] = {
    {"steps", ParamIDs::kCoreSteps},         {"subdivision", ParamIDs::kCoreSubdivision},
    {"hits", ParamIDs::kCoreHits},           {"rotation", ParamIDs::kCoreRotation},
    {"note", ParamIDs::kCoreMidiNote},       {"cellCount", ParamIDs::kCoreCellCount},
    {"timeline", ParamIDs::kCoreTimeline},   {"fixedPatternLen", ParamIDs::kCoreFixedPatternLen},
    {"tempoMult", ParamIDs::kCoreTempoMult}, {"channel", ParamIDs::kCoreMidiChannel},
};

} // namespace detail

inline std::optional<Steinberg::Vst::ParamID> resolveParamId(const char* name) {
    using namespace ParamIDs;

    if (std::strncmp(name, "macro.", 6) == 0) {
        const char* f = name + 6;
        if (std::strcmp(f, "complexity") == 0)
            return kMacroComplexity;
        if (std::strcmp(f, "density") == 0)
            return kMacroDensity;
        if (std::strcmp(f, "syncopation") == 0)
            return kMacroSyncopation;
        if (std::strcmp(f, "swing") == 0)
            return kMacroSwing;
        if (std::strcmp(f, "tension") == 0)
            return kMacroTension;
        if (std::strcmp(f, "humanize") == 0)
            return kMacroHumanize;
        return std::nullopt;
    }

    if (std::strcmp(name, "scene.morph") == 0)
        return kSceneMorph;
    if (std::strcmp(name, "scene.select") == 0)
        return kSceneSelect;

    if (std::strcmp(name, "activeLaneCount") == 0)
        return kActiveLaneCount;
    if (std::strcmp(name, "seed") == 0)
        return kSeed;

    if (std::strcmp(name, "chain.enabled") == 0)
        return kChainEnabled;
    if (std::strcmp(name, "chain.mode") == 0)
        return kChainMode;

    int chainEntry = -1;
    char chainField[32] = {};
    if (std::sscanf(name, "chain.entry.%d.%31s", &chainEntry, chainField) == 2 && chainEntry >= 0 &&
        chainEntry < kMaxChainEntries) {
        if (std::strcmp(chainField, "scene") == 0)
            return chainEntryParam(chainEntry, kChainEntryScene);
        if (std::strcmp(chainField, "bars") == 0)
            return chainEntryParam(chainEntry, kChainEntryBars);
    }

    int lane = -1;
    char field[32] = {};
    if (std::sscanf(name, "lane.%d.%31s", &lane, field) == 2 && lane >= 0 && lane < kMaxLanes) {
        for (const auto& m : detail::kLaneCoreFields) {
            if (std::strcmp(field, m.field) == 0)
                return laneCoreParam(lane, m.offset);
        }
        for (const auto& m : detail::kLaneExprFields) {
            if (std::strcmp(field, m.field) == 0)
                return laneParam(lane, m.offset);
        }
    }
    return std::nullopt;
}

} // namespace poly::webui
