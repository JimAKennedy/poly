#include "controller.h"

#include <cstdio>
#include <cstring>

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"

#include "plugids.h"
#include "poly/scene.h"
#include "poly/state_io.h"
#include "poly/types.h"
#include "ui/envelope_curve_view.h"
#include "ui/header_view.h"
#include "ui/lane_grid_view.h"
#include "ui/phase_alignment_view.h"
#include "ui/phrase_edit_view.h"
#include "ui/velocity_view.h"

namespace poly {

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
};

} // namespace

Steinberg::tresult PLUGIN_API PolyController::initialize(Steinberg::FUnknown* context) {
    auto result = EditControllerEx1::initialize(context);
    if (result != Steinberg::kResultOk)
        return result;

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

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        for (const auto& def : kLaneParamDefs) {
            ParameterInfo info = {};
            info.id = ParamIDs::laneParam(lane, def.offset);
            Steinberg::UString(info.title, 128).fromAscii(def.name);
            Steinberg::UString(info.units, 128).fromAscii(def.units);
            info.stepCount = def.steps;
            info.defaultNormalizedValue = def.defaultNorm;
            info.flags = ParameterInfo::kCanAutomate;
            info.unitId = UnitIDs::lane(lane);
            parameters.addParameter(info);
        }
    }

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

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        char title[32];
        std::snprintf(title, sizeof(title), "Lane %d", lane + 1);
        addParam(ParamIDs::velocityOutput(lane), title, "", 0, 0.0, UnitIDs::kOutput, ParameterInfo::kIsReadOnly);
    }

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        char title[32];
        std::snprintf(title, sizeof(title), "L%d Phase", lane + 1);
        addParam(ParamIDs::lanePhaseOutput(lane), title, "", 0, 0.0, UnitIDs::kOutput, ParameterInfo::kIsReadOnly);
    }

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        char title[32];
        std::snprintf(title, sizeof(title), "L%d Envelope", lane + 1);
        addParam(ParamIDs::envelopeValueOutput(lane), title, "", 0, 0.0, UnitIDs::kOutput, ParameterInfo::kIsReadOnly);
    }

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        char title[32];
        std::snprintf(title, sizeof(title), "L%d Phrase", lane + 1);
        addParam(ParamIDs::phrasePhaseOutput(lane), title, "", 0, 0.0, UnitIDs::kOutput, ParameterInfo::kIsReadOnly);
    }

    addParam(ParamIDs::kSceneSelect, "Select", "", 2, 0.0, UnitIDs::kScene);
    addParam(ParamIDs::kSceneMorph, "Morph", "%", 0, 0.0, UnitIDs::kScene);

    addUnit(new Unit(USTRING("Export"), UnitIDs::kExport, kRootUnitId));
    addParam(ParamIDs::kExportTrigger, "Export", "", 1, 0.0, UnitIDs::kExport);
    addParam(ParamIDs::kCaptureLength, "Capture Bars", "bars", 31, 7.0 / 31.0, UnitIDs::kExport);
    addParam(ParamIDs::kCaptureReady, "Capture Ready", "", 0, 0.0, UnitIDs::kExport, ParameterInfo::kIsReadOnly);

    return Steinberg::kResultOk;
}

Steinberg::IPlugView* PLUGIN_API PolyController::createView(Steinberg::FIDString name) {
    if (Steinberg::FIDStringsEqual(name, Steinberg::Vst::ViewType::kEditor)) {
        auto* view = new VSTGUI::VST3Editor(this, "view", "poly.uidesc");
        view->setDelegate(this);
        return view;
    }
    return nullptr;
}

VSTGUI::CView* PolyController::createCustomView(VSTGUI::UTF8StringPtr name, const VSTGUI::UIAttributes& /*attributes*/,
                                                const VSTGUI::IUIDescription* /*description*/,
                                                VSTGUI::VST3Editor* /*editor*/) {
    if (std::strcmp(name, "HeaderView") == 0) {
        return new HeaderView(VSTGUI::CRect(0, 0, 600, 32), this);
    }
    if (std::strcmp(name, "LaneGridView") == 0) {
        return new LaneGridView(VSTGUI::CRect(0, 0, 580, 156), this);
    }
    if (std::strcmp(name, "VelocityView") == 0) {
        return new VelocityView(VSTGUI::CRect(0, 0, 580, 76), this);
    }
    if (std::strcmp(name, "EnvelopeCurveView") == 0) {
        return new EnvelopeCurveView(VSTGUI::CRect(0, 0, 380, 146), this);
    }
    if (std::strcmp(name, "PhaseAlignmentView") == 0) {
        return new PhaseAlignmentView(VSTGUI::CRect(0, 0, 190, 146), this);
    }
    if (std::strcmp(name, "PhraseEditView") == 0) {
        return new PhraseEditView(VSTGUI::CRect(0, 0, 580, 60), this);
    }
    return nullptr;
}

Steinberg::tresult PLUGIN_API PolyController::setComponentState(Steinberg::IBStream* state) {
    if (!state)
        return Steinberg::kInvalidArgument;

    SceneState scene{};
    auto read = [state](void* data, size_t size) -> bool {
        Steinberg::int32 bytesRead;
        return state->read(data, static_cast<Steinberg::int32>(size), &bytesRead) == Steinberg::kResultOk;
    };

    if (!readSceneState(read, scene))
        return Steinberg::kResultFalse;

    const auto& gs = scene.sceneA;
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
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kPhraseLength), cfg.phraseLength / 16.0);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kPhraseGap), cfg.phraseGap / 16.0);
        setParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kPhraseOffset), cfg.phraseOffset / 16.0);
    }

    setParamNormalized(ParamIDs::kMacroComplexity, gs.macros.complexity);
    setParamNormalized(ParamIDs::kMacroDensity, gs.macros.density);
    setParamNormalized(ParamIDs::kMacroSyncopation, gs.macros.syncopation);
    setParamNormalized(ParamIDs::kMacroSwing, gs.macros.swing);
    setParamNormalized(ParamIDs::kMacroTension, gs.macros.tension);
    setParamNormalized(ParamIDs::kMacroHumanize, gs.macros.humanize);
    setParamNormalized(ParamIDs::kActiveLaneCount, (gs.activeLaneCount - 1) / 7.0);
    setParamNormalized(ParamIDs::kSeed, gs.seed / 999999.0);
    setParamNormalized(ParamIDs::kSceneSelect, static_cast<double>(static_cast<uint8_t>(scene.select)) / 2.0);
    setParamNormalized(ParamIDs::kSceneMorph, static_cast<double>(scene.morphAmount));

    return Steinberg::kResultOk;
}

} // namespace poly
