#pragma once

// Maps bridge paramId strings (webui/bridge-schema.md) to VST ParamIDs so
// the JS side never hardcodes numeric IDs. Scaffold: extend alongside the
// panes the web UI actually renders.

#include <cstdio>
#include <cstring>
#include <optional>

#include "../plugids.h"

namespace poly::webui {

inline std::optional<Steinberg::Vst::ParamID> resolveParamId(const char* name) {
    using namespace ParamIDs;
    if (std::strcmp(name, "macro.complexity") == 0)
        return kMacroComplexity;
    if (std::strcmp(name, "macro.density") == 0)
        return kMacroDensity;
    if (std::strcmp(name, "macro.syncopation") == 0)
        return kMacroSyncopation;
    if (std::strcmp(name, "macro.swing") == 0)
        return kMacroSwing;
    if (std::strcmp(name, "macro.tension") == 0)
        return kMacroTension;
    if (std::strcmp(name, "macro.humanize") == 0)
        return kMacroHumanize;

    // "lane.<n>.<field>" -> lane / core params
    int lane = -1;
    char field[32] = {};
    if (std::sscanf(name, "lane.%d.%31s", &lane, field) == 2 && lane >= 0 && lane < kMaxLanes) {
        if (std::strcmp(field, "hits") == 0)
            return laneCoreParam(lane, kCoreHits);
        if (std::strcmp(field, "steps") == 0)
            return laneCoreParam(lane, kCoreSteps);
        if (std::strcmp(field, "rotation") == 0)
            return laneCoreParam(lane, kCoreRotation);
        if (std::strcmp(field, "cellCount") == 0)
            return laneCoreParam(lane, kCoreCellCount);
        if (std::strcmp(field, "timeline") == 0)
            return laneCoreParam(lane, kCoreTimeline);
        // TODO(spike): velocity/probability/push etc. (expression params)
    }
    return std::nullopt;
}

} // namespace poly::webui
