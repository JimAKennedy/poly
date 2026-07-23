// Emits the lane-scoped parameter registry as a JSON document on stdout.
//
// Consumed by site prebuild + doc regen (M048 S05): produces the single
// source of truth for parameter appendix + engine-spec LaneConfig tables
// so hand-edited docs cannot drift from engine/include/poly/params_def.h.
//
// Schema:
// {
//   "schemaVersion": 1,
//   "expressionParams": [ { "offset": 0, "name": "Probability",
//                           "kind": "Unit01", "minEngine": 0, "maxEngine": 1,
//                           "defaultEngine": 1, "engineRange": "0.0 - 1.0",
//                           "normalizedRange": "0 - 100%",
//                           "defaultDisplay": "100%" }, ... ],
//   "coreParams":       [ ... same shape, 10 entries ... ]
// }

#include <cstdio>
#include <string>

#include "poly/params_def.h"

namespace {

const char* kindName(poly::params::Kind k) {
    using K = poly::params::Kind;
    switch (k) {
    case K::Unit01:
        return "Unit01";
    case K::LinearFloat:
        return "LinearFloat";
    case K::Byte7:
        return "Byte7";
    case K::Ranged1_64:
        return "Ranged1_64";
    case K::Ranged0_63:
        return "Ranged0_63";
    case K::Ranged0_64:
        return "Ranged0_64";
    case K::Bool:
        return "Bool";
    case K::Subdivision:
        return "Subdivision";
    case K::TempoMult:
        return "TempoMult";
    case K::DriftRate:
        return "DriftRate";
    case K::TimingOffs:
        return "TimingOffs";
    case K::MidiChannel:
        return "MidiChannel";
    case K::KotekanSrc:
        return "KotekanSrc";
    }
    return "Unknown";
}

std::string engineRange(const poly::params::Entry& e) {
    using K = poly::params::Kind;
    char buf[128];
    switch (e.kind) {
    case K::Unit01:
        return "0.0 - 1.0";
    case K::Bool:
        return "Off / On";
    case K::Subdivision:
        return "1, 2, 4, 8, 16";
    case K::Byte7:
        return "0 - 127";
    case K::Ranged1_64:
        return "1 - 64";
    case K::Ranged0_63:
        return "0 - 63";
    case K::Ranged0_64:
        return "0 - 64";
    case K::TempoMult:
        return "0.25x - 4.0x";
    case K::DriftRate:
        return "-4.0 to +4.0 steps/bar";
    case K::TimingOffs:
        return "-20 to +20 ms";
    case K::MidiChannel:
        return "-1 to 15";
    case K::KotekanSrc:
        return "-1 to 7";
    case K::LinearFloat:
        std::snprintf(buf, sizeof(buf), "%g - %g", e.minEngine, e.maxEngine);
        return buf;
    }
    return "";
}

std::string defaultDisplay(const poly::params::Entry& e) {
    using K = poly::params::Kind;
    char buf[64];
    switch (e.kind) {
    case K::Unit01:
        std::snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(e.defaultEngine * 100.0 + 0.5));
        return buf;
    case K::Bool:
        return e.defaultEngine > 0.5 ? "On" : "Off";
    case K::Subdivision:
        std::snprintf(buf, sizeof(buf), "%d (1/%d note)", static_cast<int>(e.defaultEngine),
                      static_cast<int>(e.defaultEngine));
        return buf;
    case K::TempoMult:
        std::snprintf(buf, sizeof(buf), "%gx", e.defaultEngine);
        return buf;
    case K::DriftRate:
    case K::TimingOffs:
    case K::LinearFloat:
        std::snprintf(buf, sizeof(buf), "%g", e.defaultEngine);
        return buf;
    case K::MidiChannel:
        if (e.defaultEngine < 0.0)
            return "Auto";
        std::snprintf(buf, sizeof(buf), "%d", static_cast<int>(e.defaultEngine));
        return buf;
    case K::KotekanSrc:
        if (e.defaultEngine < 0.0)
            return "Independent";
        std::snprintf(buf, sizeof(buf), "Lane %d", static_cast<int>(e.defaultEngine) + 1);
        return buf;
    case K::Byte7:
    case K::Ranged1_64:
    case K::Ranged0_63:
    case K::Ranged0_64:
        std::snprintf(buf, sizeof(buf), "%d", static_cast<int>(e.defaultEngine));
        return buf;
    }
    return "";
}

std::string jsonEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (c == '\n')
            out += "\\n";
        else
            out += c;
    }
    return out;
}

void printEntry(const poly::params::Entry& e, bool last) {
    std::printf("    {\n");
    std::printf("      \"offset\": %u,\n", e.offset);
    std::printf("      \"name\": \"%s\",\n", jsonEscape(e.name).c_str());
    std::printf("      \"kind\": \"%s\",\n", kindName(e.kind));
    std::printf("      \"minEngine\": %g,\n", e.minEngine);
    std::printf("      \"maxEngine\": %g,\n", e.maxEngine);
    std::printf("      \"defaultEngine\": %g,\n", e.defaultEngine);
    std::printf("      \"engineRange\": \"%s\",\n", jsonEscape(engineRange(e)).c_str());
    std::printf("      \"defaultDisplay\": \"%s\"\n", jsonEscape(defaultDisplay(e)).c_str());
    std::printf("    }%s\n", last ? "" : ",");
}

} // namespace

int main() {
    std::printf("{\n");
    std::printf("  \"schemaVersion\": 1,\n");

    std::printf("  \"expressionParams\": [\n");
    const auto& expr = poly::params::kLaneExprParamRegistry;
    for (size_t i = 0; i < expr.size(); ++i)
        printEntry(expr[i], i + 1 == expr.size());
    std::printf("  ],\n");

    std::printf("  \"coreParams\": [\n");
    const auto& core = poly::params::kLaneCoreParamRegistry;
    for (size_t i = 0; i < core.size(); ++i)
        printEntry(core[i], i + 1 == core.size());
    std::printf("  ]\n");

    std::printf("}\n");
    return 0;
}
