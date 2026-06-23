#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace poly {

static const Steinberg::FUID kPolyProcessorUID(0x8A3B7C01, 0xE5F24D83, 0x9B1A6E47, 0xD20C5F98);

static const Steinberg::FUID kPolyControllerUID(0x2F6D9B04, 0xA7C83E15, 0x4E0F2B69, 0x81D7A3C6);

static constexpr auto kPolyPluginName = "Poly";
static constexpr auto kPolyVersionString = "1.0.0";

namespace ParamIDs {

static constexpr int kParamsPerLane = 16;

static constexpr int kProbability = 0;
static constexpr int kBaseVelocity = 1;
static constexpr int kEmphasisProb = 2;
static constexpr int kGhostFloor = 3;
static constexpr int kVelocitySpread = 4;
static constexpr int kSwingAmount = 5;
static constexpr int kHumanizeMs = 6;
static constexpr int kNoteDuration = 7;
static constexpr int kActive = 8;
static constexpr int kPhraseLength = 9;
static constexpr int kPhraseGap = 10;
static constexpr int kPhraseOffset = 11;
static constexpr int kMutationRate = 12;
static constexpr int kDriftRate = 13;
static constexpr int kTimingOffset = 14;
static constexpr int kKotekanSource = 15;
static constexpr int kLaneParamCount = 16;

inline Steinberg::Vst::ParamID laneParam(int lane, int offset) {
    return static_cast<Steinberg::Vst::ParamID>(lane * kParamsPerLane + offset);
}

static constexpr Steinberg::Vst::ParamID kMacroComplexity = 200;
static constexpr Steinberg::Vst::ParamID kMacroDensity = 201;
static constexpr Steinberg::Vst::ParamID kMacroSyncopation = 202;
static constexpr Steinberg::Vst::ParamID kMacroSwing = 203;
static constexpr Steinberg::Vst::ParamID kMacroTension = 204;
static constexpr Steinberg::Vst::ParamID kMacroHumanize = 205;

static constexpr Steinberg::Vst::ParamID kActiveLaneCount = 300;
static constexpr Steinberg::Vst::ParamID kSeed = 301;

static constexpr Steinberg::Vst::ParamID kVelocityOutputBase = 400;
inline Steinberg::Vst::ParamID velocityOutput(int lane) {
    return kVelocityOutputBase + static_cast<Steinberg::Vst::ParamID>(lane);
}

static constexpr Steinberg::Vst::ParamID kLanePhaseOutputBase = 420;
inline Steinberg::Vst::ParamID lanePhaseOutput(int lane) {
    return kLanePhaseOutputBase + static_cast<Steinberg::Vst::ParamID>(lane);
}

static constexpr Steinberg::Vst::ParamID kEnvelopeValueOutputBase = 430;
inline Steinberg::Vst::ParamID envelopeValueOutput(int lane) {
    return kEnvelopeValueOutputBase + static_cast<Steinberg::Vst::ParamID>(lane);
}

static constexpr Steinberg::Vst::ParamID kPhrasePhaseOutputBase = 440;
inline Steinberg::Vst::ParamID phrasePhaseOutput(int lane) {
    return kPhrasePhaseOutputBase + static_cast<Steinberg::Vst::ParamID>(lane);
}

static constexpr Steinberg::Vst::ParamID kLaneCoreBase = 800;
static constexpr int kCoreParamsPerLane = 8;
static constexpr int kCoreSteps = 0;
static constexpr int kCoreSubdivision = 1;
static constexpr int kCoreHits = 2;
static constexpr int kCoreRotation = 3;
static constexpr int kCoreMidiNote = 4;
static constexpr int kCoreCellCount = 5;
static constexpr int kCoreTimeline = 6;
static constexpr int kCoreFixedPatternLen = 7;

inline Steinberg::Vst::ParamID laneCoreParam(int lane, int offset) {
    return kLaneCoreBase + static_cast<Steinberg::Vst::ParamID>(lane * kCoreParamsPerLane + offset);
}

static constexpr Steinberg::Vst::ParamID kSelectedLane = 700;

static constexpr Steinberg::Vst::ParamID kSceneSelect = 500;
static constexpr Steinberg::Vst::ParamID kSceneMorph = 501;

static constexpr Steinberg::Vst::ParamID kExportTrigger = 600;
static constexpr Steinberg::Vst::ParamID kCaptureLength = 601;
static constexpr Steinberg::Vst::ParamID kCaptureReady = 602;

} // namespace ParamIDs

namespace UnitIDs {

static constexpr Steinberg::Vst::UnitID kLaneBase = 1;
inline Steinberg::Vst::UnitID lane(int idx) {
    return kLaneBase + static_cast<Steinberg::Vst::UnitID>(idx);
}
static constexpr Steinberg::Vst::UnitID kMacros = 9;
static constexpr Steinberg::Vst::UnitID kGlobal = 10;
static constexpr Steinberg::Vst::UnitID kScene = 11;
static constexpr Steinberg::Vst::UnitID kOutput = 12;
static constexpr Steinberg::Vst::UnitID kExport = 13;

} // namespace UnitIDs

} // namespace poly
