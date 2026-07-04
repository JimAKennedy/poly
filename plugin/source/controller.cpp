#include "controller.h"

#include <cstdio>
#include <cstring>

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"

#include "plugids.h"
#include "poly/scene.h"
#include "poly/state_io.h"
#include "poly/types.h"
#include "ui/cell_editor_view.h"
#include "ui/cross_rhythm_view.h"
#include "ui/envelope_curve_view.h"
#include "ui/export_controls_view.h"
#include "ui/header_view.h"
#include "ui/lane_edit_view.h"
#include "ui/lane_grid_view.h"
#include "ui/micro_timing_editor_view.h"
#include "ui/note_map_view.h"
#include "ui/phase_alignment_view.h"
#include "ui/scene_bar_view.h"
#include "ui/timeline_step_editor_view.h"
#include "ui/velocity_view.h"
#ifdef POLY_WEB_UI
#include "webui/web_ui_view.h"
#endif

namespace poly {

static const char* kDefaultLaneNames[] = {"Kick", "Snare", "HH Closed", "HH Open", "Tom Hi", "Tom Lo", "Ride", "Crash"};

namespace {

struct LaneParamDef {
    int offset;
    const char* name;
    const char* units;
    Steinberg::int32 steps;
    double defaultNorm;
};

static constexpr LaneParamDef kLaneParamDefs[] = {
    {ParamIDs::kProbability, "Probability", "%", 0, 1.0},
    {ParamIDs::kBaseVelocity, "Base Velocity", "", 127, 100.0 / 127.0},
    {ParamIDs::kEmphasisProb, "Emphasis", "%", 0, 0.5},
    {ParamIDs::kGhostFloor, "Ghost Floor", "", 127, 30.0 / 127.0},
    {ParamIDs::kVelocitySpread, "Spread", "%", 0, 0.05},
    {ParamIDs::kSwingAmount, "Swing", "%", 0, 0.0},
    {ParamIDs::kHumanizeMs, "Humanize", "ms", 0, 0.0},
    {ParamIDs::kNoteDuration, "Duration", "beats", 0, 0.0},
    {ParamIDs::kActive, "Active", "", 1, 1.0},
    {ParamIDs::kPhraseLength, "Phrase Len", "bars", 0, 0.0},
    {ParamIDs::kPhraseGap, "Phrase Gap", "bars", 0, 0.0},
    {ParamIDs::kPhraseOffset, "Phrase Ofs", "bars", 0, 0.0},
    {ParamIDs::kMutationRate, "Mutation", "%", 0, 0.0},
    {ParamIDs::kDriftRate, "Drift Rate", "st/bar", 0, 0.5},
    {ParamIDs::kTimingOffset, "Timing Offset", "ms", 0, 0.5},
    {ParamIDs::kKotekanSource, "Kotekan Source", "", 8, 0.0},
};

struct CoreParamDef {
    int offset;
    const char* name;
    const char* units;
    Steinberg::int32 steps;
    double defaultNorm;
};

static constexpr CoreParamDef kCoreParamDefs[] = {
    {ParamIDs::kCoreSteps, "Steps", "", 63, 3.0 / 63.0},
    {ParamIDs::kCoreSubdivision, "Subdivision", "", 4, 0.5},
    {ParamIDs::kCoreHits, "Hits", "", 64, 4.0 / 64.0},
    {ParamIDs::kCoreRotation, "Rotation", "", 63, 0.0},
    {ParamIDs::kCoreMidiNote, "MIDI Note", "", 127, 36.0 / 127.0},
    {ParamIDs::kCoreCellCount, "Cell Count", "", 64, 0.0},
    {ParamIDs::kCoreTimeline, "Timeline", "", 1, 0.0},
    {ParamIDs::kCoreFixedPatternLen, "Pattern Length", "", 64, 0.0},
    {ParamIDs::kCoreTempoMult, "Tempo Mult", "x", 0, 0.2},
    {ParamIDs::kCoreMidiChannel, "MIDI Channel", "", 16, 0.0},
};

} // namespace

void PolyController::setLaneName(int lane, const std::string& name) {
    if (lane >= 0 && lane < kMaxLanes)
        laneNames_[lane] = name;
}

void PolyController::resetLaneNames() {
    for (int i = 0; i < kMaxLanes; ++i)
        laneNames_[i] = kDefaultLaneNames[i];
}

Steinberg::tresult PLUGIN_API PolyController::initialize(Steinberg::FUnknown* context) {
    auto result = EditControllerEx1::initialize(context);
    if (result != Steinberg::kResultOk)
        return result;

    for (int i = 0; i < kMaxLanes; ++i)
        laneNames_[i] = kDefaultLaneNames[i];

    using namespace Steinberg::Vst;
    using Steinberg::Vst::ParameterInfo;

    addUnit(new Unit(USTRING("Root"), kRootUnitId));
    for (int lane = 0; lane < kMaxLanes; ++lane) {
        Steinberg::Vst::String128 unitName;
        char ascii[32];
        std::snprintf(ascii, sizeof(ascii), "Lane %d", lane + 1);
        Steinberg::UString(unitName, 128).fromAscii(ascii);
        addUnit(new Unit(unitName, UnitIDs::lane(lane), kRootUnitId));
    }
    addUnit(new Unit(USTRING("Macros"), UnitIDs::kMacros, kRootUnitId));
    addUnit(new Unit(USTRING("Global"), UnitIDs::kGlobal, kRootUnitId));
    addUnit(new Unit(USTRING("Scene"), UnitIDs::kScene, kRootUnitId));
    addUnit(new Unit(USTRING("Output"), UnitIDs::kOutput, kRootUnitId));

    auto registerLaneParams = [this](const auto& defs, auto idFn) {
        using Steinberg::Vst::ParameterInfo;
        for (int lane = 0; lane < kMaxLanes; ++lane) {
            for (const auto& def : defs) {
                ParameterInfo info = {};
                info.id = idFn(lane, def.offset);
                Steinberg::UString(info.title, 128).fromAscii(def.name);
                Steinberg::UString(info.units, 128).fromAscii(def.units);
                info.stepCount = def.steps;
                info.defaultNormalizedValue = def.defaultNorm;
                info.flags = ParameterInfo::kCanAutomate;
                info.unitId = UnitIDs::lane(lane);
                parameters.addParameter(info);
            }
        }
    };
    registerLaneParams(kLaneParamDefs, ParamIDs::laneParam);
    registerLaneParams(kCoreParamDefs, ParamIDs::laneCoreParam);

    auto addParam = [this](Steinberg::Vst::ParamID id, const char* title, const char* units, Steinberg::int32 steps,
                           double defNorm, Steinberg::Vst::UnitID unitId,
                           Steinberg::int32 flags = ParameterInfo::kCanAutomate) {
        ParameterInfo info = {};
        info.id = id;
        Steinberg::UString(info.title, 128).fromAscii(title);
        Steinberg::UString(info.units, 128).fromAscii(units);
        info.stepCount = steps;
        info.defaultNormalizedValue = defNorm;
        info.flags = flags;
        info.unitId = unitId;
        parameters.addParameter(info);
    };

    addParam(ParamIDs::kMacroComplexity, "Complexity", "%", 0, 0.5, UnitIDs::kMacros);
    addParam(ParamIDs::kMacroDensity, "Density", "%", 0, 0.5, UnitIDs::kMacros);
    addParam(ParamIDs::kMacroSyncopation, "Syncopation", "%", 0, 0.0, UnitIDs::kMacros);
    addParam(ParamIDs::kMacroSwing, "Swing", "%", 0, 0.0, UnitIDs::kMacros);
    addParam(ParamIDs::kMacroTension, "Tension", "%", 0, 0.0, UnitIDs::kMacros);
    addParam(ParamIDs::kMacroHumanize, "Humanize", "%", 0, 0.0, UnitIDs::kMacros);

    addParam(ParamIDs::kActiveLaneCount, "Active Lanes", "", 7, 3.0 / 7.0, UnitIDs::kGlobal);
    addParam(ParamIDs::kSeed, "Seed", "", 0, 0.0, UnitIDs::kGlobal);

    registerOutputParameters();

    addParam(ParamIDs::kSelectedLane, "Selected Lane", "", 7, 0.0, UnitIDs::kGlobal, ParameterInfo::kNoFlags);

    addParam(ParamIDs::kTransportPpqOutput, "Transport PPQ", "", 0, 0.0, UnitIDs::kOutput, ParameterInfo::kIsReadOnly);

    addParam(ParamIDs::kSceneSelect, "Select", "", 2, 0.0, UnitIDs::kScene);
    addParam(ParamIDs::kSceneMorph, "Morph", "%", 0, 0.0, UnitIDs::kScene);

    addParam(ParamIDs::kChainEnabled, "Chain Enable", "", 1, 0.0, UnitIDs::kScene);
    addParam(ParamIDs::kChainMode, "Chain Mode", "", 2, 0.0, UnitIDs::kScene);
    addParam(ParamIDs::kChainEntryCount, "Chain Length", "", kMaxChainEntries, 0.0, UnitIDs::kScene);

    for (int e = 0; e < kMaxChainEntries; ++e) {
        char title[32];
        std::snprintf(title, sizeof(title), "Chain %d Scene", e + 1);
        addParam(ParamIDs::chainEntryParam(e, ParamIDs::kChainEntryScene), title, "", 2, 0.0, UnitIDs::kScene);
        std::snprintf(title, sizeof(title), "Chain %d Bars", e + 1);
        addParam(ParamIDs::chainEntryParam(e, ParamIDs::kChainEntryBars), title, "bars", 31, 3.0 / 31.0,
                 UnitIDs::kScene);
    }

    addUnit(new Unit(USTRING("Export"), UnitIDs::kExport, kRootUnitId));
    addParam(ParamIDs::kExportTrigger, "Export", "", 1, 0.0, UnitIDs::kExport);
    addParam(ParamIDs::kCaptureLength, "Capture Bars", "bars", 31, 7.0 / 31.0, UnitIDs::kExport);
    addParam(ParamIDs::kCaptureReady, "Capture Ready", "", 0, 0.0, UnitIDs::kExport, ParameterInfo::kIsReadOnly);
    addParam(ParamIDs::kCaptureEventCount, "Capture Events", "", 0, 0.0, UnitIDs::kExport, ParameterInfo::kIsReadOnly);

    return Steinberg::kResultOk;
}

void PolyController::registerOutputParameters() {
    using namespace Steinberg::Vst;

    auto addParam = [this](Steinberg::Vst::ParamID pid, const char* title, Steinberg::int32 flags) {
        ParameterInfo info = {};
        info.id = pid;
        Steinberg::UString(info.title, 128).fromAscii(title);
        info.flags = flags;
        info.unitId = UnitIDs::kOutput;
        parameters.addParameter(info);
    };

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        char title[32];
        std::snprintf(title, sizeof(title), "Lane %d", lane + 1);
        addParam(ParamIDs::velocityOutput(lane), title, ParameterInfo::kIsReadOnly);
    }
    for (int lane = 0; lane < kMaxLanes; ++lane) {
        char title[32];
        std::snprintf(title, sizeof(title), "L%d Phase", lane + 1);
        addParam(ParamIDs::lanePhaseOutput(lane), title, ParameterInfo::kIsReadOnly);
    }
    for (int lane = 0; lane < kMaxLanes; ++lane) {
        char title[32];
        std::snprintf(title, sizeof(title), "L%d Envelope", lane + 1);
        addParam(ParamIDs::envelopeValueOutput(lane), title, ParameterInfo::kIsReadOnly);
    }
    for (int lane = 0; lane < kMaxLanes; ++lane) {
        char title[32];
        std::snprintf(title, sizeof(title), "L%d Phrase", lane + 1);
        addParam(ParamIDs::phrasePhaseOutput(lane), title, ParameterInfo::kIsReadOnly);
    }
}

Steinberg::IPlugView* PLUGIN_API PolyController::createView(Steinberg::FIDString name) {
    if (Steinberg::FIDStringsEqual(name, Steinberg::Vst::ViewType::kEditor)) {
#ifdef POLY_WEB_UI
        return new WebUIView(this); // ownership-transfer
#else
        auto* view = new VSTGUI::VST3Editor(this, "view", "poly.uidesc"); // ownership-transfer
        view->setDelegate(this);
        return view;
#endif
    }
    return nullptr;
}

VSTGUI::CView* PolyController::createCustomView(VSTGUI::UTF8StringPtr name, const VSTGUI::UIAttributes& /*attributes*/,
                                                const VSTGUI::IUIDescription* /*description*/,
                                                VSTGUI::VST3Editor* /*editor*/) {
    if (std::strcmp(name, "HeaderView") == 0) {
        return new HeaderView(VSTGUI::CRect(0, 0, 600, 32), this); // ownership-transfer
    }
    if (std::strcmp(name, "LaneEditView") == 0) {
        return new LaneEditView(VSTGUI::CRect(0, 0, 580, 126), this); // ownership-transfer
    }
    if (std::strcmp(name, "LaneGridView") == 0) {
        return new LaneGridView(VSTGUI::CRect(0, 0, 580, 156), this); // ownership-transfer
    }
    if (std::strcmp(name, "VelocityView") == 0) {
        return new VelocityView(VSTGUI::CRect(0, 0, 580, 40), this); // ownership-transfer
    }
    if (std::strcmp(name, "EnvelopeCurveView") == 0) {
        return new EnvelopeCurveView(VSTGUI::CRect(0, 0, 380, 146), this); // ownership-transfer
    }
    if (std::strcmp(name, "PhaseAlignmentView") == 0) {
        return new PhaseAlignmentView(VSTGUI::CRect(0, 0, 190, 146), this); // ownership-transfer
    }
    if (std::strcmp(name, "CellEditorView") == 0) {
        return new CellEditorView(VSTGUI::CRect(0, 0, 580, 60), this); // ownership-transfer
    }
    if (std::strcmp(name, "TimelineStepEditorView") == 0) {
        return new TimelineStepEditorView(VSTGUI::CRect(0, 0, 580, 60), this); // ownership-transfer
    }
    if (std::strcmp(name, "MicroTimingEditorView") == 0) {
        return new MicroTimingEditorView(VSTGUI::CRect(0, 0, 580, 60), this); // ownership-transfer
    }
    if (std::strcmp(name, "SceneBarView") == 0) {
        return new SceneBarView(VSTGUI::CRect(0, 0, 580, 46), this); // ownership-transfer
    }
    if (std::strcmp(name, "CrossRhythmView") == 0) {
        return new CrossRhythmView(VSTGUI::CRect(0, 0, 580, 146), this); // ownership-transfer
    }
    if (std::strcmp(name, "NoteMapView") == 0) {
        return new NoteMapView(VSTGUI::CRect(0, 0, 600, 838), this); // ownership-transfer
    }
    if (std::strcmp(name, "ExportControlsView") == 0) {
        return new ExportControlsView(VSTGUI::CRect(0, 0, 580, 46), this); // ownership-transfer
    }
    return nullptr;
}

Steinberg::tresult PLUGIN_API PolyController::setComponentState(Steinberg::IBStream* state) {
    if (!state)
        return Steinberg::kInvalidArgument;

    auto read = [state](void* data, size_t size) -> bool {
        Steinberg::int32 bytesRead;
        return state->read(data, static_cast<Steinberg::int32>(size), &bytesRead) == Steinberg::kResultOk;
    };

    if (!readSceneState(read, cachedState_))
        return Steinberg::kResultFalse;

    const auto& gs = cachedState_.sceneA;
    for (int lane = 0; lane < kMaxLanes; ++lane) {
        const auto& cfg = gs.lanes[lane];
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kProbability), cfg.probability);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kBaseVelocity), cfg.baseVelocity / 127.0);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kEmphasisProb), cfg.emphasisProb);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kGhostFloor), cfg.ghostFloor / 127.0);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kVelocitySpread), cfg.velocitySpread);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kSwingAmount), cfg.swingAmount);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kHumanizeMs), cfg.humanizeMs / 50.0);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kNoteDuration), cfg.noteDuration / 4.0);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kActive), cfg.active ? 1.0 : 0.0);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kPhraseLength), cfg.phraseLength / 64.0);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kPhraseGap), cfg.phraseGap / 64.0);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kPhraseOffset), cfg.phraseOffset / 64.0);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kMutationRate), cfg.mutationRate);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kDriftRate),
                           static_cast<double>((cfg.driftRate + 4.0f) / 8.0f));
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kTimingOffset),
                           static_cast<double>((cfg.timingOffsetMs + 20.0f) / 40.0f));
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kKotekanSource),
                           static_cast<double>(cfg.kotekanSourceLane + 1) / 8.0);

        setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreSteps), (cfg.cycle.steps - 1) / 63.0);
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
        setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreSubdivision), subIdx / 4.0);
        setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreHits), cfg.hitCount / 64.0);
        setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreRotation), cfg.rotation / 63.0);
        setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreMidiNote), cfg.midiNote / 127.0);
        setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreCellCount), cfg.cellCount / 64.0);
        setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreTimeline), cfg.timeline ? 1.0 : 0.0);
        setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreFixedPatternLen),
                           cfg.fixedPatternLength / 64.0);
        setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreTempoMult),
                           static_cast<double>((cfg.tempoMultiplier - 0.25f) / 3.75f));
        setParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreMidiChannel), (cfg.midiChannel + 1) / 16.0);
    }

    setParamNormalized(ParamIDs::kMacroComplexity, gs.macros.complexity);
    setParamNormalized(ParamIDs::kMacroDensity, gs.macros.density);
    setParamNormalized(ParamIDs::kMacroSyncopation, gs.macros.syncopation);
    setParamNormalized(ParamIDs::kMacroSwing, gs.macros.swing);
    setParamNormalized(ParamIDs::kMacroTension, gs.macros.tension);
    setParamNormalized(ParamIDs::kMacroHumanize, gs.macros.humanize);
    setParamNormalized(ParamIDs::kActiveLaneCount, (gs.activeLaneCount - 1) / 7.0);
    setParamNormalized(ParamIDs::kSeed, gs.seed / 999999.0);
    setParamNormalized(ParamIDs::kSceneSelect, static_cast<double>(static_cast<uint8_t>(cachedState_.select)) / 2.0);
    setParamNormalized(ParamIDs::kSceneMorph, static_cast<double>(cachedState_.morphAmount));

    setParamNormalized(ParamIDs::kChainEnabled, cachedState_.chain.enabled ? 1.0 : 0.0);
    setParamNormalized(ParamIDs::kChainMode, static_cast<double>(static_cast<uint8_t>(cachedState_.chain.mode)) / 2.0);
    setParamNormalized(ParamIDs::kChainEntryCount,
                       static_cast<double>(cachedState_.chain.entryCount) / static_cast<double>(kMaxChainEntries));
    for (int e = 0; e < kMaxChainEntries; ++e) {
        const auto& entry = cachedState_.chain.entries[static_cast<size_t>(e)];
        setParamNormalized(ParamIDs::chainEntryParam(e, ParamIDs::kChainEntryScene),
                           static_cast<double>(static_cast<uint8_t>(entry.scene)) / 2.0);
        setParamNormalized(ParamIDs::chainEntryParam(e, ParamIDs::kChainEntryBars),
                           static_cast<double>(entry.bars - 1) / 31.0);
    }

    ++stateGeneration_;

    return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API PolyController::getState(Steinberg::IBStream* state) {
    if (!state)
        return Steinberg::kInvalidArgument;

    Steinberg::int32 version = kControllerStateVersion;
    if (state->write(&version, sizeof(version), nullptr) != Steinberg::kResultOk)
        return Steinberg::kResultFalse;

    for (int i = 0; i < kMaxLanes; ++i) {
        auto len = static_cast<Steinberg::int32>(laneNames_[i].size());
        if (state->write(&len, sizeof(len), nullptr) != Steinberg::kResultOk)
            return Steinberg::kResultFalse;
        if (len > 0) {
            if (state->write(laneNames_[i].data(), len, nullptr) != Steinberg::kResultOk)
                return Steinberg::kResultFalse;
        }
    }

    return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API PolyController::setState(Steinberg::IBStream* state) {
    if (!state)
        return Steinberg::kInvalidArgument;

    Steinberg::int32 version = 0;
    Steinberg::int32 bytesRead = 0;
    if (state->read(&version, sizeof(version), &bytesRead) != Steinberg::kResultOk || bytesRead != sizeof(version))
        return Steinberg::kResultFalse;

    if (version >= 1) {
        for (int i = 0; i < kMaxLanes; ++i) {
            Steinberg::int32 len = 0;
            if (state->read(&len, sizeof(len), &bytesRead) != Steinberg::kResultOk)
                break;
            if (len > 0 && len < 256) {
                laneNames_[i].resize(static_cast<size_t>(len));
                if (state->read(laneNames_[i].data(), len, &bytesRead) != Steinberg::kResultOk)
                    laneNames_[i] = kDefaultLaneNames[i];
            } else {
                laneNames_[i] = kDefaultLaneNames[i];
            }
        }
    }

    ++stateGeneration_;

    return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API PolyController::notify(Steinberg::Vst::IMessage* message) {
    if (!message)
        return Steinberg::kInvalidArgument;

    if (Steinberg::FIDStringsEqual(message->getMessageID(), "UISnapshotPtr")) {
        if (auto* attrs = message->getAttributes()) {
            Steinberg::int64 ptr = 0;
            if (attrs->getInt("ptr", ptr) == Steinberg::kResultOk && ptr != 0) {
                uiSnapshot_ = reinterpret_cast<UISnapshot*>(ptr);
            }
        }
        return Steinberg::kResultOk;
    }

    if (Steinberg::FIDStringsEqual(message->getMessageID(), "MidiExportData")) {
        if (auto* attrs = message->getAttributes()) {
            const void* data = nullptr;
            Steinberg::uint32 size = 0;
            if (attrs->getBinary("smf", data, size) == Steinberg::kResultOk && size > 0) {
                auto* bytes = static_cast<const uint8_t*>(data);
                pendingSmfData_.assign(bytes, bytes + size);
                dragSmfCache_ = pendingSmfData_;
            }
        }
        return Steinberg::kResultOk;
    }

    return EditControllerEx1::notify(message);
}

void PolyController::sendNoteMap() {
    if (auto* msg = allocateMessage()) {
        msg->setMessageID("NoteMapUpdate");
        if (auto* attrs = msg->getAttributes()) {
            attrs->setBinary("map", cachedState_.noteMap.map.data(),
                             static_cast<Steinberg::uint32>(sizeof(cachedState_.noteMap.map)));
        }
        sendMessage(msg);
        msg->release();
    }
}

void PolyController::sendCellSizes(int laneIndex) {
    if (laneIndex < 0 || laneIndex >= kMaxLanes)
        return;
    if (auto* msg = allocateMessage()) {
        msg->setMessageID("CellSizesUpdate");
        if (auto* attrs = msg->getAttributes()) {
            attrs->setInt("lane", laneIndex);
            const auto& scene = activeScene();
            attrs->setBinary("sizes", scene.lanes[laneIndex].cellSizes.data(),
                             static_cast<Steinberg::uint32>(sizeof(scene.lanes[laneIndex].cellSizes)));
        }
        sendMessage(msg);
        msg->release();
    }
}

void PolyController::sendTimelinePattern(int laneIndex) {
    if (laneIndex < 0 || laneIndex >= kMaxLanes)
        return;
    if (auto* msg = allocateMessage()) {
        msg->setMessageID("TimelinePatternUpdate");
        if (auto* attrs = msg->getAttributes()) {
            const auto& lane = activeScene().lanes[laneIndex];
            attrs->setInt("lane", laneIndex);
            attrs->setInt("patLen", lane.fixedPatternLength);
            attrs->setBinary("pattern", lane.fixedPattern.data(),
                             static_cast<Steinberg::uint32>(sizeof(lane.fixedPattern)));
        }
        sendMessage(msg);
        msg->release();
    }
}

void PolyController::sendMicroTiming(int laneIndex) {
    if (laneIndex < 0 || laneIndex >= kMaxLanes)
        return;
    if (auto* msg = allocateMessage()) {
        msg->setMessageID("MicroTimingUpdate");
        if (auto* attrs = msg->getAttributes()) {
            attrs->setInt("lane", laneIndex);
            const auto& scene = activeScene();
            attrs->setBinary("timing", scene.lanes[laneIndex].microTimingMs.data(),
                             static_cast<Steinberg::uint32>(sizeof(scene.lanes[laneIndex].microTimingMs)));
        }
        sendMessage(msg);
        msg->release();
    }
}

void PolyController::sendEnvelopeUpdate(int laneIndex, int envelopeIndex) {
    if (laneIndex < 0 || laneIndex >= kMaxLanes)
        return;
    if (envelopeIndex < 0 || envelopeIndex >= kMaxEnvelopesPerLane)
        return;
    if (auto* msg = allocateMessage()) {
        msg->setMessageID("EnvelopeUpdate");
        if (auto* attrs = msg->getAttributes()) {
            const auto& ea = activeScene().lanes[laneIndex].envelopes[envelopeIndex];
            attrs->setInt("lane", laneIndex);
            attrs->setInt("envIdx", envelopeIndex);
            attrs->setBinary("envelope", &ea.envelope, static_cast<Steinberg::uint32>(sizeof(ea.envelope)));
            attrs->setInt("active", ea.active ? 1 : 0);
        }
        sendMessage(msg);
        msg->release();
    }
}

} // namespace poly
