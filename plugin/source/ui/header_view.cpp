#include "header_view.h"

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/coptionmenu.h"

#include "../controller.h"
#include "../plugids.h"
#include "note_map_view.h"
#include "poly/presets.h"
#include "poly/types.h"

namespace poly {

HeaderView::HeaderView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setMouseEnabled(true);
}

void HeaderView::draw(VSTGUI::CDrawContext* context) {
    using namespace VSTGUI;

    auto bounds = getViewSize();

    context->setFillColor(CColor(0x18, 0x18, 0x22, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    auto titleFont = makeOwned<CFontDesc>("Arial", 15.0, kBoldFace);
    context->setFont(titleFont);
    context->setFontColor(CColor(0x4A, 0x9E, 0xFF, 0xFF));
    CRect titleRect(bounds.left + 14, bounds.top, bounds.left + 80, bounds.bottom);
    context->drawString("POLY", titleRect, kLeftText);

    auto presetFont = makeOwned<CFontDesc>("Arial", 11.0);
    context->setFont(presetFont);

    const char* presetName = "Select Preset...";
    if (selectedPreset_ == kInitPreset) {
        presetName = "Init (All Lanes)";
    } else if (selectedPreset_ >= 0 && selectedPreset_ < kFactoryPresetCount) {
        presetName = getFactoryPresetInfo(selectedPreset_).name;
    }

    CRect presetRect(bounds.left + 180, bounds.top + 5, bounds.right - 90, bounds.bottom - 5);

    context->setFillColor(CColor(0x22, 0x22, 0x30, 0xFF));
    context->setFrameColor(CColor(0x3A, 0x3A, 0x48, 0xFF));
    context->setLineWidth(1.0);
    context->drawRect(presetRect, kDrawFilledAndStroked);

    context->setFontColor(CColor(0xE8, 0xE8, 0xEC, 0xFF));
    CRect textRect = presetRect;
    textRect.left += 8;
    textRect.right -= 20;
    context->drawString(presetName, textRect, kLeftText);

    context->setFontColor(CColor(0x88, 0x88, 0xA0, 0xFF));
    CRect arrowRect = presetRect;
    arrowRect.left = arrowRect.right - 22;
    context->drawString("\xe2\x96\xbe", arrowRect, kCenterText);

    auto nmRect = noteMapButtonRect();
    bool noteMapOpen = false;
    if (auto* frame = getFrame()) {
        for (uint32_t i = 0; i < frame->getNbViews(); ++i) {
            if (dynamic_cast<NoteMapView*>(frame->getView(i))) {
                noteMapOpen = true;
                break;
            }
        }
    }
    context->setFillColor(noteMapOpen ? CColor(0x2A, 0x3A, 0x4A, 0xFF) : CColor(0x22, 0x22, 0x30, 0xFF));
    context->setFrameColor(noteMapOpen ? CColor(0x4A, 0x9E, 0xFF, 0x80) : CColor(0x3A, 0x3A, 0x48, 0xFF));
    context->setLineWidth(1.0);
    context->drawRect(nmRect, kDrawFilledAndStroked);
    auto nmFont = makeOwned<CFontDesc>("Arial", 9.0, kBoldFace);
    context->setFont(nmFont);
    context->setFontColor(noteMapOpen ? CColor(0x4A, 0x9E, 0xFF, 0xFF) : CColor(0x99, 0x99, 0xAA, 0xFF));
    context->drawString("MAP", nmRect, kCenterText);

    auto versionFont = makeOwned<CFontDesc>("Arial", 9.0);
    context->setFont(versionFont);
    context->setFontColor(CColor(0x60, 0x60, 0x6A, 0xFF));
    CRect versionRect(bounds.right - 50, bounds.top, bounds.right - 10, bounds.bottom);
    context->drawString(kPolyVersionString, versionRect, kRightText);

    CRect accentLine(bounds.left, bounds.bottom - 2, bounds.right, bounds.bottom);
    context->setFillColor(CColor(0x4A, 0x9E, 0xFF, 0x40));
    context->drawRect(accentLine, kDrawFilled);

    setDirty(false);
}

VSTGUI::CRect HeaderView::noteMapButtonRect() const {
    auto bounds = getViewSize();
    return {bounds.right - 90, bounds.top + 7, bounds.right - 54, bounds.bottom - 7};
}

VSTGUI::CMouseEventResult HeaderView::onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {
    if (!(buttons & VSTGUI::kLButton))
        return VSTGUI::kMouseEventNotHandled;

    if (noteMapButtonRect().pointInside(where)) {
        toggleNoteMap();
        return VSTGUI::kMouseEventHandled;
    }

    auto bounds = getViewSize();
    VSTGUI::CRect presetRect(bounds.left + 180, bounds.top + 5, bounds.right - 100, bounds.bottom - 5);

    if (!presetRect.pointInside(where))
        return VSTGUI::kMouseEventNotHandled;

    auto menu = VSTGUI::makeOwned<VSTGUI::COptionMenu>(VSTGUI::CRect(0, 0, 0, 0), nullptr, -1);
    menu->addEntry("Init (All Lanes)");
    menu->addSeparator();
    for (int i = 0; i < kFactoryPresetCount; ++i) {
        menu->addEntry(getFactoryPresetInfo(i).name);
    }

    VSTGUI::CPoint framePos = where;
    localToFrame(framePos);

    if (menu->popup(getFrame(), framePos)) {
        auto result = menu->getLastResult();
        if (result == 0) {
            resetToInit();
        } else if (result >= 2 && result < 2 + kFactoryPresetCount) {
            applyPreset(result - 2);
        }
    }

    return VSTGUI::kMouseEventHandled;
}

void HeaderView::applyPreset(int index) {
    if (!controller_)
        return;

    auto state = makeFactoryPreset(index);
    selectedPreset_ = index;

    auto pushParam = [this](Steinberg::Vst::ParamID id, double value) {
        controller_->beginEdit(id);
        controller_->setParamNormalized(id, value);
        controller_->performEdit(id, value);
        controller_->endEdit(id);
    };

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
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kDriftRate), static_cast<double>((cfg.driftRate + 4.0f) / 8.0f));
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

    static_cast<PolyController*>(controller_)->mutableCachedState().sceneA = state;

    pushParam(ParamIDs::kMacroComplexity, state.macros.complexity);
    pushParam(ParamIDs::kMacroDensity, state.macros.density);
    pushParam(ParamIDs::kMacroSyncopation, state.macros.syncopation);
    pushParam(ParamIDs::kMacroSwing, state.macros.swing);
    pushParam(ParamIDs::kMacroTension, state.macros.tension);
    pushParam(ParamIDs::kMacroHumanize, state.macros.humanize);

    pushParam(ParamIDs::kActiveLaneCount, (state.activeLaneCount - 1) / 7.0);
    pushParam(ParamIDs::kSeed, state.seed / 999999.0);

    if (auto* frame = getFrame())
        frame->invalid();
    else
        invalid();
}

void HeaderView::toggleNoteMap() {
    auto* frame = getFrame();
    if (!frame)
        return;

    for (uint32_t i = 0; i < frame->getNbViews(); ++i) {
        if (auto* nmv = dynamic_cast<NoteMapView*>(frame->getView(i))) {
            frame->removeView(nmv, true);
            invalid();
            return;
        }
    }

    auto frameBounds = frame->getViewSize();
    auto* nmv = new NoteMapView( // ownership-transfer
        VSTGUI::CRect(0, 32, frameBounds.right, frameBounds.bottom), controller_);
    frame->addView(nmv);
    invalid();
}

void HeaderView::resetToInit() {
    if (!controller_)
        return;

    selectedPreset_ = kInitPreset;

    auto pushParam = [this](Steinberg::Vst::ParamID id, double value) {
        controller_->beginEdit(id);
        controller_->setParamNormalized(id, value);
        controller_->performEdit(id, value);
        controller_->endEdit(id);
    };

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kProbability), 1.0);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kBaseVelocity), 100.0 / 127.0);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kEmphasisProb), 0.5);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kGhostFloor), 30.0 / 127.0);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kVelocitySpread), 0.05);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kSwingAmount), 0.0);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kHumanizeMs), 0.0);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kNoteDuration), 0.0);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kActive), 1.0);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kPhraseLength), 0.0);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kPhraseGap), 0.0);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kPhraseOffset), 0.0);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kMutationRate), 0.0);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kDriftRate), 0.5);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kTimingOffset), 0.5);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kKotekanSource), 0.0);

        static constexpr int kInitSteps[] = {4, 4, 8, 5, 7, 3, 6, 9};
        static constexpr int kInitSubs[] = {4, 4, 8, 16, 8, 16, 16, 16};
        static constexpr int kInitHits[] = {4, 2, 8, 3, 4, 2, 4, 5};
        static constexpr int kInitNotes[] = {36, 38, 42, 45, 46, 39, 43, 50};

        pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreSteps), (kInitSteps[lane] - 1) / 63.0);
        int subIdx = 0;
        switch (kInitSubs[lane]) {
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
        pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreHits), kInitHits[lane] / 64.0);
        pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreRotation), 0.0);
        pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreMidiNote), kInitNotes[lane] / 127.0);
        pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreCellCount), 0.0);
        pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreTimeline), 0.0);
        pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreFixedPatternLen), 0.0);
    }

    static_cast<PolyController*>(controller_)->mutableCachedState().sceneA = GrooveState{};

    pushParam(ParamIDs::kMacroComplexity, 0.5);
    pushParam(ParamIDs::kMacroDensity, 0.5);
    pushParam(ParamIDs::kMacroSyncopation, 0.0);
    pushParam(ParamIDs::kMacroSwing, 0.0);
    pushParam(ParamIDs::kMacroTension, 0.0);
    pushParam(ParamIDs::kMacroHumanize, 0.0);

    pushParam(ParamIDs::kActiveLaneCount, 1.0);
    pushParam(ParamIDs::kSeed, 0.0);

    if (auto* frame = getFrame())
        frame->invalid();
    else
        invalid();
}

} // namespace poly
