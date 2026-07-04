// Web UI view — hosts the webui/ bundle in a choc::ui::WebView and
// bridges to the controller per webui/bridge-schema.md. Compiled only
// with -DPOLY_WEB_UI=ON.

#include "web_ui_view.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "../controller.h"
#include "../plugids.h"
#include "bridge_params.h"
#include "choc/gui/choc_MessageLoop.h"
#include "choc/gui/choc_WebView.h"
#include "choc/text/choc_JSON.h"
#include "poly/euclidean.h"
#include "poly/presets.h"
#include "poly_webui_assets.h" // generated: jk_embed_assets(webui/*)

#ifdef __APPLE__
#include <objc/message.h>
#include <objc/runtime.h>
#endif

namespace poly {

// --- State serialization helpers ---

static std::string escapeJsonString(const std::string& s) {
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

static const char* roleToString(Role r) {
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

static const char* envTargetToString(EnvTarget t) {
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

static const char* sceneSelectToString(SceneSelect s) {
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

static const char* kWebPresetLaneNames[kFactoryPresetCount][kMaxLanes] = {
    {"Kick", "Snare", "Hi-Hat", "Open Hat", "Tom Hi", "Tom Lo", "Ride", "Crash"},
    {"Kick", "Rim", "Tom", "Hi-Hat", "Tom Hi", "Tom Lo", "Ride", "Crash"},
    {"Kick", "Rim", "Ghost", "HH Open", "Tom Hi", "Tom Lo", "Ride", "Crash"},
    {"Kick", "Snare", "Hi-Hat", "Tom", "Tom Hi", "Tom Lo", "Ride", "Crash"},
    {"Clave", "Conga", "Shaker", "Cowbell", "Tom Hi", "Tom Lo", "Ride", "Crash"},
    {"Kick", "Shaker", "Conga", "Djembe", "Perc", "Tom Lo", "Ride", "Crash"},
    {"Fixed", "Drifting", "Pulse", "HH Open", "Tom Hi", "Tom Lo", "Ride", "Crash"},
    {"Polos", "Sangsih", "Gong", "Shimmer", "Tom Hi", "Tom Lo", "Ride", "Crash"},
    {"Kick", "Snare", "Hi-Hat", "Ghost", "Tom Hi", "Tom Lo", "Ride", "Crash"},
    {"Bell", "Kick", "Snare", "Shaker", "Conga", "Tom Lo", "Ride", "Crash"},
    {"Davul", "Rim", "Zurna", "Darbuka", "Tom Hi", "Tom Lo", "Ride", "Crash"},
    {"Surdo", "Tamborim", "Agogo", "Pandeiro", "Tom Hi", "Tom Lo", "Ride", "Crash"},
    {"Mridangam Lo", "Mridangam Hi", "Ghatam", "Kanjira", "Tom Hi", "Tom Lo", "Ride", "Crash"},
    {"Kick", "Snare", "Hi-Hat", "Perc", "Glitch", "Tom Lo", "Ride", "Crash"},
};

static std::string laneToJson(const LaneConfig& cfg, const std::string& name, int laneIndex) {
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

static std::string grooveStateToJson(const GrooveState& gs, const SceneState& ss, PolyController* ctrl,
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
        js += laneToJson(gs.lanes[i], ctrl->laneName(i), i);
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

    js += "}}";
    return js;
}

// --- WebUIView implementation ---

WebUIView::WebUIView(PolyController* controller) : CPluginView(nullptr), controller_(controller) {
    Steinberg::ViewRect rect(0, 0, 1160, 760);
    setRect(rect);
}

WebUIView::~WebUIView() {
    stopFrameTimer();
}

Steinberg::tresult PLUGIN_API WebUIView::isPlatformTypeSupported(Steinberg::FIDString type) {
#if defined(__APPLE__)
    if (Steinberg::FIDStringsEqual(type, Steinberg::kPlatformTypeNSView))
        return Steinberg::kResultTrue;
#elif defined(_WIN32)
    if (Steinberg::FIDStringsEqual(type, Steinberg::kPlatformTypeHWND))
        return Steinberg::kResultTrue;
#endif
    return Steinberg::kResultFalse;
}

Steinberg::tresult PLUGIN_API WebUIView::attached(void* parent, Steinberg::FIDString type) {
    choc::ui::WebView::Options options;
    options.enableDebugMode = true;
    options.fetchResource = [](const std::string& path) -> std::optional<choc::ui::WebView::Options::Resource> {
        const auto* asset = webui_assets::lookup(path == "/" ? "/index.html" : path);
        if (!asset)
            return std::nullopt;
        choc::ui::WebView::Options::Resource res;
        res.data.assign(asset->data, asset->data + asset->size);
        res.mimeType = std::string(asset->mime);
        return res;
    };
    webview_ = std::make_unique<choc::ui::WebView>(options);

    webview_->addInitScript("window.__POLY_EMBEDDED__ = true;");
    webview_->bind("polyHostCall", [this](const choc::value::ValueView& args) -> choc::value::Value {
        if (args.isArray() && args.size() > 0)
            handleHostCall(std::string(args[0].getString()));
        return {};
    });
    // Reparent the webview child into the host-provided parent view.
    void* handle = webview_->getViewHandle();
    if (handle && parent) {
#if defined(__APPLE__)
        // NSView reparenting via objc_msgSend
        using AddSubviewFn = void (*)(id, SEL, id);
        auto addSubview = reinterpret_cast<AddSubviewFn>(objc_msgSend);
        addSubview(static_cast<id>(parent), sel_registerName("addSubview:"), static_cast<id>(handle));

        // Resize child to fill parent bounds
        resizeWebviewToRect(rect);
#elif defined(_WIN32)
        SetParent(static_cast<HWND>(handle), static_cast<HWND>(parent));
        RECT r;
        r.left = rect.left;
        r.top = rect.top;
        r.right = rect.right;
        r.bottom = rect.bottom;
        MoveWindow(static_cast<HWND>(handle), 0, 0, r.right - r.left, r.bottom - r.top, TRUE);
#endif
    }

    startFrameTimer();
    return CPluginView::attached(parent, type);
}

Steinberg::tresult PLUGIN_API WebUIView::removed() {
    stopFrameTimer();
    webview_.reset();
    return CPluginView::removed();
}

Steinberg::tresult PLUGIN_API WebUIView::onSize(Steinberg::ViewRect* newSize) {
    if (newSize) {
        resizeWebviewToRect(*newSize);
    }
    return CPluginView::onSize(newSize);
}

void WebUIView::resizeWebviewToRect(const Steinberg::ViewRect& r) {
    if (!webview_)
        return;
    void* handle = webview_->getViewHandle();
    if (!handle)
        return;

    int w = r.right - r.left;
    int h = r.bottom - r.top;

#if defined(__APPLE__)
    // Set NSView frame via objc_msgSend. CGRect is {origin.x, origin.y, size.w, size.h}.
    struct CGRect {
        double x, y, w, h;
    };
    CGRect frame{0.0, 0.0, static_cast<double>(w), static_cast<double>(h)};
    using SetFrameFn = void (*)(id, SEL, CGRect);
    auto setFrame = reinterpret_cast<SetFrameFn>(objc_msgSend);
    setFrame(static_cast<id>(handle), sel_registerName("setFrame:"), frame);
#elif defined(_WIN32)
    MoveWindow(static_cast<HWND>(handle), 0, 0, w, h, TRUE);
#endif
}

void WebUIView::handleHostCall(const std::string& json) {
    try {
        auto msg = choc::json::parse(json);

        auto typeStr = msg["type"].toString();

        if (typeStr == "ready") {
            pushState();
            return;
        }

        if (typeStr == "edit") {
            auto paramId = msg["paramId"].toString();
            auto value = msg["value"].getFloat64();
            auto gesture = msg["gesture"].toString();

            auto id = webui::resolveParamId(paramId.c_str());
            if (!id.has_value())
                return;

            if (gesture == "begin") {
                controller_->beginEdit(*id);
            } else if (gesture == "perform") {
                controller_->setParamNormalized(*id, value);
                controller_->performEdit(*id, value);
            } else if (gesture == "end") {
                controller_->endEdit(*id);
            }
            return;
        }

        if (typeStr == "action") {
            auto name = msg["name"].toString();
            auto payload = msg["payload"];
            handleAction(name, payload);
            editCooldown_ = 6;
            pushState();
            return;
        }

    } catch (...) {
        // Malformed JSON — drop silently per bridge-schema invariant
    }
}

void WebUIView::handleAction(const std::string& name, const choc::value::ValueView& payload) {
    auto& scene = controller_->mutableActiveScene();

    if (name == "toggleStep") {
        int lane = payload["lane"].getInt32();
        int step = payload["step"].getInt32();
        if (lane < 0 || lane >= kMaxLanes || step < 0 || step >= kMaxSteps)
            return;
        auto& cfg = scene.lanes[lane];
        if (cfg.timeline) {
            cfg.fixedPattern[step] = !cfg.fixedPattern[step];
            controller_->sendTimelinePattern(lane);
        }
        return;
    }

    if (name == "setEuclid") {
        int lane = payload["lane"].getInt32();
        if (lane < 0 || lane >= kMaxLanes)
            return;
        auto& cfg = scene.lanes[lane];
        if (payload.hasObjectMember("steps"))
            cfg.cycle.steps = std::clamp(payload["steps"].getInt32(), 1, kMaxSteps);
        if (payload.hasObjectMember("hits"))
            cfg.hitCount = std::clamp(payload["hits"].getInt32(), 0, cfg.cycle.steps);
        if (payload.hasObjectMember("rotation"))
            cfg.rotation = ((payload["rotation"].getInt32() % cfg.cycle.steps) + cfg.cycle.steps) % cfg.cycle.steps;

        controller_->setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreSteps),
                                        (cfg.cycle.steps - 1) / 63.0);
        controller_->performEdit(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreSteps), (cfg.cycle.steps - 1) / 63.0);
        controller_->setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreHits), cfg.hitCount / 64.0);
        controller_->performEdit(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreHits), cfg.hitCount / 64.0);
        controller_->setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreRotation), cfg.rotation / 63.0);
        controller_->performEdit(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreRotation), cfg.rotation / 63.0);
        return;
    }

    if (name == "setCells") {
        int lane = payload["lane"].getInt32();
        if (lane < 0 || lane >= kMaxLanes)
            return;
        auto& cfg = scene.lanes[lane];
        if (payload.hasObjectMember("cells") && !payload["cells"].isVoid()) {
            auto cells = payload["cells"];
            cfg.cellCount = std::min(static_cast<int>(cells.size()), kMaxSteps);
            for (int i = 0; i < cfg.cellCount; ++i)
                cfg.cellSizes[i] = std::clamp(cells[static_cast<uint32_t>(i)].getInt32(), 1, 16);
        } else {
            cfg.cellCount = 0;
        }
        controller_->setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreCellCount), cfg.cellCount / 64.0);
        controller_->performEdit(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreCellCount), cfg.cellCount / 64.0);
        controller_->sendCellSizes(lane);
        return;
    }

    if (name == "setFixedStep") {
        int lane = payload["lane"].getInt32();
        int step = payload["step"].getInt32();
        bool on = payload["on"].getBool();
        if (lane < 0 || lane >= kMaxLanes || step < 0 || step >= kMaxSteps)
            return;
        scene.lanes[lane].fixedPattern[step] = on;
        controller_->sendTimelinePattern(lane);
        return;
    }

    if (name == "setMicroTiming") {
        int lane = payload["lane"].getInt32();
        int step = payload["step"].getInt32();
        if (lane < 0 || lane >= kMaxLanes || step < 0 || step >= kMaxSteps)
            return;
        auto ms = static_cast<float>(payload["ms"].getFloat64());
        scene.lanes[lane].microTimingMs[step] = std::clamp(ms, -20.0f, 20.0f);
        controller_->sendMicroTiming(lane);
        return;
    }

    if (name == "setEnvelope") {
        int lane = payload["lane"].getInt32();
        int index = payload["index"].getInt32();
        if (lane < 0 || lane >= kMaxLanes || index < 0 || index >= kMaxEnvelopesPerLane)
            return;
        auto& ea = scene.lanes[lane].envelopes[index];
        if (payload.hasObjectMember("envelope") && !payload["envelope"].isVoid()) {
            auto env = payload["envelope"];
            if (env.hasObjectMember("target")) {
                auto t = env["target"].toString();
                if (t == "Velocity")
                    ea.envelope.target = EnvTarget::Velocity;
                else if (t == "Density")
                    ea.envelope.target = EnvTarget::Density;
                else if (t == "Probability")
                    ea.envelope.target = EnvTarget::Probability;
            }
            if (env.hasObjectMember("period"))
                ea.envelope.periodBars = static_cast<float>(env["period"].getFloat64());
            if (env.hasObjectMember("depth"))
                ea.envelope.depth = static_cast<float>(env["depth"].getFloat64());
            if (env.hasObjectMember("on"))
                ea.active = env["on"].getBool();
        } else {
            ea = EnvelopeAssign{};
            ea.active = false;
        }
        controller_->sendEnvelopeUpdate(lane, index);
        return;
    }

    if (name == "selectScene") {
        auto sceneStr = payload["scene"].toString();
        double val = 0.0;
        if (sceneStr == "B")
            val = 0.5;
        else if (sceneStr == "Morph")
            val = 1.0;
        controller_->beginEdit(ParamIDs::kSceneSelect);
        controller_->setParamNormalized(ParamIDs::kSceneSelect, val);
        controller_->performEdit(ParamIDs::kSceneSelect, val);
        controller_->endEdit(ParamIDs::kSceneSelect);
        return;
    }

    if (name == "applyPreset") {
        int index = payload["index"].getInt32();

        auto pushParam = [this](Steinberg::Vst::ParamID id, double value) {
            controller_->beginEdit(id);
            controller_->setParamNormalized(id, value);
            controller_->performEdit(id, value);
            controller_->endEdit(id);
        };

        if (index == -1) {
            currentPresetName_ = "Init";
            GrooveState init{};
            init.activeLaneCount = kMaxLanes;
            static constexpr int kInitSteps[] = {4, 4, 8, 5, 7, 3, 6, 9};
            static constexpr int kInitSubs[] = {4, 4, 8, 16, 8, 16, 16, 16};
            static constexpr int kInitHits[] = {4, 2, 8, 3, 4, 2, 4, 5};
            static constexpr int kInitNotes[] = {36, 38, 42, 45, 46, 39, 43, 50};
            for (int lane = 0; lane < kMaxLanes; ++lane) {
                init.lanes[lane].id = lane;
                init.lanes[lane].cycle = {kInitSteps[lane], kInitSubs[lane]};
                init.lanes[lane].hitCount = kInitHits[lane];
                init.lanes[lane].midiNote = kInitNotes[lane];
                init.lanes[lane].baseVelocity = 100;
                init.lanes[lane].probability = 1.0f;
            }
            controller_->mutableActiveScene() = init;
            controller_->resetLaneNames();
            for (int lane = 0; lane < kMaxLanes; ++lane) {
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kActive), 1.0);
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreSteps), (kInitSteps[lane] - 1) / 63.0);
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreHits), kInitHits[lane] / 64.0);
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreMidiNote), kInitNotes[lane] / 127.0);
            }
            pushParam(ParamIDs::kActiveLaneCount, (kMaxLanes - 1) / 7.0);
            pushParam(ParamIDs::kSeed, 0.0);
        } else if (index >= 0 && index < kFactoryPresetCount) {
            auto state = makeFactoryPreset(index);
            currentPresetName_ = getFactoryPresetInfo(index).name;
            controller_->mutableActiveScene() = state;
            for (int lane = 0; lane < kMaxLanes; ++lane)
                controller_->setLaneName(lane, kWebPresetLaneNames[index][lane]);

            for (int lane = 0; lane < kMaxLanes; ++lane) {
                const auto& cfg = state.lanes[lane];
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kProbability), cfg.probability);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kBaseVelocity), cfg.baseVelocity / 127.0);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kEmphasisProb), cfg.emphasisProb);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kGhostFloor), cfg.ghostFloor / 127.0);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kVelocitySpread), cfg.velocitySpread);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kSwingAmount), cfg.swingAmount);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kHumanizeMs), cfg.humanizeMs / 50.0);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kNoteDuration), cfg.noteDuration / 4.0);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kActive), (lane < state.activeLaneCount) ? 1.0 : 0.0);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kPhraseLength), cfg.phraseLength / 64.0);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kPhraseGap), cfg.phraseGap / 64.0);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kPhraseOffset), cfg.phraseOffset / 64.0);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kMutationRate), cfg.mutationRate);
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kDriftRate),
                          static_cast<double>((cfg.driftRate + 4.0f) / 8.0f));
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kTimingOffset),
                          static_cast<double>((cfg.timingOffsetMs + 20.0f) / 40.0f));
                pushParam(ParamIDs::laneParam(lane, ParamIDs::kKotekanSource),
                          static_cast<double>(cfg.kotekanSourceLane + 1) / 8.0);

                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreSteps), (cfg.cycle.steps - 1) / 63.0);
                int subIdx = 0;
                switch (cfg.cycle.subdivision) {
                case 1:
                    subIdx = 0;
                    break;
                case 2:
                    subIdx = 1;
                    break;
                case 4:
                    subIdx = 2;
                    break;
                case 8:
                    subIdx = 3;
                    break;
                case 16:
                    subIdx = 4;
                    break;
                default:
                    subIdx = 2;
                    break;
                }
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreSubdivision), subIdx / 4.0);
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreHits), cfg.hitCount / 64.0);
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreRotation), cfg.rotation / 63.0);
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreMidiNote), cfg.midiNote / 127.0);
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreCellCount), cfg.cellCount / 64.0);
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreTimeline), cfg.timeline ? 1.0 : 0.0);
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreFixedPatternLen), cfg.fixedPatternLength / 64.0);
            }

            pushParam(ParamIDs::kMacroComplexity, state.macros.complexity);
            pushParam(ParamIDs::kMacroDensity, state.macros.density);
            pushParam(ParamIDs::kMacroSyncopation, state.macros.syncopation);
            pushParam(ParamIDs::kMacroSwing, state.macros.swing);
            pushParam(ParamIDs::kMacroTension, state.macros.tension);
            pushParam(ParamIDs::kMacroHumanize, state.macros.humanize);

            pushParam(ParamIDs::kActiveLaneCount, (state.activeLaneCount - 1) / 7.0);
            pushParam(ParamIDs::kSeed, state.seed / 999999.0);
        }
        return;
    }

    if (name == "exportRequest") {
        controller_->beginEdit(ParamIDs::kExportTrigger);
        controller_->setParamNormalized(ParamIDs::kExportTrigger, 1.0);
        controller_->performEdit(ParamIDs::kExportTrigger, 1.0);
        controller_->endEdit(ParamIDs::kExportTrigger);
        return;
    }
}

void WebUIView::pushState() {
    if (!webview_ || !controller_)
        return;

    const auto& ss = controller_->cachedState();
    const auto& gs = controller_->activeScene();
    std::string json = grooveStateToJson(gs, ss, controller_, currentPresetName_);
    if (json == lastPushedJson_)
        return;
    lastPushedJson_ = json;
    webview_->evaluateJavascript("window.polyHostPush(" + json + ")");
}

void WebUIView::pushFrame() {
    if (!webview_ || !controller_)
        return;

    auto* snap = controller_->uiSnapshot();

    if (editCooldown_ > 0) {
        --editCooldown_;
        if (snap && snap->stateReady.load(std::memory_order_acquire))
            snap->stateReady.store(false, std::memory_order_release);
    } else if (snap && snap->stateReady.load(std::memory_order_acquire)) {
        controller_->mutableCachedState() = snap->state;
        snap->stateReady.store(false, std::memory_order_release);
        pushState();
    }

    // Check for host-side state changes (preset load via setComponentState)
    uint32_t gen = controller_->stateGeneration();
    if (gen != lastStateGen_) {
        lastStateGen_ = gen;
        pushState();
    }

    // Read transport from per-instance snapshot (no globals)
    double ppqNorm = snap ? snap->ppqNorm.load(std::memory_order_relaxed) : 0.0;
    bool playing = snap ? snap->playing.load(std::memory_order_relaxed) : false;

    double t8 = ppqNorm * 256.0;

    constexpr int kConvWindow = 120;
    int convLeft = kConvWindow;
    if (playing) {
        int t8i = static_cast<int>(std::floor(t8));
        convLeft = (kConvWindow - (t8i % kConvWindow)) % kConvWindow;
        if (convLeft == 0)
            convLeft = kConvWindow;
    }

    std::string js;
    js.reserve(512);
    js += "{\"type\":\"frame\",\"frame\":{";

    char buf[64];
    std::snprintf(buf, sizeof(buf), "\"t8\":%.4f,\"playing\":%s,\"convLeft\":%d", t8, playing ? "true" : "false",
                  convLeft);
    js += buf;

    const auto& gs = controller_->activeScene();
    js += ",\"lanes\":[";
    for (int i = 0; i < gs.activeLaneCount; ++i) {
        if (i > 0)
            js += ',';
        double phase = snap ? snap->lanePhases[i].load(std::memory_order_relaxed) : 0.0;
        int step = static_cast<int>(phase * gs.lanes[i].cycle.steps) % gs.lanes[i].cycle.steps;
        std::snprintf(buf, sizeof(buf), "{\"ph\":%.4f,\"step\":%d}", phase, step);
        js += buf;
    }
    js += "]}}";

    webview_->evaluateJavascript("window.polyHostPush(" + js + ")");
}

void WebUIView::startFrameTimer() {
    if (frameTimer_)
        return;
    frameTimer_ = choc::messageloop::Timer(33, [this] {
        pushFrame();
        return true;
    });
}

void WebUIView::stopFrameTimer() {
    frameTimer_.reset();
}

} // namespace poly
