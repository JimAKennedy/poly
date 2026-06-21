#include "controller.h"

#include <cstdio>
#include <cstring>

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"

#include "plugids.h"
#include "poly/state_io.h"
#include "poly/types.h"
#include "ui/lane_grid_view.h"
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
    {ParamIDs::kProbability, "Probability", "", 0, 1.0},
    {ParamIDs::kBaseVelocity, "Base Velocity", "", 127, 100.0 / 127.0},
    {ParamIDs::kEmphasisProb, "Emphasis Prob", "", 0, 0.5},
    {ParamIDs::kGhostFloor, "Ghost Floor", "", 127, 30.0 / 127.0},
    {ParamIDs::kVelocitySpread, "Velocity Spread", "", 0, 0.05},
    {ParamIDs::kSwingAmount, "Swing Amount", "", 0, 0.0},
    {ParamIDs::kHumanizeMs, "Humanize", "ms", 0, 0.0},
    {ParamIDs::kNoteDuration, "Note Duration", "beats", 0, 0.0},
    {ParamIDs::kActive, "Active", "", 1, 1.0},
};

} // namespace

Steinberg::tresult PLUGIN_API PolyController::initialize(Steinberg::FUnknown* context) {
    auto result = EditController::initialize(context);
    if (result != Steinberg::kResultOk)
        return result;

    using Steinberg::Vst::ParameterInfo;

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        for (const auto& def : kLaneParamDefs) {
            ParameterInfo info = {};
            info.id = ParamIDs::laneParam(lane, def.offset);
            char ascii[64];
            std::snprintf(ascii, sizeof(ascii), "L%d %s", lane + 1, def.name);
            Steinberg::UString(info.title, 128).fromAscii(ascii);
            Steinberg::UString(info.units, 128).fromAscii(def.units);
            info.stepCount = def.steps;
            info.defaultNormalizedValue = def.defaultNorm;
            info.flags = ParameterInfo::kCanAutomate;
            info.unitId = Steinberg::Vst::kRootUnitId;
            parameters.addParameter(info);
        }
    }

    auto addSimple = [this](Steinberg::Vst::ParamID id, const char* title, Steinberg::int32 steps, double defNorm,
                            Steinberg::int32 flags = ParameterInfo::kCanAutomate) {
        ParameterInfo info = {};
        info.id = id;
        Steinberg::UString(info.title, 128).fromAscii(title);
        Steinberg::UString(info.units, 128).fromAscii("");
        info.stepCount = steps;
        info.defaultNormalizedValue = defNorm;
        info.flags = flags;
        info.unitId = Steinberg::Vst::kRootUnitId;
        parameters.addParameter(info);
    };

    addSimple(ParamIDs::kMacroComplexity, "Macro Complexity", 0, 0.5);
    addSimple(ParamIDs::kMacroDensity, "Macro Density", 0, 0.5);
    addSimple(ParamIDs::kMacroSyncopation, "Macro Syncopation", 0, 0.0);
    addSimple(ParamIDs::kMacroSwing, "Macro Swing", 0, 0.0);
    addSimple(ParamIDs::kMacroTension, "Macro Tension", 0, 0.0);
    addSimple(ParamIDs::kMacroHumanize, "Macro Humanize", 0, 0.0);
    addSimple(ParamIDs::kActiveLaneCount, "Active Lanes", 7, 3.0 / 7.0);
    addSimple(ParamIDs::kSeed, "Seed", 0, 0.0);

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        char title[64];
        std::snprintf(title, sizeof(title), "L%d Velocity Out", lane + 1);
        addSimple(ParamIDs::velocityOutput(lane), title, 0, 0.0, ParameterInfo::kIsReadOnly);
    }

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
    if (std::strcmp(name, "LaneGridView") == 0) {
        return new LaneGridView(VSTGUI::CRect(0, 0, 580, 160), this);
    }
    if (std::strcmp(name, "VelocityView") == 0) {
        return new VelocityView(VSTGUI::CRect(0, 0, 580, 80), this);
    }
    return nullptr;
}

Steinberg::tresult PLUGIN_API PolyController::setComponentState(Steinberg::IBStream* state) {
    if (!state)
        return Steinberg::kInvalidArgument;

    GrooveState gs{};
    auto read = [state](void* data, size_t size) -> bool {
        Steinberg::int32 bytesRead;
        return state->read(data, static_cast<Steinberg::int32>(size), &bytesRead) == Steinberg::kResultOk;
    };

    if (!readGrooveState(read, gs))
        return Steinberg::kResultFalse;

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
    }

    setParamNormalized(ParamIDs::kMacroComplexity, gs.macros.complexity);
    setParamNormalized(ParamIDs::kMacroDensity, gs.macros.density);
    setParamNormalized(ParamIDs::kMacroSyncopation, gs.macros.syncopation);
    setParamNormalized(ParamIDs::kMacroSwing, gs.macros.swing);
    setParamNormalized(ParamIDs::kMacroTension, gs.macros.tension);
    setParamNormalized(ParamIDs::kMacroHumanize, gs.macros.humanize);
    setParamNormalized(ParamIDs::kActiveLaneCount, (gs.activeLaneCount - 1) / 7.0);
    setParamNormalized(ParamIDs::kSeed, gs.seed / 999999.0);

    return Steinberg::kResultOk;
}

} // namespace poly
