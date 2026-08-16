// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
// Web UI view — hosts the webui/ bundle in a choc::ui::WebView and
// bridges to the controller per webui/bridge-schema.md. Compiled only on
// the choc-webview platforms (defined(__APPLE__) || defined(_WIN32)).

#include "web_ui_view.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>

#include "../controller.h"
#include "../plugids.h"
#include "bridge_params.h"
#include "bridge_serialization.h"
#include "choc/gui/choc_MessageLoop.h"
#include "choc/gui/choc_WebView.h"
#include "choc/text/choc_JSON.h"
#include "platform_drag_source.h"
#include "platform_save_dialog.h"
#include "poly/euclidean.h"
#include "poly/offline_render.h"
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

// The table is intentionally sparse: only presets with bespoke lane labels
// carry a row; the rest (index >= the initialised rows) zero-fill to null and
// fall back to default lane names at the applyPreset call site. The null-guard
// there is load-bearing — do NOT setLaneName() a raw kWebPresetLaneNames entry
// without checking it first. The declared extent stays pinned to the preset
// count so index arithmetic can never read past the array.
static_assert(sizeof(kWebPresetLaneNames) / sizeof(kWebPresetLaneNames[0]) == kFactoryPresetCount,
              "kWebPresetLaneNames must be dimensioned to kFactoryPresetCount");

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
    parentView_ = parent;
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
    // On Windows, WebView2 initialises asynchronously — addInitScript/bind
    // fail if called before the core is ready.  Defer them to webviewIsReady
    // which fires synchronously on macOS/Linux and after the async callback
    // on Windows.
    options.webviewIsReady = [this](choc::ui::WebView& wv) {
        wv.addInitScript("window.__POLY_EMBEDDED__ = true;");
        wv.bind("polyHostCall", [this](const choc::value::ValueView& args) -> choc::value::Value {
            if (args.isArray() && args.size() > 0)
                handleHostCall(std::string(args[0].getString()));
            return {};
        });
    };
    webview_ = std::make_unique<choc::ui::WebView>(options);
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
        // choc creates the wrapper HWND as WS_POPUP; switch to WS_CHILD
        // before reparenting so it clips to the host window and renders
        // correctly inside the DAW (mirrors choc DesktopWindow::setContent).
        auto child = static_cast<HWND>(handle);
        auto flags = GetWindowLongPtr(child, GWL_STYLE);
        flags = (flags & ~static_cast<decltype(flags)>(WS_POPUP)) | static_cast<decltype(flags)>(WS_CHILD);
        SetWindowLongPtr(child, GWL_STYLE, flags);
        SetParent(child, static_cast<HWND>(parent));
        MoveWindow(child, 0, 0, rect.right - rect.left, rect.bottom - rect.top, TRUE);
        ShowWindow(child, SW_SHOW);
#endif
    }

    startFrameTimer();
    return CPluginView::attached(parent, type);
}

Steinberg::tresult PLUGIN_API WebUIView::removed() {
    stopFrameTimer();
    webview_.reset();
    parentView_ = nullptr;
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

// M032 S02 (T03): read the optional {lane} export payload. Absent, non-int, or
// out-of-range values yield -1 (all lanes), so the all-lanes default is
// preserved and renderPatternToSMF safely ignores a bad index (conductor-only).
static int exportLaneFilter(const choc::value::ValueView& payload) {
    if (!payload.isObject() || !payload.hasObjectMember("lane"))
        return -1;
    const auto lane = payload["lane"].get<int32_t>();
    return (lane >= 0 && lane < kMaxLanes) ? lane : -1;
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

    if (name == "setLaneName") {
        int lane = payload["lane"].get<int32_t>();
        if (lane < 0 || lane >= kMaxLanes)
            return;
        if (!payload.hasObjectMember("name") || !payload["name"].isString())
            return;
        auto laneName = payload["name"].toString();
        // Mirror the native inline-rename invariant (lane_edit_view.cpp): reject
        // empty names and cap length at LaneEditView::kMaxNameLength (15). Empty or
        // oversized payloads are dropped silently per the clamp-and-ignore rule
        // shared by setCells/setEuclid. laneName lands in the pushed state JSON via
        // nameFn (controller_->laneName) and persists through controller_base
        // getState serialization, exactly like the native rename gesture.
        if (laneName.empty() || laneName.size() > 15)
            return;
        controller_->setLaneName(lane, laneName);
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

    if (name == "setCaptureBars") {
        // G07: capture-window length is the global kCaptureLength param, a 1-32
        // bar integer. processor.cpp maps norm -> 1 + round(norm * 31), so the
        // inverse is norm = (bars - 1) / 31. Clamp to the 1-32 domain and drive
        // the real parameter through begin/perform/end just like selectScene.
        if (!payload.hasObjectMember("bars"))
            return;
        int bars = std::clamp(payload["bars"].get<int32_t>(), 1, 32);
        const double norm = static_cast<double>(bars - 1) / 31.0;
        controller_->beginEdit(ParamIDs::kCaptureLength);
        controller_->setParamNormalized(ParamIDs::kCaptureLength, norm);
        controller_->performEdit(ParamIDs::kCaptureLength, norm);
        controller_->endEdit(ParamIDs::kCaptureLength);
        return;
    }

    if (name == "setEnvelope") {
        int lane = payload["lane"].get<int32_t>();
        int index = payload["index"].get<int32_t>();
        if (lane < 0 || lane >= kMaxLanes || index < 0 || index >= kMaxEnvelopesPerLane)
            return;
        auto& laneCfg = scene.lanes[lane];
        auto& ea = laneCfg.envelopes[index];
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
            // E1 parity (M049 S01, wasm_api.cpp poly_action_set_envelope): if the
            // caller adds an envelope past the current count, clear any gap slots
            // [envelopeCount, index) — EnvelopeAssign{} defaults to active=true, so
            // without this the engine would resurrect phantom full-depth Velocity
            // LFOs — then grow envelopeCount so the new envelope is evaluated.
            for (int i = laneCfg.envelopeCount; i < index; ++i) {
                laneCfg.envelopes[i].envelope = Envelope{};
                laneCfg.envelopes[i].active = false;
            }
            if (index >= laneCfg.envelopeCount)
                laneCfg.envelopeCount = index + 1;
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
        // The preset label is per-scene and lives on the controller; re-push so the
        // header shows the newly-selected scene's label (blank during Morph) instead
        // of the previous scene's name. Runs on the WebView message thread, same as
        // pushState's other callers.
        pushState();
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
            controller_->setPresetLabel(currentPresetName_);
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
            controller_->setPresetLabel(currentPresetName_);
            controller_->mutableActiveScene() = state;
            // kWebPresetLaneNames only carries explicit rows for the first
            // batch of factory presets; later presets (index >= the number of
            // initialised rows) have all-null entries. Reset to sane defaults
            // first, then override only where a non-null label exists — passing
            // a null const char* into setLaneName() would construct
            // std::string(nullptr) and crash the host in strlen(). See L4/L6:
            // the preset inventory grew from 14 to 43 without extending this
            // table.
            controller_->resetLaneNames();
            for (int lane = 0; lane < kMaxLanes; ++lane) {
                if (const char* label = kWebPresetLaneNames[index][lane])
                    controller_->setLaneName(lane, label);
            }

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

    // M051 S08: capture state machine drivers. notify() on the processor only
    // latches an atomic command; process() applies it on the audio thread. Both
    // ride the existing message channel (no kExportTrigger param edge — that
    // WebUI prefetch path is retired here; the native VSTGUI variant keeps
    // kExportTrigger until M053).
    if (name == "armCapture") {
        sendCaptureCommand("ArmCapture");
        return;
    }

    if (name == "resetCapture") {
        sendCaptureCommand("ResetCapture");
        return;
    }

    if (name == "exportSaveAs") {
        // M053 S11: offline export. Render the CURRENT pattern to SMF directly
        // (fabricated playing transport, no DAW involvement) and open the Save-As
        // panel over those bytes. No capture-state gate and no processor
        // round-trip — the Export chip works from a stopped preview.
        // saveDialogOpen_ still guards against re-entrant panels.
        // M032 S02 (T03): an optional {lane} payload restricts the export to one
        // lane; absent/negative means all lanes (unchanged all-lanes default).
        if (saveDialogOpen_)
            return;
        openMidiExportDialog(renderCurrentPatternSmf(exportLaneFilter(payload)));
        return;
    }

    if (name == "beginMidiDrag") {
        // M053 S11: offline drag-to-DAW sibling of exportSaveAs. Render the
        // current pattern offline and hand the bytes to the native drag-source
        // window (NSPasteboard / OLE CF_HDROP, via the platform_drag_source
        // seam). No capture gating — works from a stopped preview.
        // M032 S02 (T03): honours the same optional {lane} per-lane payload.
        beginDragExport(renderCurrentPatternSmf(exportLaneFilter(payload)));
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
    // The controller holds the persisted preset label (restored from v3 state on
    // reload). Sync our working copy from it so a project reload shows the
    // last-applied preset name instead of the constructor default "Init", and so
    // suggestedExportName() slugs from the restored name too.
    currentPresetName_ = controller_->presetLabel();
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
    // M051 S02: host time signature for subtle header display
    int tsNum = snap ? snap->timeSigNumerator.load(std::memory_order_relaxed) : 4;
    int tsDen = snap ? snap->timeSigDenominator.load(std::memory_order_relaxed) : 4;

    // M051 S08: capture state machine progression (T02's UISnapshot atomics).
    // The Cloth timeline renders these directly (the visual is the receipt).
    int capState = snap ? snap->captureState.load(std::memory_order_relaxed) : 0;
    int capBars = snap ? snap->captureBars.load(std::memory_order_relaxed) : 8;
    double capProg = snap ? snap->captureProgressBars.load(std::memory_order_relaxed) : 0.0;

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
    // M073: emissions add up to kEmissionRingCap entries/lane (~60 chars each);
    // reserve generously so the per-frame build stays a single allocation.
    js.reserve(4096);
    js += "{\"type\":\"frame\",\"frame\":{";

    char buf[160];
    std::snprintf(buf, sizeof(buf),
                  "\"t8\":%.4f,\"playing\":%s,\"convLeft\":%d,\"tsNum\":%d,\"tsDen\":%d,"
                  "\"capState\":%d,\"capBars\":%d,\"capProg\":%.4f",
                  t8, playing ? "true" : "false", convLeft, tsNum, tsDen, capState, capBars, capProg);
    js += buf;

    const auto& gs = controller_->activeScene();
    js += ",\"lanes\":[";
    for (int i = 0; i < gs.activeLaneCount; ++i) {
        if (i > 0)
            js += ',';
        double phase = snap ? snap->lanePhases[i].load(std::memory_order_relaxed) : 0.0;
        int step = static_cast<int>(phase * gs.lanes[i].cycle.steps) % gs.lanes[i].cycle.steps;
        std::snprintf(buf, sizeof(buf), "{\"ph\":%.4f,\"step\":%d,\"emissions\":[", phase, step);
        js += buf;
        // M073: drain the lane's UISnapshot emission ring (oldest→newest) so the
        // desk overlay + played timeline light up in the DAW. kind is the int
        // EmissionKind; plugin-host.js maps it to the base/ghost/add/drop label.
        if (snap && i < kMaxLanes) {
            constexpr int cap = UISnapshot::kEmissionRingCap;
            uint64_t head = snap->emissionHead[i].load(std::memory_order_acquire);
            int n = head < static_cast<uint64_t>(cap) ? static_cast<int>(head) : cap;
            uint64_t startIdx = head - static_cast<uint64_t>(n);
            for (int k = 0; k < n; ++k) {
                const auto& slot = snap->emissionRing[i][static_cast<int>((startIdx + k) % cap)];
                if (k > 0)
                    js += ',';
                std::snprintf(buf, sizeof(buf), "{\"ppq\":%.4f,\"shiftedPpq\":%.4f,\"step\":%d,\"kind\":%d}",
                              slot.ppq.load(std::memory_order_relaxed), slot.shiftedPpq.load(std::memory_order_relaxed),
                              slot.step.load(std::memory_order_relaxed), slot.kind.load(std::memory_order_relaxed));
                js += buf;
            }
        }
        js += "]}";
    }
    js += "]}}";

    webview_->evaluateJavascript("window.polyHostPush(" + js + ")");
}

void WebUIView::sendCaptureCommand(const char* messageId) {
    if (auto* msg = controller_->allocateMessage()) {
        msg->setMessageID(messageId);
        controller_->sendMessage(msg);
        msg->release();
    }
}

std::string WebUIView::suggestedExportName() const {
    std::string name = "poly";
    if (!currentPresetName_.empty()) {
        name += '-';
        // Sanitise preset name to a filesystem-friendly slug: lowercase,
        // spaces → '-', drop non-alphanumeric-non-hyphen.
        for (char c : currentPresetName_) {
            if (c >= 'A' && c <= 'Z')
                name += static_cast<char>(c - 'A' + 'a');
            else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')
                name += c;
            else if (c == ' ' || c == '_')
                name += '-';
        }
    }
    const auto& gs = controller_->activeScene();
    char seedBuf[24];
    std::snprintf(seedBuf, sizeof(seedBuf), "-%llu", static_cast<unsigned long long>(gs.seed));
    name += seedBuf;

    // Wall-clock timestamp disambiguator. Poly is deterministic — the same
    // preset+seed produces byte-identical MIDI, so without this two exports
    // collide on one filename and the second silently offers to overwrite the
    // first. This runs on the UI/message thread at dialog-open time (not the
    // audio thread), so a wall-clock read here is RT-safe by placement.
    const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tmBuf{};
#ifdef _WIN32
    localtime_s(&tmBuf, &now);
#else
    localtime_r(&now, &tmBuf);
#endif
    char tsBuf[20];
    if (std::strftime(tsBuf, sizeof(tsBuf), "-%Y%m%d-%H%M%S", &tmBuf) > 0)
        name += tsBuf;

    name += ".mid";
    return name;
}

std::vector<uint8_t> WebUIView::renderCurrentPatternSmf(int laneFilter) const {
    // M053 S11: render the CURRENT pattern (controller cachedState) to an SMF
    // blob with no DAW transport. Bars come from the capture-length mirror
    // (kCaptureLength) and timesig from the UI snapshot. Runs on the message
    // thread; allocation is fine.
    //
    // M032 S03 (T02): tempo now comes from the UI snapshot too. The processor
    // publishes ctx.tempo into UISnapshot.tempoBpm every process() block, so the
    // exported SMF tempo meta mirrors the host tempo the plugin last saw instead
    // of the old hardcoded 120.0 (MEM054). The snapshot defaults to 120.0 before
    // the first process() call, preserving the prior fallback when no host tempo
    // has been observed yet. Tempo only affects the SMF tempo meta event, not the
    // PPQ note positions.
    auto* snap = controller_->uiSnapshot();
    const int bars = snap ? snap->captureBars.load(std::memory_order_relaxed) : 8;
    const int tsNum = snap ? snap->timeSigNumerator.load(std::memory_order_relaxed) : 4;
    const int tsDen = snap ? snap->timeSigDenominator.load(std::memory_order_relaxed) : 4;
    const double tempo = snap ? snap->tempoBpm.load(std::memory_order_relaxed) : 120.0;
    return renderPatternToSMF(controller_->cachedState(), bars, tempo, tsNum, tsDen, laneFilter);
}

// Test-only export byte sink, gated on the POLY_EXPORT_SINK env var. When set
// (runner-only; unset in every shipping build), the shipping Export path also
// writes the exact SMF bytes it would hand to the native Save-As panel to that
// file path. This lets the L4-web e2e capture the IN-DAW export blob over CDP
// without automating the modal native file dialog — the same pattern as
// POLY_PROBE_OUTPUT (the env var is read from the process Cubase inherited at
// launch; launch-cubase.ps1 exports it). No-op and zero cost when the var is
// unset. Returns true if a sink was configured and the write succeeded, so the
// caller can log the fallout; a failed write is logged but never blocks the
// user-facing dialog/drag. Message-thread only (called from the export
// handlers), so blocking file I/O is fine here.
static bool writeExportSinkIfEnabled(const std::vector<uint8_t>& bytes) {
    const char* sinkPath = std::getenv("POLY_EXPORT_SINK");
    if (sinkPath == nullptr || sinkPath[0] == '\0')
        return false;
    std::FILE* f = std::fopen(sinkPath, "wb");
    if (f == nullptr) {
        std::fprintf(stderr, "[poly] POLY_EXPORT_SINK: could not open %s for writing\n", sinkPath);
        return false;
    }
    const size_t written = bytes.empty() ? 0 : std::fwrite(bytes.data(), 1, bytes.size(), f);
    std::fclose(f);
    if (written != bytes.size()) {
        std::fprintf(stderr, "[poly] POLY_EXPORT_SINK: short write to %s (%zu/%zu bytes)\n", sinkPath, written,
                     bytes.size());
        return false;
    }
    std::fprintf(stderr, "[poly] POLY_EXPORT_SINK: wrote %zu bytes to %s\n", bytes.size(), sinkPath);
    return true;
}

void WebUIView::beginDragExport(const std::vector<uint8_t>& bytes) {
    // Open the native drag-source window over the offline-rendered SMF bytes.
    // Unlike the Save-As panel this is non-modal (no saveDialogOpen_ re-entrancy
    // guard): beginMidiDragExport writes a temp .mid and hands off to the OS drag
    // pasteboard, returning immediately.
    //
    // Test hook (POLY_EXPORT_SINK): in sink mode the per-lane DRAG export bytes
    // are written to the sink and the OS drag is SKIPPED — beginMidiDragExport
    // would start a native drag loop with no drop target under the unattended
    // e2e. This makes S02's deferred single-lane-drag UAT verifiable over CDP
    // (click the lane's drag affordance, then validate the sink file) without
    // automating an OS drag gesture. Off and free in shipping builds, where the
    // real OS drag runs unchanged.
    if (writeExportSinkIfEnabled(bytes))
        return;
    beginMidiDragExport(parentView_, suggestedExportName(), bytes);
}

void WebUIView::pushExportResult(const std::string& savedPath) {
    if (!webview_ || !webviewReady_)
        return;
    // Notify the WebUI so it can toast success or clear its "…" pending
    // indicator. Empty path = cancelled.
    std::string js = "window.polyHostPush({\"type\":\"exportResult\",\"savedPath\":";
    if (savedPath.empty()) {
        js += "\"\"";
    } else {
        js += choc::json::getEscapedQuotedString(savedPath);
    }
    js += "})";
    webview_->evaluateJavascript(js);
}

void WebUIView::openMidiExportDialog(const std::vector<uint8_t>& bytes) {
    // Test hook (POLY_EXPORT_SINK): in sink mode the shipping export bytes are
    // written straight to the sink path and the native Save-As panel is SKIPPED.
    // The panel's Show() is modal and blocks the UI thread until a human picks a
    // path — under the unattended L4-web e2e that would hang forever. In sink
    // mode we already have the exact bytes on disk, so bypassing the modal loses
    // nothing and lets the e2e drive export unattended. We still push an
    // exportResult carrying the sink path so the WebUI toast fires — an
    // observable success signal the CDP spec can assert on. Off (and free) in
    // every shipping build, where the modal path runs unchanged.
    if (writeExportSinkIfEnabled(bytes)) {
        pushExportResult(std::getenv("POLY_EXPORT_SINK"));
        return;
    }
    saveDialogOpen_ = true;
    openMidiSaveDialog(parentView_, suggestedExportName(), bytes, [this](const std::string& savedPath) {
        saveDialogOpen_ = false;
        pushExportResult(savedPath);
    });
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
