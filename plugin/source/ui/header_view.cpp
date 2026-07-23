#include "header_view.h"

#include <cstring>

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/coptionmenu.h"

#include "../controller.h"
#include "../plugids.h"
#include "note_map_view.h"
#include "poly/params_def.h"
#include "poly/presets.h"
#include "poly/types.h"

namespace poly {

static const char* kPresetLaneNames[kFactoryPresetCount][kMaxLanes] = {
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
    auto* initItem = menu->addEntry("Init (All Lanes)");
    initItem->setTag(-1);
    menu->addSeparator();

    for (int c = 0; c < kFactoryPresetCategoryCount; ++c) {
        const char* categoryName = kFactoryPresetCategories[c];
        auto submenu = VSTGUI::makeOwned<VSTGUI::COptionMenu>(VSTGUI::CRect(0, 0, 0, 0), nullptr, -1);
        int addedCount = 0;
        for (int p = 0; p < kFactoryPresetCount; ++p) {
            const auto& info = getFactoryPresetInfo(p);
            if (info.category && std::strcmp(info.category, categoryName) == 0) {
                auto* presetItem = submenu->addEntry(info.name);
                presetItem->setTag(p);
                ++addedCount;
            }
        }
        if (addedCount > 0) {
            menu->addEntry(submenu, categoryName);
        }
    }

    VSTGUI::CPoint framePos = where;
    localToFrame(framePos);

    if (menu->popup(getFrame(), framePos)) {
        int32_t idxInMenu = 0;
        VSTGUI::COptionMenu* clickedMenu = menu->getLastItemMenu(idxInMenu);
        if (clickedMenu) {
            VSTGUI::CMenuItem* clickedItem = clickedMenu->getEntry(idxInMenu);
            if (clickedItem) {
                int32_t tag = clickedItem->getTag();
                if (tag == -1) {
                    resetToInit();
                } else if (tag >= 0 && tag < kFactoryPresetCount) {
                    applyPreset(tag);
                }
            }
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

    // Inverse scaling via poly::params::engineToNorm{Expr,Core} (engine/include/poly/params_def.h).
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

    auto* polyCtrl = static_cast<PolyController*>(controller_);
    polyCtrl->mutableActiveScene() = state;
    for (int lane = 0; lane < kMaxLanes; ++lane)
        polyCtrl->setLaneName(lane, kPresetLaneNames[index][lane]);

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

    // kInit* arrays are ENGINE values (steps, subdivision, hits, midi note). The registry
    // converts them to normalized via engineToNormCore; UI-facing constants (0.5, 0.0, 1.0)
    // are already normalized so bypass the helper.
    static constexpr int kInitSteps[] = {4, 4, 8, 5, 7, 3, 6, 9};
    static constexpr int kInitSubs[] = {4, 4, 8, 16, 8, 16, 16, 16};
    static constexpr int kInitHits[] = {4, 2, 8, 3, 4, 2, 4, 5};
    static constexpr int kInitNotes[] = {36, 38, 42, 45, 46, 39, 43, 50};

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        auto exprEngine = [&](int offset, double engine) {
            pushParam(ParamIDs::laneParam(lane, offset),
                      params::engineToNormExpr(static_cast<uint32_t>(offset), engine));
        };
        auto coreEngine = [&](int offset, double engine) {
            pushParam(ParamIDs::laneCoreParam(lane, offset),
                      params::engineToNormCore(static_cast<uint32_t>(offset), engine));
        };
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kProbability), 1.0);
        exprEngine(ParamIDs::kBaseVelocity, 100.0);
        pushParam(ParamIDs::laneParam(lane, ParamIDs::kEmphasisProb), 0.5);
        exprEngine(ParamIDs::kGhostFloor, 30.0);
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

        coreEngine(ParamIDs::kCoreSteps, kInitSteps[lane]);
        coreEngine(ParamIDs::kCoreSubdivision, kInitSubs[lane]);
        coreEngine(ParamIDs::kCoreHits, kInitHits[lane]);
        pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreRotation), 0.0);
        coreEngine(ParamIDs::kCoreMidiNote, kInitNotes[lane]);
        pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreCellCount), 0.0);
        pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreTimeline), 0.0);
        pushParam(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreFixedPatternLen), 0.0);
    }

    GrooveState initState{};
    for (int lane = 0; lane < kMaxLanes; ++lane) {
        initState.lanes[static_cast<size_t>(lane)].midiNote = static_cast<int16_t>(kInitNotes[lane]);
        initState.lanes[static_cast<size_t>(lane)].cycle.steps = kInitSteps[lane];
        initState.lanes[static_cast<size_t>(lane)].cycle.subdivision = kInitSubs[lane];
        initState.lanes[static_cast<size_t>(lane)].hitCount = kInitHits[lane];
        initState.lanes[static_cast<size_t>(lane)].active = true;
    }
    initState.activeLaneCount = kMaxLanes;
    auto* polyCtrl = static_cast<PolyController*>(controller_);
    polyCtrl->mutableActiveScene() = initState;
    polyCtrl->resetLaneNames();

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
