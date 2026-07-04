#include "bridge_serialization.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>

#include "poly/euclidean.h"
#include "poly/presets.h"

namespace poly {

std::string escapeJsonString(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    out += '"';
    for (char c : s) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
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
            out += c;
        }
    }
    out += '"';
    return out;
}

const char* roleToString(Role r) {
    switch (r) {
    case Role::AnchorPulse:
        return "Anchor pulse";
    case Role::Backbeat:
        return "Backbeat";
    case Role::Shimmer:
        return "Shimmer";
    case Role::Accent:
        return "Accent";
    case Role::Ghost:
        return "Ghost";
    case Role::Ornament:
        return "Ornament";
    case Role::Fill:
        return "Fill";
    case Role::Custom:
        return "Custom";
    }
    return "Custom";
}

const char* envTargetToString(EnvTarget t) {
    switch (t) {
    case EnvTarget::Velocity:
        return "Velocity";
    case EnvTarget::Density:
        return "Density";
    case EnvTarget::Probability:
        return "Probability";
    case EnvTarget::AccentBias:
        return "AccentBias";
    case EnvTarget::NoteLength:
        return "NoteLength";
    case EnvTarget::TimingLooseness:
        return "TimingLooseness";
    case EnvTarget::ActivationWeight:
        return "ActivationWeight";
    case EnvTarget::FillLikelihood:
        return "FillLikelihood";
    }
    return "Velocity";
}

const char* sceneSelectToString(SceneSelect s) {
    switch (s) {
    case SceneSelect::A:
        return "A";
    case SceneSelect::B:
        return "B";
    case SceneSelect::Morph:
        return "Morph";
    }
    return "A";
}

static const char* kLaneHues[] = {"#F5B54A", "#E4604E", "#5AC8DA", "#9BC46B",
                                  "#B48AE0", "#E8A33D", "#4EBBE0", "#D47AB8"};

std::string laneToJson(const LaneConfig& cfg, const std::string& name, int laneIndex) {
    std::string js;
    js.reserve(1024);
    js += '{';

    js += "\"name\":" + escapeJsonString(name);
    js += ",\"hue\":\"";
    js += kLaneHues[laneIndex % 8];
    js += '"';
    js += ",\"role\":\"";
    js += roleToString(cfg.role);
    js += '"';

    char buf[512];
    std::snprintf(buf, sizeof(buf),
                  ",\"note\":%d,\"ch\":%d,\"steps\":%d,\"subdivision\":%d"
                  ",\"vel\":%u,\"prob\":%.4f,\"spread\":%.4f"
                  ",\"ghost\":%u,\"push\":%.4f"
                  ",\"hits\":%d,\"rot\":%d,\"timeline\":%s"
                  ",\"active\":%s",
                  cfg.midiNote, cfg.midiChannel, cfg.cycle.steps, cfg.cycle.subdivision,
                  static_cast<unsigned>(cfg.baseVelocity), static_cast<double>(cfg.probability),
                  static_cast<double>(cfg.velocitySpread), static_cast<unsigned>(cfg.ghostFloor),
                  static_cast<double>(cfg.syncopationOffset), cfg.hitCount, cfg.rotation,
                  cfg.timeline ? "true" : "false", cfg.active ? "true" : "false");
    js += buf;

    double stepLen = 8.0 / cfg.cycle.subdivision;
    std::snprintf(buf, sizeof(buf), ",\"stepLen\":%.6g", stepLen);
    js += buf;

    std::snprintf(buf, sizeof(buf),
                  ",\"humanize\":%.4f,\"swing\":%.4f,\"duration\":%.4f"
                  ",\"emphasisProb\":%.4f,\"timingOffset\":%.4f"
                  ",\"mutationRate\":%.4f,\"driftRate\":%.4f"
                  ",\"phraseLength\":%.4f,\"phraseGap\":%.4f,\"phraseOffset\":%.4f"
                  ",\"tempoMultiplier\":%.4f,\"kotekanSource\":%d"
                  ",\"cellCount\":%d",
                  static_cast<double>(cfg.humanizeMs), static_cast<double>(cfg.swingAmount),
                  static_cast<double>(cfg.noteDuration), static_cast<double>(cfg.emphasisProb),
                  static_cast<double>(cfg.timingOffsetMs), static_cast<double>(cfg.mutationRate),
                  static_cast<double>(cfg.driftRate), static_cast<double>(cfg.phraseLength),
                  static_cast<double>(cfg.phraseGap), static_cast<double>(cfg.phraseOffset),
                  static_cast<double>(cfg.tempoMultiplier), cfg.kotekanSourceLane, cfg.cellCount);
    js += buf;

    // pattern (computed from Euclidean or fixed)
    std::array<bool, kMaxSteps> pat{};
    if (cfg.timeline) {
        int len = cfg.fixedPatternLength > 0 ? cfg.fixedPatternLength : cfg.cycle.steps;
        for (int i = 0; i < len && i < kMaxSteps; ++i)
            pat[i] = cfg.fixedPattern[i];
    } else {
        euclidean(cfg.hitCount, cfg.cycle.steps, cfg.rotation, pat);
    }
    js += ",\"pattern\":[";
    for (int i = 0; i < cfg.cycle.steps; ++i) {
        if (i > 0)
            js += ',';
        js += pat[i] ? '1' : '0';
    }
    js += ']';

    // fixed pattern
    if (cfg.timeline) {
        int len = cfg.fixedPatternLength > 0 ? cfg.fixedPatternLength : cfg.cycle.steps;
        js += ",\"fixed\":[";
        for (int i = 0; i < len; ++i) {
            if (i > 0)
                js += ',';
            js += cfg.fixedPattern[i] ? '1' : '0';
        }
        js += ']';
    } else {
        js += ",\"fixed\":null";
    }

    // cells
    if (cfg.cellCount > 0) {
        js += ",\"cells\":[";
        for (int i = 0; i < cfg.cellCount; ++i) {
            if (i > 0)
                js += ',';
            std::snprintf(buf, sizeof(buf), "%d", cfg.cellSizes[i]);
            js += buf;
        }
        js += ']';
    } else {
        js += ",\"cells\":null";
    }

    // micro-timing
    js += ",\"mt\":[";
    for (int i = 0; i < cfg.cycle.steps; ++i) {
        if (i > 0)
            js += ',';
        std::snprintf(buf, sizeof(buf), "%.2f", static_cast<double>(cfg.microTimingMs[i]));
        js += buf;
    }
    js += ']';

    // envelopes
    js += ",\"envs\":[";
    for (int i = 0; i < cfg.envelopeCount && i < kMaxEnvelopesPerLane; ++i) {
        if (i > 0)
            js += ',';
        const auto& ea = cfg.envelopes[i];
        std::snprintf(buf, sizeof(buf),
                      "{\"target\":\"%s\",\"period\":%.2f,\"depth\":%.4f,\"on\":%s"
                      ",\"shape\":%d,\"curvature\":%.4f,\"phaseOffset\":%.4f}",
                      envTargetToString(ea.envelope.target), static_cast<double>(ea.envelope.periodBars),
                      static_cast<double>(ea.envelope.depth), ea.active ? "true" : "false",
                      static_cast<int>(ea.envelope.shape), static_cast<double>(ea.envelope.curvature),
                      static_cast<double>(ea.envelope.phaseOffset));
        js += buf;
    }
    js += ']';

    // accent mask
    js += ",\"accents\":[";
    for (int i = 0; i < cfg.cycle.steps; ++i) {
        if (i > 0)
            js += ',';
        std::snprintf(buf, sizeof(buf), "%.4f", static_cast<double>(cfg.accents.steps[i]));
        js += buf;
    }
    js += ']';

    js += '}';
    return js;
}

std::string grooveStateToJson(const GrooveState& gs, const SceneState& ss, LaneNameFn nameFunc, void* nameCtx,
                              const std::string& presetName) {
    std::string js;
    js.reserve(8192);
    js += "{\"type\":\"state\",\"state\":{";

    js += "\"preset\":" + escapeJsonString(presetName);

    char buf[128];
    std::snprintf(buf, sizeof(buf), ",\"seed\":%llu,\"tempo\":0", static_cast<unsigned long long>(gs.seed));
    js += buf;

    js += ",\"scene\":\"";
    js += sceneSelectToString(ss.select);
    js += '"';
    std::snprintf(buf, sizeof(buf), ",\"morph\":%.4f", static_cast<double>(ss.morphAmount));
    js += buf;

    std::snprintf(buf, sizeof(buf),
                  ",\"macros\":{\"complexity\":%.4f,\"density\":%.4f,\"syncopation\":%.4f"
                  ",\"swing\":%.4f,\"tension\":%.4f,\"humanize\":%.4f}",
                  static_cast<double>(gs.macros.complexity), static_cast<double>(gs.macros.density),
                  static_cast<double>(gs.macros.syncopation), static_cast<double>(gs.macros.swing),
                  static_cast<double>(gs.macros.tension), static_cast<double>(gs.macros.humanize));
    js += buf;

    js += ",\"lanes\":[";
    for (int i = 0; i < gs.activeLaneCount; ++i) {
        if (i > 0)
            js += ',';
        js += laneToJson(gs.lanes[i], nameFunc(i, nameCtx), i);
    }
    js += ']';

    js += ",\"presets\":[";
    for (int i = 0; i < kFactoryPresetCount; ++i) {
        if (i > 0)
            js += ',';
        const auto& info = getFactoryPresetInfo(i);
        js += "{\"name\":" + escapeJsonString(info.name);
        js += ",\"description\":" + escapeJsonString(info.description) + "}";
    }
    js += ']';

    // Chain
    js += ",\"chain\":{\"enabled\":";
    js += ss.chain.enabled ? "true" : "false";
    char chainBuf[64];
    std::snprintf(chainBuf, sizeof(chainBuf), ",\"mode\":%d,\"entryCount\":%d", static_cast<int>(ss.chain.mode),
                  ss.chain.entryCount);
    js += chainBuf;
    js += ",\"entries\":[";
    for (int i = 0; i < ss.chain.entryCount; ++i) {
        if (i > 0)
            js += ',';
        std::snprintf(chainBuf, sizeof(chainBuf), "{\"scene\":%d,\"bars\":%d}",
                      static_cast<int>(ss.chain.entries[i].scene), ss.chain.entries[i].bars);
        js += chainBuf;
    }
    js += "]}";

    // NoteMap
    js += ",\"noteMap\":[";
    for (int i = 0; i < 128; ++i) {
        if (i > 0)
            js += ',';
        char nmBuf[8];
        std::snprintf(nmBuf, sizeof(nmBuf), "%d", ss.noteMap.map[i]);
        js += nmBuf;
    }
    js += ']';

    js += "}}";
    return js;
}

} // namespace poly
