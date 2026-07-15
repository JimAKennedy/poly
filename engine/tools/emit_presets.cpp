// Emits all factory presets as a JSON document on stdout.
//
// Consumed by the site prebuild pipeline (M043 S11 T02+): produces the single
// source of truth for factory preset lane data so the card runtime, the WASM
// host, the alias map, and the chapter documentation cannot drift.
//
// Schema:
// {
//   "schemaVersion": 2,
//   "presetCount": 43,
//   "categories": ["Foundational", "Minimalist / Compositional", ...],  // ordered
//   "presets": [
//     {
//       "index": 0,
//       "name": "Four on the Floor",
//       "description": "...",
//       "category": "House / Techno",  // one of `categories`
//       "activeLaneCount": 4,
//       "notesInBar": 16,          // composite loop length in 16th-note ticks
//                                  // (LCM of steps * 16/subdivision across active lanes)
//       "seed": 1,
//       "lanes": [
//         {
//           "laneIndex": 0,
//           "noteNumber": 36,
//           "roleLabel": "kick",   // derived from GM MIDI note number
//           "role": "AnchorPulse", // engine Role enum name
//           "cycleSteps": 4,
//           "subdivision": 4,      // 4=quarter, 8=eighth, 16=sixteenth
//           "stepLen": 4,          // 16/subdivision — 16th-note ticks per step
//           "hits": 4,
//           "rotation": 0,
//           "velocity": 110,
//           "probability": 1.0
//         }, ...
//       ]
//     }, ...
//   ]
// }

#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>

#include "poly/presets.h"
#include "poly/types.h"

namespace {

// GM percussion mapping — matches the taxonomy used by the site sample manifest
// and preset-patterns.ts. Anything not in this table falls through to "perc".
const char* roleLabelForNote(int note) {
    switch (note) {
    case 35:
    case 36:
        return "kick";
    case 37:
        return "rim";
    case 38:
    case 40:
        return "snare";
    case 39:
        return "clap";
    case 41:
    case 43:
    case 45:
    case 47:
    case 48:
    case 50:
        return "tom";
    case 42:
    case 44:
        return "hat";
    case 46:
        return "openhat";
    case 49:
    case 51:
    case 52:
    case 53:
    case 55:
    case 57:
    case 59:
        return "cymbal";
    case 56:
        return "cowbell";
    case 60:
    case 61:
        return "bongo";
    case 62:
    case 63:
    case 64:
        return "conga";
    case 65:
    case 66:
        return "timbale";
    case 67:
    case 68:
        return "agogo";
    case 69:
    case 70:
    case 71:
    case 72:
        return "shaker";
    case 73:
    case 74:
        return "guiro";
    case 75:
        return "clave";
    case 76:
    case 77:
        return "woodblock";
    case 78:
    case 79:
        return "cuica";
    case 80:
    case 81:
        return "triangle";
    default:
        return "perc";
    }
}

const char* roleName(poly::Role r) {
    switch (r) {
    case poly::Role::AnchorPulse:
        return "AnchorPulse";
    case poly::Role::Backbeat:
        return "Backbeat";
    case poly::Role::Shimmer:
        return "Shimmer";
    case poly::Role::Accent:
        return "Accent";
    case poly::Role::Ghost:
        return "Ghost";
    case poly::Role::Ornament:
        return "Ornament";
    case poly::Role::Fill:
        return "Fill";
    case poly::Role::Custom:
        return "Custom";
    }
    return "Custom";
}

// Composite loop length in 16th-note ticks: LCM of (steps * 16/subdivision)
// across active lanes. Matches the site's tick math (TICKS_PER_BEAT=4).
int compositeSteps16ths(const poly::GrooveState& s) {
    int result = 1;
    int lanes = s.activeLaneCount;
    if (lanes <= 0)
        return 16;
    if (lanes > poly::kMaxLanes)
        lanes = poly::kMaxLanes;
    for (int i = 0; i < lanes; ++i) {
        const auto& l = s.lanes[i];
        if (!l.active)
            continue;
        int subdivision = l.cycle.subdivision > 0 ? l.cycle.subdivision : 4;
        int stepLen = 16 / subdivision;
        if (stepLen <= 0)
            stepLen = 1;
        int laneTicks = l.cycle.steps * stepLen;
        if (laneTicks <= 0)
            continue;
        result = std::lcm(result, laneTicks);
    }
    return result;
}

std::string escapeJson(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\b':
            out += "\\b";
            break;
        case '\f':
            out += "\\f";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                out += buf;
            } else {
                out += c;
            }
        }
    }
    return out;
}

void writeFloat(std::ostringstream& out, float v) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.6g", static_cast<double>(v));
    out << buf;
}

void writeLane(std::ostringstream& out, int laneIndex, const poly::LaneConfig& lane) {
    int subdivision = lane.cycle.subdivision > 0 ? lane.cycle.subdivision : 4;
    int stepLen = 16 / subdivision;
    if (stepLen <= 0)
        stepLen = 1;
    out << "      {"
        << "\"laneIndex\":" << laneIndex << ",\"noteNumber\":" << lane.midiNote << ",\"roleLabel\":\""
        << roleLabelForNote(lane.midiNote) << "\""
        << ",\"role\":\"" << roleName(lane.role) << "\""
        << ",\"cycleSteps\":" << lane.cycle.steps << ",\"subdivision\":" << subdivision << ",\"stepLen\":" << stepLen
        << ",\"hits\":" << lane.hitCount << ",\"rotation\":" << lane.rotation
        << ",\"velocity\":" << static_cast<int>(lane.baseVelocity) << ",\"probability\":";
    writeFloat(out, lane.probability);
    out << "}";
}

void writePreset(std::ostringstream& out, int index) {
    poly::GrooveState state = poly::makeFactoryPreset(index);
    const poly::PresetInfo& info = poly::getFactoryPresetInfo(index);
    int lanes = state.activeLaneCount;
    if (lanes < 0)
        lanes = 0;
    if (lanes > poly::kMaxLanes)
        lanes = poly::kMaxLanes;

    out << "  {\n"
        << "    \"index\":" << index << ",\n"
        << "    \"name\":\"" << escapeJson(info.name) << "\",\n"
        << "    \"description\":\"" << escapeJson(info.description) << "\",\n"
        << "    \"category\":\"" << escapeJson(info.category) << "\",\n"
        << "    \"activeLaneCount\":" << lanes << ",\n"
        << "    \"notesInBar\":" << compositeSteps16ths(state) << ",\n"
        << "    \"seed\":" << state.seed << ",\n"
        << "    \"lanes\":[\n";
    for (int i = 0; i < lanes; ++i) {
        writeLane(out, i, state.lanes[i]);
        if (i + 1 < lanes)
            out << ",";
        out << "\n";
    }
    out << "    ]\n"
        << "  }";
}

} // namespace

int main() {
    std::ostringstream out;
    out << "{\n"
        << "  \"schemaVersion\":2,\n"
        << "  \"presetCount\":" << poly::kFactoryPresetCount << ",\n"
        << "  \"categories\":[";
    for (int i = 0; i < poly::kFactoryPresetCategoryCount; ++i) {
        out << "\"" << escapeJson(poly::kFactoryPresetCategories[i]) << "\"";
        if (i + 1 < poly::kFactoryPresetCategoryCount)
            out << ",";
    }
    out << "],\n"
        << "  \"presets\":[\n";
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        writePreset(out, i);
        if (i + 1 < poly::kFactoryPresetCount)
            out << ",";
        out << "\n";
    }
    out << "  ]\n"
        << "}\n";
    std::fputs(out.str().c_str(), stdout);
    return 0;
}
