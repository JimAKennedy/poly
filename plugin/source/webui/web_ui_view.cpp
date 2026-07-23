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
#include "bridge_serialization.h"
#include "choc/gui/choc_MessageLoop.h"
#include "choc/gui/choc_WebView.h"
#include "choc/text/choc_JSON.h"
#include "poly/euclidean.h"
#include "poly/params_def.h"
#include "poly/presets.h"
#include "poly_webui_assets.h" // generated: jk_embed_assets(webui/*)

#ifdef __APPLE__
#include <objc/message.h>
#include <objc/runtime.h>
#endif

namespace poly {

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

// --- WebUIView implementation ---

WebUIView::WebUIView(PolyController* controller) : CPluginView(nullptr), controller_(controller) {
    Steinberg::ViewRect rect(0, 0, 1160, 760);
    setRect(rect);
    if (currentPresetName_.empty())
        currentPresetName_ = "Init";
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
        if (path == "/" || path == "/index.html") {
            static const std::string kEmbedTag = "<head>";
            static const std::string kInject = "<head><script>window.__POLY_EMBEDDED__=true;</script>";
            auto html = std::string(res.data.begin(), res.data.end());
            auto pos = html.find(kEmbedTag);
            if (pos != std::string::npos)
                html.replace(pos, kEmbedTag.size(), kInject);
            res.data.assign(html.begin(), html.end());
        }
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
            webviewReady_ = true;
            lastPushedJson_.clear();
            pushState();
            return;
        }

        if (typeStr == "edit") {
            auto paramId = msg["paramId"].toString();
            auto value = msg["value"].get<double>();
            auto gesture = msg["gesture"].toString();

            auto id = webui::resolveParamId(paramId.c_str());
            if (!id.has_value())
                return;

            if (gesture == "begin") {
                controller_->beginEdit(*id);
            } else if (gesture == "perform") {
                controller_->setParamNormalized(*id, value);
                controller_->performEdit(*id, value);
                applyEditToCache(*id, value);
                editCooldown_ = 20;
                pushState();
            } else if (gesture == "end") {
                controller_->endEdit(*id);
                applyEditToCache(*id, value);
                editCooldown_ = 20;
                pushState();
            }
            return;
        }

        if (typeStr == "action") {
            auto name = msg["name"].toString();
            auto payload = msg["payload"];
            handleAction(name, payload);
            editCooldown_ = 20;
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
        int lane = payload["lane"].get<int32_t>();
        int step = payload["step"].get<int32_t>();
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
        int lane = payload["lane"].get<int32_t>();
        if (lane < 0 || lane >= kMaxLanes)
            return;
        auto& cfg = scene.lanes[lane];
        if (payload.hasObjectMember("steps"))
            cfg.cycle.steps = std::clamp(payload["steps"].get<int32_t>(), 1, kMaxSteps);
        if (payload.hasObjectMember("hits"))
            cfg.hitCount = std::clamp(payload["hits"].get<int32_t>(), 0, cfg.cycle.steps);
        if (payload.hasObjectMember("rotation"))
            cfg.rotation = ((payload["rotation"].get<int32_t>() % cfg.cycle.steps) + cfg.cycle.steps) % cfg.cycle.steps;

        auto stepsId = ParamIDs::laneCoreParam(lane, ParamIDs::kCoreSteps);
        auto hitsId = ParamIDs::laneCoreParam(lane, ParamIDs::kCoreHits);
        auto rotId = ParamIDs::laneCoreParam(lane, ParamIDs::kCoreRotation);
        const double stepsNorm = params::engineToNormCore(ParamIDs::kCoreSteps, cfg.cycle.steps);
        const double hitsNorm = params::engineToNormCore(ParamIDs::kCoreHits, cfg.hitCount);
        const double rotNorm = params::engineToNormCore(ParamIDs::kCoreRotation, cfg.rotation);
        controller_->beginEdit(stepsId);
        controller_->beginEdit(hitsId);
        controller_->beginEdit(rotId);
        controller_->setParamNormalized(stepsId, stepsNorm);
        controller_->performEdit(stepsId, stepsNorm);
        controller_->setParamNormalized(hitsId, hitsNorm);
        controller_->performEdit(hitsId, hitsNorm);
        controller_->setParamNormalized(rotId, rotNorm);
        controller_->performEdit(rotId, rotNorm);
        controller_->endEdit(stepsId);
        controller_->endEdit(hitsId);
        controller_->endEdit(rotId);
        return;
    }

    if (name == "setCells") {
        int lane = payload["lane"].get<int32_t>();
        if (lane < 0 || lane >= kMaxLanes)
            return;
        auto& cfg = scene.lanes[lane];
        if (payload.hasObjectMember("cells") && !payload["cells"].isVoid()) {
            auto cells = payload["cells"];
            cfg.cellCount = std::min(static_cast<int>(cells.size()), kMaxSteps);
            for (int i = 0; i < cfg.cellCount; ++i)
                cfg.cellSizes[i] = std::clamp(cells[static_cast<uint32_t>(i)].get<int32_t>(), 1, 16);
        } else {
            cfg.cellCount = 0;
        }
        auto cellId = ParamIDs::laneCoreParam(lane, ParamIDs::kCoreCellCount);
        const double cellNorm = params::engineToNormCore(ParamIDs::kCoreCellCount, cfg.cellCount);
        controller_->beginEdit(cellId);
        controller_->setParamNormalized(cellId, cellNorm);
        controller_->performEdit(cellId, cellNorm);
        controller_->endEdit(cellId);
        controller_->sendCellSizes(lane);
        return;
    }

    if (name == "setFixedStep") {
        int lane = payload["lane"].get<int32_t>();
        int step = payload["step"].get<int32_t>();
        bool on = payload["on"].getBool();
        if (lane < 0 || lane >= kMaxLanes || step < 0 || step >= kMaxSteps)
            return;
        scene.lanes[lane].fixedPattern[step] = on;
        controller_->sendTimelinePattern(lane);
        return;
    }

    if (name == "setMicroTiming") {
        int lane = payload["lane"].get<int32_t>();
        int step = payload["step"].get<int32_t>();
        if (lane < 0 || lane >= kMaxLanes || step < 0 || step >= kMaxSteps)
            return;
        auto ms = static_cast<float>(payload["ms"].get<double>());
        scene.lanes[lane].microTimingMs[step] = std::clamp(ms, -20.0f, 20.0f);
        controller_->sendMicroTiming(lane);
        return;
    }

    if (name == "setEnvelope") {
        int lane = payload["lane"].get<int32_t>();
        int index = payload["index"].get<int32_t>();
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
                ea.envelope.periodBars = static_cast<float>(env["period"].get<double>());
            if (env.hasObjectMember("depth"))
                ea.envelope.depth = static_cast<float>(env["depth"].get<double>());
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
        SceneSelect newSel = SceneSelect::A;
        if (sceneStr == "B") {
            val = 0.5;
            newSel = SceneSelect::B;
        } else if (sceneStr == "Morph") {
            val = 1.0;
            newSel = SceneSelect::Morph;
        }
        controller_->mutableCachedState().select = newSel;
        controller_->beginEdit(ParamIDs::kSceneSelect);
        controller_->setParamNormalized(ParamIDs::kSceneSelect, val);
        controller_->performEdit(ParamIDs::kSceneSelect, val);
        controller_->endEdit(ParamIDs::kSceneSelect);
        return;
    }

    if (name == "applyPreset") {
        int index = payload["index"].get<int32_t>();

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
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreSteps),
                          params::engineToNormCore(ParamIDs::kCoreSteps, kInitSteps[lane]));
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreHits),
                          params::engineToNormCore(ParamIDs::kCoreHits, kInitHits[lane]));
                pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreMidiNote),
                          params::engineToNormCore(ParamIDs::kCoreMidiNote, kInitNotes[lane]));
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
                auto expr = [&](int offset, double engine) {
                    pushParam(ParamIDs::laneParam(lane, offset),
                              params::engineToNormExpr(static_cast<uint32_t>(offset), engine));
                };
                auto core = [&](int offset, double engine) {
                    pushParam(ParamIDs::laneCoreParam(lane, offset),
                              params::engineToNormCore(static_cast<uint32_t>(offset), engine));
                };
                expr(ParamIDs::kProbability, cfg.probability);
                expr(ParamIDs::kBaseVelocity, cfg.baseVelocity);
                expr(ParamIDs::kEmphasisProb, cfg.emphasisProb);
                expr(ParamIDs::kGhostFloor, cfg.ghostFloor);
                expr(ParamIDs::kVelocitySpread, cfg.velocitySpread);
                expr(ParamIDs::kSwingAmount, cfg.swingAmount);
                expr(ParamIDs::kHumanizeMs, cfg.humanizeMs);
                expr(ParamIDs::kNoteDuration, cfg.noteDuration);
                expr(ParamIDs::kActive, (lane < state.activeLaneCount) ? 1.0 : 0.0);
                expr(ParamIDs::kPhraseLength, cfg.phraseLength);
                expr(ParamIDs::kPhraseGap, cfg.phraseGap);
                expr(ParamIDs::kPhraseOffset, cfg.phraseOffset);
                expr(ParamIDs::kMutationRate, cfg.mutationRate);
                expr(ParamIDs::kDriftRate, cfg.driftRate);
                expr(ParamIDs::kTimingOffset, cfg.timingOffsetMs);
                expr(ParamIDs::kKotekanSource, cfg.kotekanSourceLane);

                core(ParamIDs::kCoreSteps, cfg.cycle.steps);
                core(ParamIDs::kCoreSubdivision, cfg.cycle.subdivision);
                core(ParamIDs::kCoreHits, cfg.hitCount);
                core(ParamIDs::kCoreRotation, cfg.rotation);
                core(ParamIDs::kCoreMidiNote, cfg.midiNote);
                core(ParamIDs::kCoreCellCount, cfg.cellCount);
                core(ParamIDs::kCoreTimeline, cfg.timeline ? 1.0 : 0.0);
                core(ParamIDs::kCoreFixedPatternLen, cfg.fixedPatternLength);
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
        for (int lane = 0; lane < kMaxLanes; ++lane) {
            controller_->sendCellSizes(lane);
            controller_->sendTimelinePattern(lane);
            controller_->sendMicroTiming(lane);
            controller_->sendAccentMask(lane);
            const auto& cfg = controller_->activeScene().lanes[lane];
            for (int ei = 0; ei < cfg.envelopeCount && ei < kMaxEnvelopesPerLane; ++ei)
                controller_->sendEnvelopeUpdate(lane, ei);
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

    if (name == "setAccent") {
        int lane = payload["lane"].get<int32_t>();
        int step = payload["step"].get<int32_t>();
        if (lane < 0 || lane >= kMaxLanes || step < 0 || step >= kMaxSteps)
            return;
        scene.lanes[lane].accents.steps[step] = static_cast<float>(payload["value"].get<double>());
        controller_->sendAccentMask(lane);
        return;
    }

    if (name == "chainAddEntry") {
        auto& chain = controller_->mutableCachedState().chain;
        if (chain.entryCount < kMaxChainEntries) {
            chain.entries[chain.entryCount] = {SceneSelect::A, 4};
            chain.entryCount++;
            double norm = static_cast<double>(chain.entryCount) / static_cast<double>(kMaxChainEntries);
            controller_->beginEdit(ParamIDs::kChainEntryCount);
            controller_->setParamNormalized(ParamIDs::kChainEntryCount, norm);
            controller_->performEdit(ParamIDs::kChainEntryCount, norm);
            controller_->endEdit(ParamIDs::kChainEntryCount);
        }
        return;
    }

    if (name == "chainRemoveEntry") {
        int index = payload["index"].get<int32_t>();
        auto& chain = controller_->mutableCachedState().chain;
        if (index >= 0 && index < chain.entryCount) {
            for (int i = index; i < chain.entryCount - 1; ++i)
                chain.entries[i] = chain.entries[i + 1];
            chain.entryCount--;
            double norm = static_cast<double>(chain.entryCount) / static_cast<double>(kMaxChainEntries);
            controller_->beginEdit(ParamIDs::kChainEntryCount);
            controller_->setParamNormalized(ParamIDs::kChainEntryCount, norm);
            controller_->performEdit(ParamIDs::kChainEntryCount, norm);
            controller_->endEdit(ParamIDs::kChainEntryCount);
        }
        return;
    }

    if (name == "resetNoteMap") {
        auto& nm = controller_->mutableCachedState().noteMap;
        for (int i = 0; i < 128; ++i)
            nm.map[i] = static_cast<int16_t>(i);
        controller_->sendNoteMap();
        return;
    }

    if (name == "setNoteMap") {
        int note = payload["note"].get<int32_t>();
        int output = payload["output"].get<int32_t>();
        if (note < 0 || note > 127 || output < 0 || output > 127)
            return;
        controller_->mutableCachedState().noteMap.map[note] = static_cast<int16_t>(output);
        controller_->sendNoteMap();
        return;
    }
}

void WebUIView::applyEditToCache(Steinberg::Vst::ParamID id, double normalized) {
    using namespace ParamIDs;
    auto& ss = controller_->mutableCachedState();
    auto& gs = controller_->mutableActiveScene();

    if (id == kSceneSelect) {
        int sel = static_cast<int>(std::round(normalized * 2.0));
        ss.select = static_cast<SceneSelect>(std::clamp(sel, 0, 2));
        return;
    }
    if (id == kSceneMorph) {
        ss.morphAmount = static_cast<float>(normalized);
        return;
    }
    if (id == kChainEnabled) {
        ss.chain.enabled = (normalized > 0.5);
        return;
    }
    if (id == kChainMode) {
        int m = static_cast<int>(std::round(normalized * 2.0));
        ss.chain.mode = static_cast<ChainMode>(std::clamp(m, 0, 2));
        return;
    }
    if (id == kChainEntryCount) {
        ss.chain.entryCount = static_cast<int>(std::round(normalized * static_cast<double>(kMaxChainEntries)));
        return;
    }
    if (id >= kChainEntryBase &&
        id < kChainEntryBase + static_cast<Steinberg::Vst::ParamID>(kMaxChainEntries * kChainParamsPerEntry)) {
        auto rel = static_cast<int>(id - kChainEntryBase);
        int entry = rel / kChainParamsPerEntry;
        int offset = rel % kChainParamsPerEntry;
        if (entry < kMaxChainEntries) {
            auto& e = ss.chain.entries[static_cast<size_t>(entry)];
            if (offset == kChainEntryScene) {
                int sel = static_cast<int>(std::round(normalized * 2.0));
                e.scene = static_cast<SceneSelect>(std::clamp(sel, 0, 2));
            } else if (offset == kChainEntryBars) {
                e.bars = 1 + static_cast<int>(std::round(normalized * 31.0));
            }
        }
        return;
    }

    if (id >= kLaneCoreBase &&
        id < kLaneCoreBase + static_cast<Steinberg::Vst::ParamID>(kMaxLanes * kCoreParamsPerLane)) {
        auto rel = static_cast<int>(id - kLaneCoreBase);
        int lane = rel / kCoreParamsPerLane;
        int offset = rel % kCoreParamsPerLane;
        auto& cfg = gs.lanes[lane];
        const double engineValue = params::normToEngineCore(static_cast<uint32_t>(offset), normalized);
        switch (offset) {
        case kCoreSteps:
            cfg.cycle.steps = static_cast<int>(engineValue);
            break;
        case kCoreSubdivision:
            cfg.cycle.subdivision = static_cast<int>(engineValue);
            break;
        case kCoreHits:
            cfg.hitCount = static_cast<int>(engineValue);
            break;
        case kCoreRotation:
            cfg.rotation = static_cast<int>(engineValue);
            break;
        case kCoreMidiNote:
            cfg.midiNote = static_cast<int16_t>(engineValue);
            break;
        case kCoreCellCount:
            cfg.cellCount = static_cast<int>(engineValue);
            break;
        case kCoreTimeline: {
            const bool next = engineValue > 0.5;
            // On the false→true edge, seed fixedPattern[] from the current
            // Euclidean state so the user's timeline-mode step grid starts
            // populated with the same hits the lane was already playing.
            // Enables the "Euclidean approximation → adjust one step" workflow
            // documented in chapter 3 for son clave / rumba clave.
            // On true→false edge, leave fixedPattern intact so re-enabling
            // timeline mode restores any manual edits.
            // UI-thread only (applyEditToCache); no RT constraint.
            if (next && !cfg.timeline) {
                poly::euclidean(cfg.hitCount, cfg.cycle.steps, cfg.rotation, cfg.fixedPattern);
                if (cfg.fixedPatternLength == 0) {
                    cfg.fixedPatternLength = cfg.cycle.steps;
                }
            }
            cfg.timeline = next;
            break;
        }
        case kCoreFixedPatternLen:
            cfg.fixedPatternLength = static_cast<int>(engineValue);
            break;
        case kCoreTempoMult:
            cfg.tempoMultiplier = static_cast<float>(engineValue);
            break;
        case kCoreMidiChannel:
            cfg.midiChannel = static_cast<int16_t>(engineValue);
            break;
        default:
            break;
        }
        return;
    }

    if (id < static_cast<Steinberg::Vst::ParamID>(kMaxLanes * kParamsPerLane)) {
        int lane = static_cast<int>(id) / kParamsPerLane;
        int offset = static_cast<int>(id) % kParamsPerLane;
        auto& cfg = gs.lanes[lane];
        const double engineValue = params::normToEngineExpr(static_cast<uint32_t>(offset), normalized);
        switch (offset) {
        case kProbability:
            cfg.probability = static_cast<float>(engineValue);
            break;
        case kBaseVelocity:
            cfg.baseVelocity = static_cast<uint8_t>(engineValue);
            break;
        case kEmphasisProb:
            cfg.emphasisProb = static_cast<float>(engineValue);
            break;
        case kGhostFloor:
            cfg.ghostFloor = static_cast<uint8_t>(engineValue);
            break;
        case kVelocitySpread:
            cfg.velocitySpread = static_cast<float>(engineValue);
            break;
        case kSwingAmount:
            cfg.swingAmount = static_cast<float>(engineValue);
            break;
        case kHumanizeMs:
            cfg.humanizeMs = static_cast<float>(engineValue);
            break;
        case kNoteDuration:
            cfg.noteDuration = static_cast<float>(engineValue);
            break;
        case kActive:
            cfg.active = (engineValue > 0.5);
            break;
        case kPhraseLength:
            cfg.phraseLength = static_cast<float>(engineValue);
            break;
        case kPhraseGap:
            cfg.phraseGap = static_cast<float>(engineValue);
            break;
        case kPhraseOffset:
            cfg.phraseOffset = static_cast<float>(engineValue);
            break;
        case kMutationRate:
            cfg.mutationRate = static_cast<float>(engineValue);
            break;
        case kDriftRate:
            cfg.driftRate = static_cast<float>(engineValue);
            break;
        case kTimingOffset:
            cfg.timingOffsetMs = static_cast<float>(engineValue);
            break;
        case kKotekanSource:
            cfg.kotekanSourceLane = static_cast<int>(engineValue);
            break;
        default:
            break;
        }
        return;
    }

    switch (id) {
    case kMacroComplexity:
        gs.macros.complexity = static_cast<float>(normalized);
        break;
    case kMacroDensity:
        gs.macros.density = static_cast<float>(normalized);
        break;
    case kMacroSyncopation:
        gs.macros.syncopation = static_cast<float>(normalized);
        break;
    case kMacroSwing:
        gs.macros.swing = static_cast<float>(normalized);
        break;
    case kMacroTension:
        gs.macros.tension = static_cast<float>(normalized);
        break;
    case kMacroHumanize:
        gs.macros.humanize = static_cast<float>(normalized);
        break;
    case kActiveLaneCount:
        gs.activeLaneCount = 1 + static_cast<int>(std::round(normalized * 7.0));
        break;
    case kSeed:
        gs.seed = static_cast<uint64_t>(std::round(normalized * 999999.0));
        break;
    default:
        break;
    }
}

void WebUIView::pushState() {
    if (!webview_ || !controller_)
        return;

    const auto& ss = controller_->cachedState();
    const auto& gs = controller_->activeScene();
    auto nameFn = [](int lane, void* ctx) -> const std::string& {
        return static_cast<PolyController*>(ctx)->laneName(lane);
    };
    std::string json = grooveStateToJson(gs, ss, nameFn, controller_, currentPresetName_);
    if (json == lastPushedJson_)
        return;
    lastPushedJson_ = json;
    webview_->evaluateJavascript("window.polyHostPush(" + json + ")");
}

void WebUIView::pushFrame() {
    if (!webview_ || !controller_)
        return;

    if (!webviewReady_)
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
